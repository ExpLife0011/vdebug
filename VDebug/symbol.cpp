#include <Windows.h>
#include <DbgHelp.h>
#include "symbol.h"
#include "Debugger.h"
#include "common.h"
#include "Command.h"
#include "view/MainView.h"

CSymbolHlpr::CSymbolHlpr(const ustring &wstrSymbolPath)
{
    m_hNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hInitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_wstrSymbolPath = wstrSymbolPath;
    CloseHandle(CreateThread(NULL, 0, WorkThread, this, 0, 0));
    WaitForSingleObject(m_hInitEvent, INFINITE);
}

bool CSymbolHlpr::SetSymbolPath(const ustring &wstrSymbolPath)
{
    CSymbolTaskHeader task;
    task.m_dwSize = sizeof(CSymbolTaskHeader);
    task.m_eTaskType = em_task_setpath;
    task.m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
    task.m_pParam = NULL;
    SendTask(&task);
    return true;
}

CSymbolHlpr:: ~CSymbolHlpr()
{}

bool CSymbolHlpr::SendTask(CSymbolTaskHeader *pTask, BOOL bAtOnce)
{
    {
        CScopedLocker lock(this);
        pTask->m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (bAtOnce)
        {
            m_vTaskQueue.push_front(pTask);
        }
        else
        {
            m_vTaskQueue.push_back(pTask);
        }
        SetEvent(m_hNotifyEvent);
    }

    WaitForSingleObject(pTask->m_hNotify, INFINITE);
    CloseHandle(pTask->m_hNotify);
    return true;
}

bool CSymbolHlpr::PostTask(CSymbolTaskHeader *pTask, BOOL bAtOnce)
{
    {
        CScopedLocker(this);
        if (bAtOnce)
        {
            m_vTaskQueue.push_front(pTask);
        }
        else
        {
            m_vTaskQueue.push_back(pTask);
        }
    }
    SetEvent(m_hNotifyEvent);
    return true;
}

BOOL CSymbolHlpr::EnumSymbolProc(PSYMBOL_INFOW pSymInfo, ULONG uSymbolSize, PVOID pUserContext)
{
    CTaskLoadSymbol *ptr = (CTaskLoadSymbol *)pUserContext;
    ustring wstr = ptr->m_ModuleInfo.m_wstrDllName;
    size_t pos = wstr.rfind(L'.');
    if (pos != wstring::npos)
    {
        wstr.erase(pos, wstr.size() - pos);
    }
    wstr += L"!";
    wstr += pSymInfo->Name;
    DbgFunInfo info;
    info.m_dwModuleBase = ptr->m_ModuleInfo.m_dwBaseOfImage;
    info.m_dwProcAddr = pSymInfo->Address;
    info.m_dwProcOffset = (info.m_dwProcAddr - info.m_dwModuleBase);
    info.m_wstrModule = ptr->m_ModuleInfo.m_wstrDllName;
    info.m_wstrFunName = pSymInfo->Name;
    GetCurrentDbgger()->InsertFunMsg(wstr.makelower(), info);
    return TRUE;
}

bool CSymbolHlpr::LoadSymbol(CTaskLoadSymbol *pModuleInfo)
{
    ustring wstrDll = GetFilePathFromHandle(pModuleInfo->m_hImgaeFile);
    DWORD64 dwBaseAddr = SymLoadModuleExW(
        NULL,
        pModuleInfo->m_hImgaeFile,
        PathFindFileNameW(wstrDll.c_str()),
        0,
        (DWORD64)pModuleInfo->m_dwBaseOfModule,
        0,
        0,
        0
        );

    if (ustring::npos != wstrDll.find(L"kernel32.dll"))
    {
        SYMBOL_INFOW info = {0};
        SymFromNameW(NULL, L"kernel32!CreateFileW", &info);
        int e = 0;
    }

    IMAGEHLP_MODULEW64 info = {0};
    info.SizeOfStruct = sizeof(info);
    SymGetModuleInfoW64(NULL, (DWORD64)dwBaseAddr, &info);

    ModuleInfo *pModule = &(pModuleInfo->m_ModuleInfo);
    pModule->m_wstrDllPath = wstrDll;
    pModule->m_wstrDllName = PathFindFileNameW(wstrDll.c_str());
    SymEnumSymbolsW(
        NULL,
        dwBaseAddr,
        NULL,
        EnumSymbolProc,
        pModuleInfo
        );

    pModule->m_dwBaseOfImage = dwBaseAddr;
    pModule->m_dwModuleSize = info.ImageSize;
    pModule->m_dwEndAddr = (pModule->m_dwBaseOfImage + pModule->m_dwModuleSize);
    pModule->m_hModule = (HMODULE)dwBaseAddr;
    SymUnloadModule64(NULL, pModuleInfo->m_dwBaseOfModule);
    return true;
}

