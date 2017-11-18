#include <Windows.h>
#include "view.h"
#include "Debugger.h"
#include "Index.h"
#include "Script.h"
#include "minidump.h"
#include "symbol.h"

HINSTANCE g_hInstance = NULL;

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    InitSymbolHlpr(L"SRV*F:\\mysymbol*http://msdl.microsoft.com/download/symbols/");

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
    //return 0;

    CoInitialize(NULL);
    g_hInstance = m;
    ShowMainView();
    CoUninitialize();
    ExitProcess(0);     //直接返回可能长时间不能退出进程
    return 0;
}