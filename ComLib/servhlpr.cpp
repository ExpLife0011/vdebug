#include <Windows.h>
#include <Shlwapi.h>
#include <ComStatic/StrUtil.h>
#include "ComUtil.h"

//初始化服务
BOOL WINAPI InstallLocalServiceW(LPCWSTR image, LPCWSTR cmd, LPCWSTR wszsName, LPCWSTR wszDisplayName, LPCWSTR wszDescripion)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSev = NULL;
    BOOL bStat = FALSE;
    do
    {
        if (!cmd || !*cmd)
        {
            break;
        }
        hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hScm)
        {
            break;
        }
        WCHAR wszDependencies[] = L"tcpip\0\0\0";
        hSev = OpenServiceW(hScm, wszsName, SERVICE_ALL_ACCESS);
        WCHAR wszCmd[MAX_PATH] = {0};
        lstrcpynW(wszCmd, image, MAX_PATH);
        PathQuoteSpacesW(wszCmd);
        StrCatBuffW(wszCmd, L" ", MAX_PATH);
        StrCatBuffW(wszCmd, cmd, MAX_PATH);

        if (!hSev)
        {
            hSev = CreateServiceW(
                hScm,
                wszsName,
                wszDisplayName,
                SERVICE_ALL_ACCESS,
                SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
                SERVICE_AUTO_START,
                SERVICE_ERROR_NORMAL,
                wszCmd,
                NULL,
                NULL,
                wszDependencies,
                NULL,
                NULL
                );
        }
        else
        {
            ChangeServiceConfigW(
                hSev,
                SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
                SERVICE_AUTO_START,
                SERVICE_ERROR_NORMAL,
                wszCmd,
                NULL,
                NULL,
                wszDependencies,
                NULL,
                NULL,
                wszDisplayName
                );
        }

        if (hSev)
        {
            SERVICE_DESCRIPTIONW sdc = {(LPWSTR)wszDescripion};
            bStat = ChangeServiceConfig2W(hSev, SERVICE_CONFIG_DESCRIPTION, &sdc);
        }
    } while (FALSE);

    if (hSev)
    {
        CloseServiceHandle(hSev);
    }

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }
    return bStat;
}

BOOL WINAPI InstallLocalServiceA(LPCSTR image, LPCSTR cmd, LPCSTR szName, LPCSTR szDisplayName, LPCSTR szDescripion) {
    return InstallLocalServiceW(AtoW(image).c_str(), AtoW(cmd).c_str(), AtoW(szName).c_str(), AtoW(szDisplayName).c_str(), AtoW(szDescripion).c_str());
}

//启动服务
BOOL WINAPI StartLocalServiceW(LPCWSTR wszSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSev = NULL;
    BOOL bStat = FALSE;
    do 
    {
        hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hScm)
        {
            break;
        }
        hSev = OpenServiceW(hScm, wszSrvName, SERVICE_ALL_ACCESS);
        if (!hSev)
        {
            break;
        }
        bStat = StartServiceW(hSev, 0, NULL);
        int ddd = GetLastError();
        if (!bStat && ERROR_SERVICE_ALREADY_RUNNING == GetLastError())
        {
            bStat = TRUE;
        }
    } while (FALSE);

    if (hSev)
    {
        CloseServiceHandle(hSev);
    }

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }
    return bStat;
}

BOOL WINAPI StartLocalServiceA(LPCSTR szSrvName) {
    return StartLocalServiceW(AtoW(szSrvName).c_str());
}

//停止服务
BOOL WINAPI StopLocalServiceW(LPCWSTR wszServName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSev = NULL;
    BOOL bStat = FALSE;
    do 
    {
        hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hScm)
        {
            break;
        }
        hSev = OpenServiceW(hScm, wszServName, SERVICE_ALL_ACCESS);
        if (!hSev)
        {
            break;
        }
        SERVICE_STATUS status;
        if (!QueryServiceStatus(hSev, &status))
        {
            break;
        }
        if (status.dwCurrentState == SERVICE_STOPPED)
        {
            bStat = TRUE;
            break;
        }

        if (status.dwCurrentState == SERVICE_STOP_PENDING || ControlService(hSev, SERVICE_CONTROL_STOP, &status))
        {
            DWORD dwCount = GetTickCount();
            while (GetTickCount() - dwCount <= 5000)
            {
                if (QueryServiceStatus(hSev, &status) && status.dwCurrentState == SERVICE_STOPPED)
                {
                    bStat = TRUE;
                }
                Sleep(500);
            }
        }
    } while (FALSE);

    if (hSev)
    {
        CloseServiceHandle(hSev);
    }
    if (hScm)
    {
        CloseServiceHandle(hScm);
    }
    return bStat;
}

