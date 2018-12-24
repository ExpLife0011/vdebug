#include "ServiceRunner.h"
#include "runner.h"
#include <ComStatic/ComStatic.h>
#include <Shlwapi.h>
#include <list>
#include <string>

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
    SERVICE_TABLE_ENTRYW ste[] = {
        {SFV_SERVICE_NAME, ServiceMainProc},
        {NULL, NULL},
    };
    StartServiceCtrlDispatcherW(ste);
    return true;
}

void ServiceRunner::RunProcess(LPCWSTR wszKey)
{
    WCHAR wszImage[1024] = {0};
    WCHAR wszCmd[1024] = {0};
    WCHAR wszSub[MAX_PATH] = {0};
    DWORD dwSize = sizeof(wszImage);
    DWORD dwSessionId = 1;
    wnsprintfW(
        wszSub,
        sizeof(wszSub) / sizeof(WCHAR),
        L"%ls\\%ls",
        PATH_SERVICE_CACHE,
        wszKey
        );
    if (!wszSub[0])
    {
        return;
    }
    SHGetValueW(
        HKEY_LOCAL_MACHINE,
        wszSub,
        L"image",
        NULL,
        wszImage,
        &dwSize
        );
    if (!wszImage[0])
    {
        return;
    }
    dwSize = sizeof(wszCmd);
    SHGetValueW(
        HKEY_LOCAL_MACHINE,
        wszSub,
        L"cmd",
        NULL,
        wszCmd,
        &dwSize
        );
    dwSize = sizeof(dwSessionId);
    SHGetValueW(
        HKEY_LOCAL_MACHINE,
        wszSub,
        L"sessionId",
        NULL,
        &dwSessionId,
        &dwSize
        );

    DWORD dwShell = 0;

    dwSize = sizeof(dwShell);

    SHGetValueW(HKEY_LOCAL_MACHINE, wszSub, L"shell", NULL, &dwShell, &dwSize);
    //dp(L"image:%ls, cmd:%ls, session:%d, shell:%d", wszImage, wszCmd, dwSessionId, dwShell);
    RunInSession(wszImage, wszCmd, dwSessionId, dwShell);
}

void ServiceRunner::OnServWork()
{
    HKEY hKey = NULL;
    if (ERROR_SUCCESS != RegOpenKeyW(HKEY_LOCAL_MACHINE, PATH_SERVICE_CACHE, &hKey))
    {
        return;
    }
    DWORD dwIdex = 0;
    WCHAR wszName[MAX_PATH] = {0x00};
    DWORD dwNameLen = MAX_PATH;
    DWORD dwStatus = 0;
    list<wstring> lst;
    while (TRUE)
    {
        dwNameLen = MAX_PATH;
        wszName[0] = 0;
        dwStatus = SHEnumKeyExW(hKey, dwIdex++, wszName, &dwNameLen);
        if (wszName[0])
        {
            lst.push_back(wszName);
        }
        if (ERROR_SUCCESS != dwStatus)
        {
            break;
        }
        if (16 == lstrlenW(wszName))
        {
            RunProcess(wszName);
        }
    }
    list<wstring>::iterator itm;
    for (itm = lst.begin() ; itm != lst.end() ; itm++)
    {
        SHDeleteKeyW(hKey, itm->c_str());
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
                ReportLocalServStatus(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
                break;
            }
            ReportLocalServStatus(pThis->m_ServStatus, SERVICE_STOP_PENDING, ERROR_SUCCESS);
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
            ReportLocalServStatus(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
        }
        break;
    case  SERVICE_CONTROL_SHUTDOWN:
        break;
    case  SERVICE_CONTROL_INTERROGATE:
        {
            if (WAIT_OBJECT_0 == WaitForSingleObject(pThis->m_ServiceThread, 0))
            {
                ReportLocalServStatus(pThis->m_ServStatus, SERVICE_STOPPED, ERROR_SUCCESS);
            }
            else
            {
                ReportLocalServStatus(pThis->m_ServStatus, SERVICE_RUNNING, ERROR_SUCCESS);
            }
        }
        break;
    default:
        break;
    }
    return ERROR_SUCCESS;
}

VOID ServiceRunner::ServiceMainProc(DWORD dwArgc, LPWSTR *wszArgv) {
    OutputDebugStringA("runner:aaaa");
    ServiceRunner *pThis = ServiceRunner::GetInstance();
    pThis->m_ServiceNotify = CreateLowsdEvent(FALSE, FALSE, SFV_NOTIFY_NAME);
    pThis->m_ServiceLeave = CreateEventW(NULL, FALSE, FALSE, NULL);
    OutputDebugStringA("runner:bbb");
    pThis->m_ServStatus = RegisterServiceCtrlHandlerExW(SFV_SERVICE_NAME, ServiceHandlerEx, NULL);
    ReportLocalServStatus(pThis->m_ServStatus, SERVICE_START_PENDING, ERROR_SUCCESS);
    if (!pThis->m_ServiceThread)
    {
        OutputDebugStringA("runner:ccc");
        pThis->m_ServiceThread = CreateThread(NULL, 0, ServiveMain, NULL, 0, NULL);
    }
    ReportLocalServStatus(pThis->m_ServStatus, SERVICE_RUNNING, ERROR_SUCCESS);
}