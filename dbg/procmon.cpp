#include <Windows.h>
#include "procmon.h"

int ProcMonitor::ms_curIndex = 0xff11;

ProcMonitor::ProcMonitor() {
    m_hMonitorThread = NULL;
    m_hExitNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
}

ProcMonitor::~ProcMonitor() {
}

ProcMonitor *ProcMonitor::GetInstance() {
    static ProcMonitor *s_p = NULL;
    if (s_p == NULL)
    {
        s_p = new ProcMonitor();
    }
    return s_p;
}

HProcListener ProcMonitor::RegisterListener(ProcListener *listener) {
    CScopedLocker lock(this);
    ProcRegisterInfo tmp;
    tmp.m_index = ms_curIndex++;
    tmp.m_listener = listener;

    //dispatch cur proc
    list<const ProcMonInfo *> added;
    for (map<DWORD, ProcMonInfo *>::const_iterator it = m_ProcInfo.begin() ; it != m_ProcInfo.end() ; it++)
    {
        ProcMonInfo *ptr = it->second;
        added.push_back(ptr);
        tmp.m_ProcCache.insert(it->first);
    }
    m_register[tmp.m_index] = tmp;
    listener->OnProcChanged(tmp.m_index, added, list<DWORD>());

    if (!m_hMonitorThread)
    {
        m_hMonitorThread = CreateThread(NULL, 0, MonitorThread, this, 0, NULL);
    }
    return tmp.m_index;
}

void ProcMonitor::UnRegisterListener(HProcListener index) {
    CScopedLocker lock(this);
    m_register.erase(index);
}

struct ProcEnumParam {
    set<DWORD> curProcSet;                  //all proc
    map<DWORD, ProcMonInfo *> addedProc;    //new proc
};

BOOL ProcMonitor::ProcHandlerW(PPROCESSENTRY32W pe, void *pParam)
{
    ProcEnumParam *ptr = (ProcEnumParam *)pParam;
    ProcMonitor *pThis = ProcMonitor::GetInstance();
    DWORD unique = GetProcUnique(pe->th32ProcessID);
    if (unique == 0)
    {
        return TRUE;
    }

    {
        CScopedLocker lock(pThis);
        ptr->curProcSet.insert(unique);
        if (pThis->m_ProcInfo.end() != pThis->m_ProcInfo.find(unique))
        {
            return TRUE;
        }
    }

    ProcMonInfo *newProc = new ProcMonInfo();
    newProc->procUnique = unique;
    newProc->procPath = GetProcPathByPid(pe->th32ProcessID);

    if (newProc->procPath.empty())
    {
        delete newProc;
        return TRUE;
    }

    newProc->procPid = pe->th32ProcessID;
    IsPeFileW(newProc->procPath.c_str(), &newProc->x64);

    newProc->procCmd = GetProcessCommandLine(newProc->procPid, newProc->x64);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, pe->th32ProcessID);
    if (hProcess)
    {
        FILETIME filetime = {0};
        FILETIME exittime = {0};
        FILETIME kerneltime = {0};
        FILETIME usertime = {0};
        GetProcessTimes(hProcess, &filetime, &exittime, &kerneltime, &usertime);
        FILETIME local = {0};
        FileTimeToLocalFileTime(&filetime, &local);
        SYSTEMTIME systime = {0};
        FileTimeToSystemTime(&local, &systime);
        newProc->startTime.format(
            L"%02d:%02d:%02d %03d",
            systime.wHour,
            systime.wMinute,
            systime.wSecond,
            systime.wMilliseconds
            );
        CloseHandle(hProcess);
    }
    else
    {
        newProc->startTime = L"UnKnown";
    }
    ptr->addedProc[unique] = newProc;
    return TRUE;
}

void ProcMonitor::RefushProc() {
    {
        CScopedLocker lock(this);
        if (m_register.empty())
        {
            return;
        }
    }

    PVOID ptr = DisableWow64Red();
    ProcEnumParam param;
    IterateProcW(ProcHandlerW, &param);
    RevertWow64Red(ptr);

    {
        CScopedLocker lock(this);
        //减少的
        list<DWORD> killed;
        for (map<DWORD, ProcMonInfo *>::const_iterator it1 = m_ProcInfo.begin() ; it1 != m_ProcInfo.end() ;)
        {
            if (param.curProcSet.end() == param.curProcSet.find(it1->first))
            {
                for (map<HProcListener, ProcRegisterInfo>::const_iterator ij1 = m_register.begin() ; ij1 != m_register.end() ; ij1++)
                {
                    killed.push_back(it1->first);
                }

                delete it1->second;
                it1 = m_ProcInfo.erase(it1);
            } else {
                it1++;
            }
        }

        //新增的
        list<const ProcMonInfo *> added;
        for (map<DWORD, ProcMonInfo *>::const_iterator it = param.addedProc.begin() ; it != param.addedProc.end() ; it++)
        {
            added.push_back(it->second);
            m_ProcInfo[it->first] = it->second;
        }

        for (map<HProcListener, ProcRegisterInfo>::const_iterator ij = m_register.begin() ; ij != m_register.end() ; ij++)
        {
            ij->second.m_listener->OnProcChanged(ij->first, added, killed);
        }
    }
}

DWORD ProcMonitor::MonitorThread(LPVOID pParam) {
    ProcMonitor *pThis = (ProcMonitor *)pParam;

    pThis->RefushProc();
    while (true) {
        DWORD ret = WaitForSingleObject(pThis->m_hExitNotify, 3000);

        if (ret != WAIT_TIMEOUT)
        {
            break;
        }
        pThis->RefushProc();
    }
    return 0;
}

void ProcMonitor::DispatchProcChanged() {
}

DWORD ProcMonitor::GetProcUnique(DWORD pid) {
    HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!proc)
    {
        return 0;
    }

    FILETIME create = {0};
    FILETIME exit = {0};
    FILETIME kernel = {0};
    FILETIME user = {0};
    GetProcessTimes(proc, &create, &exit, &kernel, &user);
    CloseHandle(proc);
    return crc32((const char *)&create, sizeof(create), pid);
}