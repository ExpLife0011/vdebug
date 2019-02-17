#include <Windows.h>
#include <Shlwapi.h>
#include <DbgCtrl/DbgCtrl.h>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include <runner/runner.h>
#include <DbgCtrl/DbgStat.h>
#include "ProcDbgProxy.h"
#include "DumpDbgProxy.h"
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

static void _KeepAlive(const mstring &unique) {
    while (TRUE) {
        HANDLE service = OpenEventA(EVENT_ALL_ACCESS, FALSE, FormatA(SERVICE_EVENT, unique.c_str()).c_str());

        if (!service)
        {
            break;
        }

        CloseHandle(service);
        Sleep(5000);
    }

    //clean cache
    SHDeleteValueA(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str());
}

static void _TestProc() {
    class TestProcMon : public ProcListener {
        virtual void OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed) {
            int dd = 1;
        }
    };

    ProcMonitor::GetInstance()->RegisterListener(new TestProcMon());
}

extern void TestProc();

int WINAPI WinMain(HINSTANCE hT, HINSTANCE hP, LPSTR szCmdLine, int iShow)
{
    char path[256];
    GetModuleFileNameA(NULL, path, 256);
#if WIN64 || _WIN64
    PathAppendA(path, "..\\..\\ComLib64.dll");
    LoadLibraryA(path);
    PathAppendA(path, "..\\mq64.dll");
    LoadLibraryA(path);
    PathAppendA(path, "..\\DbgCtrl64.dll");
    LoadLibraryA(path);
#else
    PathAppendA(path, "..\\..\\ComLib32.dll");
    LoadLibraryA(path);
    PathAppendA(path, "..\\mq32.dll");
    LoadLibraryA(path);
    PathAppendA(path, "..\\DbgCtrl32.dll");
    LoadLibraryA(path);
#endif
    TestProc();
    return 0;

    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);

    if (count != 2)
    {
        LocalFree(args);
        return 0;
    }

    mstring cmd = WtoA(args[1]);
    LocalFree(args);
    if (!cmd.startwith("dbg"))
    {
        return 0;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    CSymbolHlpr::GetInst()->InitSymbol("SRV*F:\\mysymbol*http://msdl.microsoft.com/download/symbols/", GetCurrentProcess());
    size_t pos = cmd.rfind('_');
    mstring unique = cmd.substr(pos + 1, cmd.size() - pos - 1);

    CDbgStatMgr::GetInst()->InitStatMgr(unique);
    ProcDbgProxy::GetInstance()->InitProcDbgProxy(unique.c_str());
    DumpDbgProxy::GetInstance()->InitDumpDbgProxy(unique.c_str());
#ifdef _DEBUG
    MessageBoxA(0, "DbgTest", "dbg", 0);
#else
    _KeepAlive(unique);
#endif;
    WSACleanup();
    return 0;
}