bool CSymbolHlpr::GetSymbolFromAddr(CTaskSymbolFromAddr *pSymbolInfo)
{
    DWORD64 dwOffset = 0;
    ULONG64 buffer[(sizeof(SYMBOL_INFO) +
        MAX_SYM_NAME*sizeof(TCHAR) +
        sizeof(ULONG64) - 1) /
        sizeof(ULONG64)] = {0};
    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;

    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    if (!SymFromAddrW(NULL, pSymbolInfo->m_dwAddr, &dwOffset, pSymbol))
    {
        return false;
    }
    pSymbolInfo->m_wstrSymbol = FormatW(L"%ls+0x%08x", pSymbol->Name, dwOffset);
    return true;
}

bool CSymbolHlpr::SetSymbolPath(CTaskSetSymbolPath *pSymbolPath)
{
    return (TRUE == SymSetSearchPathW(NULL, pSymbolPath->m_wstrSymbolPath.c_str()));
}

bool CSymbolHlpr::StackWalk(CTaskStackWalkInfo *pStackWalkInfo)
{
    STACKFRAME64 *pFrame = &(pStackWalkInfo->m_context);
    DWORD machineType = pStackWalkInfo->m_dwMachineType;
    int iMaxWalks = 1024;
    bool bStat = false;
    for (int i = 0 ; i < iMaxWalks ; i++)
    {
        if (StackWalk64(
            machineType,
            NULL,
            NULL,
            pFrame,
            NULL,
            pStackWalkInfo->m_pfnReadMemoryProc,
            SymFunctionTableAccess64,
            pStackWalkInfo->m_pfnGetModuleBaseProc,
            NULL
            ))
        {
            pStackWalkInfo->m_FrameSet.push_back(*pFrame);
            bStat = true;
        }
        else
        {
            break;
        }
    }
    return bStat;
}

bool CSymbolHlpr::FindFileForDump(CTaskFindFileForDump *pFileInfo)
{
    WCHAR wszOut[MAX_PATH] = {0};
    SymFindFileInPathW(
        NULL,
        GetSymbolHlpr()->m_wstrSymbolPath.c_str(),
        pFileInfo->m_wstrModuleName.c_str(),
        (PVOID)pFileInfo->m_TimeStamp,
        (DWORD)pFileInfo->m_SizeofImage,
        0,
        SSRVOPT_DWORD,
        wszOut,
        NULL,
        NULL
        );
    pFileInfo->m_wstrFullPath = wszOut;
    return !pFileInfo->m_wstrFullPath.empty();
}

bool CSymbolHlpr::DoTask(CSymbolTaskHeader *pTask)
{
    bool bStat = false;
    switch (pTask->m_eTaskType)
    {
    case  em_task_loadsym:
        {
            bStat = LoadSymbol((CTaskLoadSymbol *)pTask->m_pParam);
        }
        break;
    case  em_task_symaddr:
        {
            bStat = GetSymbolFromAddr((CTaskSymbolFromAddr *)pTask->m_pParam);
        }
        break;
    case  em_task_setpath:
        {
            bStat = SetSymbolPath((CTaskSetSymbolPath *)pTask->m_pParam);
        }
        break;
    case em_task_stackwalk:
        {
            bStat = StackWalk((CTaskStackWalkInfo *)pTask->m_pParam);
        }
        break;
    case em_task_findfile:
        {
            bStat = FindFileForDump((CTaskFindFileForDump *)pTask->m_pParam);
        }
        break;
    }
    return bStat;
}

DWORD CSymbolHlpr::WorkThread(LPVOID pParam)
{
    CSymbolHlpr *pThis = (CSymbolHlpr *)pParam;
    CSymbolTaskHeader *pTask = NULL;

    //符号支持需要symsrv.dll的支持并需要symsrv.yes文件
    SymInitializeW(NULL, pThis->m_wstrSymbolPath.c_str(), FALSE);
    SetEvent(pThis->m_hInitEvent);
    while (TRUE)
    {
        WaitForSingleObject(pThis->m_hNotifyEvent, INFINITE);
        {
            CScopedLocker lock(pThis);
            if (pThis->m_vTaskQueue.empty())
            {
                continue;
            }
            pTask = *(pThis->m_vTaskQueue.begin());
            pThis->m_vTaskQueue.pop_front();
        }

        if (!pTask)
        {
            continue;
        }

        if (DoTask(pTask))
        {
            pTask->m_bSucc = TRUE;
        }
        SetEvent(pTask->m_hNotify);
    }
    return 0;
}

static CSymbolHlpr *s_ptr = NULL;

CSymbolHlpr *GetSymbolHlpr()
{
    return s_ptr;
}

bool InitSymbolHlpr(const ustring &wtrSymbolPath)
{
    s_ptr = new CSymbolHlpr(wtrSymbolPath);
    return true;
}