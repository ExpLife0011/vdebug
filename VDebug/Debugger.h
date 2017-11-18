#ifndef DEBUGGER_VDEBUG_H_H_
#define DEBUGGER_VDEBUG_H_H_
#include <Windows.h>
#include <TlHelp32.h>
#include <DbgHelp.h>
#include <map>
#include "mstring.h"
#include "TitanEngine/TitanEngine.h"
#include "DbgBase.h"

using namespace std;

enum DebuggerType
{
    em_debugger_unconnect,
    em_debugger_attach,
    em_debugger_open,
    em_debugger_dump
};

typedef void (__stdcall *pfnDebuggerStart)();

struct DebuggerInfo
{
    DebuggerType m_eType;
    ustring m_wstrPePath;
    ustring m_wstrCmd;
    ustring m_wstrCurrentDir;
    DWORD m_dwPid;
    HANDLE m_hProcess;
    BOOL m_bIsx64Proc;
    pfnDebuggerStart m_pfn;

    DebuggerInfo()
    {
        m_eType = em_debugger_attach;
        m_dwPid = 0;
        m_pfn = NULL;
        m_hProcess = NULL;
        m_bIsx64Proc;
    }
};

struct ThreadInfo
{
    DWORD m_dwThreadNum;
    HANDLE m_hThread;
    DWORD m_dwThreadId;
    DWORD64 m_dwStartAddr;
    DWORD64 m_dwLocalBase;
    ustring m_wstrName;

    ThreadInfo()
    {
        m_dwThreadNum = 0;
        m_hThread = NULL;
        m_dwThreadId = 0;
        m_dwStartAddr = 0;
        m_dwLocalBase = 0;
    }
};

struct ModuleProcInfo
{
    ustring m_wstrName;     //名称
    DWORD64 m_dwBaseAddr;   //模块基址
    DWORD64 m_dwProcAddr;   //函数名称
    DWORD64 m_dwType;       //类型

    ModuleProcInfo()
    {
        m_dwBaseAddr = 0;
        m_dwProcAddr = 0;
        m_dwType = 0;
    }
};

struct ModuleInfo
{
    ustring m_wstrDllPath;      //dll路径
    ustring m_wstrDllName;      //dll名称
    DWORD64 m_dwBaseOfImage;    //dll基址
    DWORD64 m_dwEndAddr;        //模块结束地址
    DWORD64 m_dwModuleSize;     //模块大小
    HMODULE m_hModule;          //模块句柄
    map<DWORD64, ModuleProcInfo> m_vProcInfo;   //函数信息

    ModuleInfo()
    {
        m_dwBaseOfImage = 0;
        m_dwEndAddr = 0;
        m_dwModuleSize = 0;
        m_hModule = NULL;
    }
};

class CDebuggerEngine
{
public:
    VOID InitEngine();

    bool OpenExe(LPCWSTR wszPeName, LPCWSTR wszCommand = NULL, LPCWSTR wszCurrentDir = NULL);

    VOID Attach(DWORD dwPid);

    VOID Detach();

    VOID Wait();

    VOID Run();

    DebuggerStatus Status();

    TITAN_ENGINE_CONTEXT_t GetCurrentContext();

    DEBUG_EVENT *GetDebugProcData();

    ustring GetSymFromAddr(DWORD64 dwAddr);

    ModuleInfo GetModuleFromAddr(DWORD64 dwAddr);

    DebuggerInfo GetDebuggerInfo();

    HANDLE GetThreadFromId(DWORD dwThread);

    HANDLE GetDebugProc();
protected:
    static void OnCreateProcess(CREATE_PROCESS_DEBUG_INFO* pCreateProcessInfo);
    static void OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess);
    static void OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread);
    static void OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread);
    static void OnSystemBreakpoint(void* ExceptionData);
    static void OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll);
    static void OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll);
    static void OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString);
    static void OnException(EXCEPTION_DEBUG_INFO* ExceptionData);
    static void OnDebugEvent(DEBUG_EVENT* DebugEvent);
    static void __stdcall CustomBreakPoint();

protected:
    static bool LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule);
    static void AddProcIndex(const ustring &wstrModuleName, const ustring &wstrProcName, LPVOID ptr);
    static void DeleteIndex(LPVOID ptr);

protected:
    static DWORD WINAPI DebugThread(LPVOID pParam);
    static BOOL WINAPI ModuleHandlerW(PMODULEENTRY32W, void *);
    static BOOL CALLBACK EnumSymbolProc(PSYMBOL_INFOW pSymInfo, ULONG uSymbolSize, PVOID pUserContext);

protected:
    static DebuggerInfo ms_DebugInfo;
    static DWORD ms_dwCurDebugProc;
    static map<DWORD, ThreadInfo> *ms_pThreadMap;
    static DWORD ms_dwCurrentThreadId;
    static map<DWORD64, ModuleInfo> *ms_pModuleMap;
    static HANDLE ms_hRunNotify;
    static DebuggerStatus ms_eDebuggerStatus;
};

CDebuggerEngine *GetDebugger();

#endif