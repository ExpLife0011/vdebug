#include <WinSock2.h>
#include <Windows.h>
#include "view.h"
#include "SyntaxHlpr/SyntaxParser.h"
#include <runner/runner.h>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include <Shlwapi.h>

#pragma comment(lib, "Ws2_32.lib")

HINSTANCE g_hInstance = NULL;

static void _TestProc() {
    //InitSymbolHlpr(L"SRV*F:\\mysymbol*http://msdl.microsoft.com/download/symbols/");
    //InitSymbolHlpr(L"F:\\mysymbol");

    //WCHAR wsz[MAX_PATH] = {0};
    //GetModuleFileNameW(NULL, wsz, MAX_PATH);
    //PathAppendW(wsz, L"..\\112233.dmp");
    //GetMiniDumpHlpr()->LodeDump(wsz);

    //list<DumpModuleInfo> modules;
    //GetMiniDumpHlpr()->GetModuleSet(modules);

    //list<DumpThreadInfo> threads;
    //GetMiniDumpHlpr()->GetThreadSet(threads);

    //DumpSystemInfo system;
    //GetMiniDumpHlpr()->GetSystemInfo(system);

    //list<DumpMemoryInfo> memorys;
    //GetMiniDumpHlpr()->GetMemoryInfo(memorys);

    //GetMiniDumpHlpr()->GetCallStack();

    //MessageBoxW(0, 0, 0, 0);
}

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

    SyntaxParser::GetInstance()->InitParser();
    CoInitialize(NULL);
    ShowMainView();
    CoUninitialize();
    WSACleanup();
    return TRUE;
}

static BOOL _StartService() {
    WCHAR wszRunner[MAX_PATH] = {0};
    WCHAR wszServ[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, wszRunner, MAX_PATH);
    PathAppendW(wszRunner, L"../runner.exe");

    GetWindowsDirectoryW(wszServ, MAX_PATH);
    PathAppendW(wszServ, L"DbgRunner.exe");

    BOOL bServ = TRUE;
    BOOL bStat = FALSE;
    HANDLE hNotify = NULL;
    do 
    {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wszServ))
        {
            CopyFileW(wszRunner, wszServ, FALSE);
        }
        else
        {
            if (!IsSameFileW(wszRunner, wszServ))
            {
                ustring wstrTemp(wszServ);
                ustring wstrName;
                wstrName.format(L"..\\DbgRunner%08x.tmp", GetTickCount());
                wstrTemp.path_append(wstrName.c_str());

                MoveFileExW(wszServ, wstrTemp.c_str(), MOVEFILE_REPLACE_EXISTING);
                MoveFileExW(wstrTemp.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                CopyFileW(wszRunner, wszServ, FALSE);
                ServStopW(SFV_SERVICE_NAME);
                ServStartW(SFV_SERVICE_NAME);
            }
        }

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wszServ))
        {
            break;
        }

        hNotify = OpenEventW(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);

        if (hNotify)
        {
            dp(L"服务进程已经启动");
            bStat = TRUE;
            break;
        }

        PathQuoteSpacesW(wszServ);
        ustring cmd = FormatW(L"%ls -service", wszServ);
        
        bServ = (InstallLocalService(cmd.c_str(), SFV_SERVICE_NAME, SFV_SERVICE_DISPLAY_NAME, SFV_SERVICE_DESCRIPTION) && StartLocalService(SFV_SERVICE_NAME));
        if (bServ)
        {
            hNotify = OpenEventW(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
            DWORD dwTimeCount = GetTickCount();
            while (GetTickCount() - dwTimeCount < 5000)
            {
                if (hNotify = OpenEventW(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME))
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

    if (hNotify)
    {
        CloseHandle(hNotify);
    }
    return bStat;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);

    g_hInstance = m;

    do
    {
        if (1 == count)
        {
            _StartService();

            if (!_StartViewProc())
            {
                break;
            }
        }
    } while (FALSE);

    LocalFree(args);
    return 0;
}