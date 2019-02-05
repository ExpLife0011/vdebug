#include <Windows.h>
#include <Shlwapi.h>
#include <list>
#include <string>
#include <ComStatic/ComStatic.h>
#include "ServiceRunner.h"
#include "runner.h"

using namespace std;

#pragma comment(lib, "Shlwapi.lib")

ServiceRunner::ServiceRunner() {
    m_ServStatus = NULL;
    m_ServiceThread = NULL;
    m_ServiceNotify = NULL;
    m_ServiceLeave = NULL;
}

ServiceRunner *ServiceRunner::GetInstance() {
    static ServiceRunner *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new ServiceRunner();
    }

    return s_ptr;
}

ServiceRunner::~ServiceRunner() {
}

bool ServiceRunner::InitServiceRunner() {
    SERVICE_TABLE_ENTRYA ste[] = {
        {SFV_SERVICE_NAME, ServiceMainProc},
        {NULL, NULL},
    };
    StartServiceCtrlDispatcherA(ste);
    return true;
}

void ServiceRunner::RunProcess(LPCSTR szKey)
{
    CHAR szImage[1024] = {0};
    CHAR szCmd[1024] = {0};
    CHAR szSub[MAX_PATH] = {0};
    DWORD dwSize = sizeof(szImage);
    DWORD dwSessionId = 1;
    wnsprintfA(
        szSub,
        sizeof(szSub) / sizeof(char),
        "%hs\\%hs",
        PATH_SERVICE_CACHE,
        szKey
        );
    if (!szSub[0])
    {
        return;
    }
    SHGetValueA(
        HKEY_LOCAL_MACHINE,
        szSub,
        "image",
        NULL,
        szImage,
        &dwSize
        );
    if (!szImage[0])
    {
        return;
    }
    dwSize = sizeof(szCmd);
    SHGetValueA(
        HKEY_LOCAL_MACHINE,
        szSub,
        "cmd",
        NULL,
        szCmd,
        &dwSize
        );
    dwSize = sizeof(dwSessionId);
    SHGetValueA(
        HKEY_LOCAL_MACHINE,
        szSub,
        "sessionId",
        NULL,
        &dwSessionId,
        &dwSize
        );

    DWORD dwShell = 0;

    dwSize = sizeof(dwShell);

    SHGetValueA(HKEY_LOCAL_MACHINE, szSub, "shell", NULL, &dwShell, &dwSize);
    //dp(L"image:%ls, cmd:%ls, session:%d, shell:%d", wszImage, wszCmd, dwSessionId, dwShell);
    RunInSession(szImage, szCmd, dwSessionId, dwShell);
}

void ServiceRunner::OnServWork()
{
    HKEY hKey = NULL;
    if (ERROR_SUCCESS != RegOpenKeyA(HKEY_LOCAL_MACHINE, PATH_SERVICE_CACHE, &hKey))
    {
        return;
    }
    DWORD dwIdex = 0;
    char szName[MAX_PATH] = {0x00};
    DWORD dwNameLen = MAX_PATH;
    DWORD dwStatus = 0;
    list<string> lst;
    while (TRUE)
    {
        dwNameLen = MAX_PATH;
        szName[0] = 0;
        dwStatus = SHEnumKeyExA(hKey, dwIdex++, szName, &dwNameLen);
        if (szName[0])
        {
            lst.push_back(szName);
        }
        if (ERROR_SUCCESS != dwStatus)
        {
            break;
        }
        if (16 == lstrlenA(szName))
        {
            RunProcess(szName);
        }
    }
    list<string>::iterator itm;
    for (itm = lst.begin() ; itm != lst.end() ; itm++)
    {
        SHDeleteKeyA(hKey, itm->c_str());
    }
    RegCloseKey(hKey);
}

DWORD ServiceRunner::ServiveMain(LPVOID param)
{
    ServiceRunner *pThis = ServiceRunner::GetInstance();
    DWORD dwTickCount = 0;
    DWORD dwRet = 0;
    HANDLE arry[] = {pThis->m_ServiceNotify, pThis->m_ServiceLeave};
    OutputDebugStringA("runner:ddd");
    while (TRUE)
    {
        dwRet = WaitForMultipleObjects(2, arry, FALSE, 2000);
        if (WAIT_TIMEOUT == dwRet)
        {
        }

        if (WAIT_OBJECT_0 == dwRet)
        {
            pThis->OnServWork();
        }

        if (WAIT_OBJECT_0 + 1 == dwRet)
        {
            break;
        }
    }
    return 0;
}

BOOL ServiceRunner::ReportLocalServStatusInRunner(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode)
{
    static SERVICE_STATUS s_stat =
    {
        SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS
    };
    if (SERVICE_START_PENDING == dwCurrentStat)
    {
        s_stat.dwControlsAccepted = 0;
    }
    else
    {
        s_stat.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
    }
    s_stat.dwCurrentState = dwCurrentStat;
    s_stat.dwWin32ExitCode = dwWin32ExitCode;
    return SetServiceStatus(hStatus, &s_stat);
}

DWORD ServiceRunner::ServiceHandlerEx(DWORD dwControl, DWORD dwEvent, LPVOID pEventData, LPVOID pContext)
{
    ServiceRunner *pThis = ServiceRunner::GetInstance();
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
        {
            OutputDebugStringA("runner:fff");
            if (!pThis->m_ServiceThread)
            {
                ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
                break;
            }
            ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_STOP_PENDING, ERROR_SUCCESS);
            SetEvent(pThis->m_ServiceLeave);
            if (WAIT_TIMEOUT == WaitForSingleObject(pThis->m_ServiceThread, 3000))
            {
                DWORD dwCode = 0;
                if (GetExitCodeThread(pThis->m_ServiceThread, &dwCode) && STILL_ACTIVE == dwCode)
                {
                    TerminateThread(pThis->m_ServiceThread, 0);
                }
            }
            CloseHandle(pThis->m_ServiceThread);
            pThis->m_ServiceThread = NULL;
            ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
        }
        break;
    case  SERVICE_CONTROL_SHUTDOWN:
        break;
    case  SERVICE_CONTROL_INTERROGATE:
        {
            if (WAIT_OBJECT_0 == WaitForSingleObject(pThis->m_ServiceThread, 0))
            {
                ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
            }
            else
            {
                ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_RUNNING, ERROR_SUCCESS);
            }
        }
        break;
    default:
        break;
    }
    return ERROR_SUCCESS;
}

VOID ServiceRunner::ServiceMainProc(DWORD dwArgc, LPSTR *szArgv) {
    OutputDebugStringA("runner:aaaa");
    ServiceRunner *pThis = ServiceRunner::GetInstance();
    pThis->m_ServiceNotify = CreateLowsdEvent(FALSE, FALSE, SFV_NOTIFY_NAME);
    pThis->m_ServiceLeave = CreateEventW(NULL, FALSE, FALSE, NULL);
    OutputDebugStringA("runner:bbb");
    pThis->m_ServStatus = RegisterServiceCtrlHandlerExA(SFV_SERVICE_NAME, ServiceHandlerEx, NULL);
    ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_START_PENDING, ERROR_SUCCESS);
    if (!pThis->m_ServiceThread)
    {
        OutputDebugStringA("runner:ccc");
        pThis->m_ServiceThread = CreateThread(NULL, 0, ServiveMain, NULL, 0, NULL);
    }
    ReportLocalServStatusInRunner(pThis->m_ServStatus, SERVICE_RUNNING, ERROR_SUCCESS);
}