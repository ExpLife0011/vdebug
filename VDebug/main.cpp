#include <Windows.h>
#include "view.h"
#include "Script.h"
#include "minidump.h"
#include "symbol.h"

#pragma comment(lib, "Dbghelp.lib")

#if _WIN64 || WIN64
#pragma comment(lib, "capstone/capstone_x64.lib")
#else
#pragma comment(lib, "capstone/capstone_x86.lib")
#endif

HINSTANCE g_hInstance = NULL;

static ULONG_PTR __stdcall GetModuleBase(_In_ HANDLE hProcess, _In_ ULONG_PTR dwReturnAddress)
{
    IMAGEHLP_MODULE moduleInfo;
    moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

#ifdef _WIN64
    if (SymGetModuleInfo(hProcess, dwReturnAddress, &moduleInfo))
#else
    if (SymGetModuleInfo(hProcess, (ULONG)dwReturnAddress, &moduleInfo))
#endif
        return moduleInfo.BaseOfImage;
    else
    {
        MEMORY_BASIC_INFORMATION memoryBasicInfo;

        if (::VirtualQueryEx(hProcess, (LPVOID) dwReturnAddress,
            &memoryBasicInfo, sizeof(memoryBasicInfo)))
        {
            DWORD cch = 0;
            char szFile[MAX_PATH] = { 0 };

            cch = GetModuleFileNameA((HINSTANCE)memoryBasicInfo.AllocationBase,
                szFile, MAX_PATH);

            // Ignore the return code since we can't do anything with it.
            SymLoadModule(hProcess,
                NULL, ((cch) ? szFile : NULL),
#ifdef _WIN64
                NULL, (DWORD_PTR) memoryBasicInfo.AllocationBase, 0);
#else
                NULL, (DWORD)(DWORD_PTR)memoryBasicInfo.AllocationBase, 0);
#endif
            return (DWORD_PTR) memoryBasicInfo.AllocationBase;
        }
    }

    return 0;
}

HANDLE gs_hTestThread = 0;
DWORD gs_dwThreadId = 0;

static DWORD _TestThread(LPVOID pParam)
{
    gs_dwThreadId = GetCurrentThreadId();
    gs_hTestThread = GetCurrentThread();
    while (TRUE)
    {
        Sleep(3000);
    }
    return 0;
}

static DWORD _TestProc()
{
    gs_hTestThread = CreateThread(NULL, 0, _TestThread, NULL, 0, NULL);
    Sleep(3000);
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, gs_dwThreadId);
    SuspendThread(hThread);

    CONTEXT context = {0};
    context.ContextFlags = CONTEXT_CONTROL;
    GetThreadContext(hThread, &context);

    HANDLE hProcess = ::GetCurrentProcess();
    if (SymInitialize(hProcess, NULL, TRUE))
    {
        // force undecorated names to get params
        DWORD dw = SymGetOptions();
        dw &= ~SYMOPT_UNDNAME;
        SymSetOptions(dw);

        STACKFRAME stackFrame;
        memset(&stackFrame, 0, sizeof(stackFrame));
        stackFrame.AddrPC.Mode = AddrModeFlat;
        stackFrame.AddrFrame.Mode = AddrModeFlat;
        stackFrame.AddrStack.Mode = AddrModeFlat;
        stackFrame.AddrReturn.Mode = AddrModeFlat;
        stackFrame.AddrBStore.Mode = AddrModeFlat;

        DWORD dwMachType;

#if defined(_M_IX86)
        dwMachType                   = IMAGE_FILE_MACHINE_I386;

        // program counter, stack pointer, and frame pointer
        stackFrame.AddrPC.Offset     = context.Eip;
        stackFrame.AddrStack.Offset  = context.Esp;
        stackFrame.AddrFrame.Offset  = context.Ebp;
#elif defined(_M_AMD64)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_AMD64;
        stackFrame.AddrPC.Offset     = context.Rip;
#elif defined(_M_MRX000)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_R4000;
        stackFrame.AddrPC.Offset     = context.Fir;
#elif defined(_M_ALPHA)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_ALPHA;
        stackFrame.AddrPC.Offset     = (unsigned long) context.Fir;
#elif defined(_M_PPC)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_POWERPC;
        stackFrame.AddrPC.Offset     = context.Iar;
#elif defined(_M_IA64)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_IA64;
        stackFrame.AddrPC.Offset     = context.StIIP;
#elif defined(_M_ALPHA64)
        // only program counter
        dwMachType                   = IMAGE_FILE_MACHINE_ALPHA64;
        stackFrame.AddrPC.Offset     = context.Fir;
#else
#error("Unknown Target Machine");
#endif


        int nFrame;
        for (nFrame = 0; nFrame < 256; nFrame++)
        {
            if (!StackWalk(dwMachType, hProcess, hThread,
                &stackFrame, &context, NULL,
                SymFunctionTableAccess, GetModuleBase, NULL))
            {
                int eee = GetLastError();
                int fff = 1;
                break;
            }

            int ddd = 0;

            if (stackFrame.AddrPC.Offset != 0)
            {
            }
        }
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    //InitSymbolHlpr(L"SRV*F:\\mysymbol*http://msdl.microsoft.com/download/symbols/");
    InitSymbolHlpr(L"F:\\mysymbol");
    _TestProc();

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