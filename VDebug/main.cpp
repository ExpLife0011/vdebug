#include <WinSock2.h>
#include <Windows.h>
#include "view.h"
#include <runner/runner.h>
#include <ComLib/ComLib.h>
#include <ComLib/ComLib.h>
#include <Shlwapi.h>
#include "DbgCtrlService.h"
#include "OpenView.h"

#pragma comment(lib, "Ws2_32.lib")

HINSTANCE g_hInstance = NULL;

static BOOL _StartViewProc() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    if (!LoadLibraryW(L"SyntaxView.dll"))
    {
        MessageBoxW(NULL, L"SyntaxView.dll not Find", L"Error", 0);
        return FALSE;
    }

    if (!LoadLibraryW(L"ComLib32.dll"))
    {
        MessageBoxW(NULL, L"ComLib32.dll not Find", L"Error", 0);
        return FALSE;
    }

    CoInitialize(NULL);
    ShowMainView();
    CoUninitialize();
    WSACleanup();
    return TRUE;
}

static BOOL _StartService() {
    CHAR szRunner[MAX_PATH] = {0};
    CHAR szServ[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, szRunner, MAX_PATH);
    PathAppendA(szRunner, "..\\runner.exe");

    GetWindowsDirectoryA(szServ, MAX_PATH);
    PathAppendA(szServ, "DbgService.exe");

    BOOL bServ = TRUE;
    BOOL bStat = FALSE;
    do 
    {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(szServ))
        {
            CopyFileA(szRunner, szServ, FALSE);
        }
        else
        {
            if (!IsSameFileA(szRunner, szServ))
            {
                mstring strTemp(szServ);
                mstring strName;
                strName.format("..\\DbgService%08x.tmp", GetTickCount());
                strTemp.path_append(strName.c_str());

                MoveFileExA(szServ, strTemp.c_str(), MOVEFILE_REPLACE_EXISTING);
                MoveFileExA(strTemp.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                CopyFileA(szRunner, szServ, FALSE);
                ServStopA(SFV_SERVICE_NAME);
                ServStartA(SFV_SERVICE_NAME);
            }
        }

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(szServ))
        {
            break;
        }

        HANDLE hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
        HandleAutoClose abc(hNotify);

        if (hNotify)
        {
            dp(L"服务进程已经启动");
            bStat = TRUE;
            break;
        }

        bServ = (InstallLocalServiceA(szServ, "-service", SFV_SERVICE_NAME, SFV_SERVICE_DISPLAY_NAME, SFV_SERVICE_DESCRIPTION) && StartLocalServiceA(SFV_SERVICE_NAME));
        dp(L"stat:%d, err:%d", bServ, GetLastError());
        if (bServ)
        {
            hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
            DWORD dwTimeCount = GetTickCount();
            while (GetTickCount() - dwTimeCount < 5000)
            {
                if (hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME))
                {
                    bStat = TRUE;
                    break;
                }
                Sleep(10);
            }
        }
        else
        {
            dp(L"启动SfvServ服务失败:%d", GetLastError());
        }
    } while (FALSE);
    return bStat;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    g_hInstance = m;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    LoadLibraryW(L"ComLib32.dll");
    LoadLibraryW(L"mq32.dll");
    LoadLibraryW(L"DbgCtrl32.dll");

    DbProxy::GetInstance()->InitDbEnv();

    _StartService();
    DbgCtrlService::GetInstance()->InitCtrlService();
    _StartViewProc();
    WSACleanup();
    return 0;
}