BOOL WINAPI StopLocalServiceA(LPCSTR szServName) {
    return StopLocalServiceW(AtoW(szServName).c_str());
}

BOOL WINAPI RemoveLocalServiceW(LPCWSTR wszServName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hServ = NULL;
    BOOL bStat = FALSE;
    do 
    {
        hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!hScm)
        {
            break;
        }
        hServ = OpenServiceW(hScm, wszServName, SERVICE_ALL_ACCESS);
        if (!hServ)
        {
            break;
        }
        bStat = DeleteService(hServ);
    } while (FALSE);

    if (hServ)
    {
        CloseServiceHandle(hServ);
    }

    if (hScm)
    {
        CloseServiceHandle(hScm);
    }
    return bStat;
}

BOOL WINAPI RemoveLocalServiceA(LPCSTR szServName) {
    return RemoveLocalServiceW(AtoW(szServName).c_str());
}

BOOL WINAPI ReportLocalServStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode)
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

typedef struct _SERVICE_HANDLE_PACK
{
    SC_HANDLE scManager;
    SC_HANDLE serviceHandle;
} SERVICE_HANDLE_PACK, *PSERVICE_HANDLE_PACK;

static void WINAPI _FreeServiceHandlePack(PSERVICE_HANDLE_PACK pHandlePack)
{
    if (pHandlePack)
    {
        if (pHandlePack->serviceHandle)
        {
            CloseServiceHandle(pHandlePack->serviceHandle);
        }

        if (pHandlePack->scManager)
        {
            CloseServiceHandle(pHandlePack->scManager);
        }

        free((void*)pHandlePack);
    }
}
static PSERVICE_HANDLE_PACK WINAPI _ServGetServiceHandle(LPCWSTR wszServName, DWORD dwDesiredAccess)
{
    BOOL bSucc = FALSE;
    PSERVICE_HANDLE_PACK ret = NULL;

    do
    {
        SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!scManager)
        {
            break;
        }

        ret = (PSERVICE_HANDLE_PACK)malloc(sizeof(SERVICE_HANDLE_PACK));
        if (!ret)
        {
            CloseServiceHandle(scManager);
            break;
        }

        ret->scManager = scManager;
        if (!wszServName)
        {
            ret->serviceHandle = NULL;
            bSucc = TRUE;
            break;
        }

        ret->serviceHandle = OpenServiceW(scManager, wszServName, dwDesiredAccess);
        if (!ret->serviceHandle)
        {
            break;
        }

        bSucc = TRUE;
    } while (FALSE);

    if (!bSucc)
    {
        _FreeServiceHandlePack(ret);

        ret = NULL;
    }

    return ret;
}

BOOL ServControlW(LPCWSTR servName, DWORD controlCode)
{
    BOOL bRet = FALSE;
    PSERVICE_HANDLE_PACK pHandlePack = NULL;

    do
    {
        pHandlePack = _ServGetServiceHandle(servName, SERVICE_STOP);
        if (!pHandlePack)
        {
            break;
        }

        SERVICE_STATUS status = {0};
        bRet = ControlService(pHandlePack->serviceHandle, controlCode, &status);
    } while (FALSE);

    _FreeServiceHandlePack(pHandlePack);

    return bRet;
}

BOOL ServControlA(LPCSTR servName, DWORD controlCode) {
    return ServControlW(AtoW(servName).c_str(), controlCode);
}

BOOL WINAPI ServStopW(LPCWSTR servName)
{
    return ServControlW(servName, SERVICE_CONTROL_STOP) || (GetLastError() == ERROR_SERVICE_NOT_ACTIVE);
}

BOOL WINAPI ServStopA(LPCSTR servName) {
    return ServStopW(AtoW(servName).c_str());
}

BOOL WINAPI ServStartW(LPCWSTR servName)
{
    BOOL bRet = FALSE;
    PSERVICE_HANDLE_PACK pHandlePack = NULL;

    do
    {
        pHandlePack = _ServGetServiceHandle(servName, SERVICE_START);
        if (!pHandlePack)
        {
            break;
        }

        bRet = StartService(pHandlePack->serviceHandle, 0, NULL) || (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING);
    } while (FALSE);

    _FreeServiceHandlePack(pHandlePack);
    return bRet;
}

BOOL WINAPI ServStartA(LPCSTR servName) {
    return ServStartW(AtoW(servName).c_str());
}