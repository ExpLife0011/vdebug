#ifndef PEOPENDBG_VDEBUG_H_H_
#define PEOPENDBG_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include "mstring.h"
#include "DbgBase.h"
#include "CmdBase.h"
#include "DbgProxy.h"
#include "TitanEngine/TitanEngine.h"

using namespace std;

enum DbgProcType
{
    em_dbgproc_attach,
    em_dbgproc_open
};

enum DbdProcStatus
{
    em_dbgproc_stat_init,
    em_dbgproc_stat_busy,
    em_dbgproc_stat_free
};

typedef void (__cdecl *pfnDebuggerStart1)();

struct DbgProcUserContext
{
    ustring m_wstrPePath;
    ustring m_wstrCmd;
    ustring m_wstrCurrentDir;
};

struct DbgProcInfo
{
    DbgProcType m_eType;
    ustring m_wstrPePath;
    ustring m_wstrCmd;
    ustring m_wstrCurrentDir;
    DWORD m_dwPid;
    HANDLE m_hProcess;
    BOOL m_bIsx64Proc;
    pfnDebuggerStart1 m_pfn;

    DbgProcInfo()
    {
        m_eType = em_dbgproc_attach;
        m_dwPid = 0;
        m_pfn = NULL;
        m_hProcess = NULL;
        m_bIsx64Proc = FALSE;
    }

    void Clear()
    {
        m_eType = em_dbgproc_attach;
        m_dwPid = 0;
        m_pfn = NULL;
        m_hProcess = NULL;
        m_bIsx64Proc = FALSE;
    }
};

struct DbgProcThreadInfo
{
    DWORD m_dwThreadNum;
    HANDLE m_hThread;
    DWORD m_dwThreadId;
    DWORD64 m_dwStartAddr;
    DWORD64 m_dwLocalBase;
    ustring m_wstrName;

    DbgProcThreadInfo()
    {
        m_dwThreadNum = 0;
        m_hThread = NULL;
        m_dwThreadId = 0;
        m_dwStartAddr = 0;
        m_dwLocalBase = 0;
    }
};

struct DbgProcModuleProcInfo
{
    ustring m_wstrName;     //名称
    DWORD64 m_dwBaseAddr;   //模块基址
    DWORD64 m_dwProcAddr;   //函数名称
    DWORD64 m_dwType;       //类型

    DbgProcModuleProcInfo()
    {
        m_dwBaseAddr = 0;
        m_dwProcAddr = 0;
        m_dwType = 0;
    }
};

struct DbgProcModuleInfo
{
    ustring m_wstrDllPath;      //dll路径
    ustring m_wstrDllName;      //dll名称
    DWORD64 m_dwBaseOfImage;    //dll基址
    DWORD64 m_dwEndAddr;        //模块结束地址
    DWORD64 m_dwModuleSize;     //模块大小
    HMODULE m_hModule;          //模块句柄
    map<DWORD64, DbgProcModuleProcInfo> m_vProcInfo;   //函数信息

    DbgProcModuleInfo()
    {
        m_dwBaseOfImage = 0;
        m_dwEndAddr = 0;
        m_dwModuleSize = 0;
        m_hModule = NULL;
    }
};

class CProcDbgger : public CDbggerProxy
{
public:
    CProcDbgger();
    virtual ~CProcDbgger();

    virtual BOOL Connect(LPCWSTR wszTarget, LPVOID pParam);
    virtual BOOL Connect(DWORD dwPid);
    virtual BOOL DisConnect();
    virtual BOOL IsConnect();
    virtual TITAN_ENGINE_CONTEXT_t GetCurrentContext();
    VOID Wait();
    void Run();
    DbgProcModuleInfo GetModuleFromAddr(DWORD64 dwAddr);
    ustring GetSymFromAddr(DWORD64 dwAddr);
    HANDLE GetDbgProc();

protected:
    static void __cdecl OnCreateProcess(CREATE_PROCESS_DEBUG_INFO* pCreateProcessInfo);
    static void __cdecl OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess);
    static void __cdecl OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread);
    static void __cdecl OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread);
    static void __cdecl OnSystemBreakpoint(void* ExceptionData);
    static void __cdecl OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll);
    static void __cdecl OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll);
    static void __cdecl OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString);
    static void __cdecl OnException(EXCEPTION_DEBUG_INFO* ExceptionData);
    static void __cdecl OnDebugEvent(DEBUG_EVENT* DebugEvent);
    static void __cdecl CustomBreakPoint();

public:
    static VOID InitEngine();
protected:
    static DWORD __stdcall DebugThread(LPVOID pParam);

protected:
    DEBUG_EVENT *GetDebugProcData();
    bool LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule);
    void ResetCache();

protected:
    map<DWORD, DbgProcThreadInfo> m_vThreadMap;
    map<DWORD64, DbgProcModuleInfo> m_vModuleInfo;
    DWORD m_dwCurDebugProc;
    DbgProcInfo m_vDbgProcInfo;
    DWORD m_dwCurrentThreadId;
    DebuggerStatus m_eDbggerStat;
    HANDLE m_hRunNotify;

    //调试器对应的命令
protected:
    virtual DbgCmdResult OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdBp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDisass(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdGo(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdKv(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdReg(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdScript(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
};

CProcDbgger *GetProcDbgger();
#endif