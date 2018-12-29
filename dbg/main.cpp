#include <Windows.h>
#include <Shlwapi.h>
#include <DbgCtrl/DbgCtrl.h>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include <runner/runner.h>
#include "ProcDbgProxy.h"
#include "symbol.h"
#include "procmon.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Dbghelp.lib")
#if _WIN64 || WIN64
#pragma comment(lib, "capstone/capstone_x64.lib")
#else
#pragma comment(lib, "capstone/capstone_x86.lib")
#endif

static void _KeepAlive(const ustring &unique) {
    while (TRUE) {
        HANDLE service = OpenEventW(EVENT_ALL_ACCESS, FALSE, FormatW(SERVICE_EVENT, unique.c_str()).c_str());

        if (!service)
        {
            break;
        }

        CloseHandle(service);
        Sleep(5000);
    }

    //clean cache
    SHDeleteValueW(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str());
}

static void _TestProc() {
    class TestProcMon : public ProcListener {
        virtual void OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed) {
            int dd = 1;
        }
    };

    ProcMonitor::GetInstance()->RegisterListener(new TestProcMon());
}

int WINAPI WinMain(HINSTANCE hT, HINSTANCE hP, LPSTR szCmdLine, int iShow)
{
    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);

    if (count != 2)
    {
        LocalFree(args);
        return 0;
    }

    ustring cmd = args[1];
    LocalFree(args);
    if (!cmd.startwith(L"runner"))
    {
        return 0;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    WCHAR path[256];
    GetModuleFileNameW(NULL, path, 256);
#if WIN64 || _WIN64
    PathAppendW(path, L"..\\..\\ComLib64.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\mq64.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\DbgCtrl64.dll");
    LoadLibraryW(path);
#else
    PathAppendW(path, L"..\\..\\ComLib32.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\mq32.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\DbgCtrl32.dll");
    LoadLibraryW(path);
#endif
    InitSymbolHlpr(L"H:\\mysymbols");

    size_t pos = cmd.rfind('_');
    ustring unique = cmd.substr(pos + 1, cmd.size() - pos - 1);

    ProcDbgProxy::GetInstance()->InitProcDbgProxy(unique.c_str());
#ifdef _DEBUG
    MessageBoxA(0, "DbgTest", "dbg", 0);
#else
    _KeepAlive(unique);
#endif;
    WSACleanup();
    return 0;
}