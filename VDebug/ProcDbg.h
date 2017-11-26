#ifndef PEOPENDBG_VDEBUG_H_H_
#define PEOPENDBG_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include "mstring.h"
#include "DbgBase.h"
#include "CmdBase.h"
#include "DbgProxy.h"
#include "TitanEngine/TitanEngine.h"
#include "SyntaxDescHlpr.h"

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

enum ProcDbgBreakPointStat
{
    em_bp_enable,       //断点启用
    em_bp_disable,      //断点禁用
    em_bp_uneffect      //断点尚未生效
};

//断点信息
struct ProcDbgBreakPoint
{
    DWORD m_dwSerial;
    DWORD64 m_dwBpAddr;
    ustring m_wstrAddr;
    ustring m_wstrSymbol;
    ProcDbgBreakPointStat m_eStat;
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
    DbgModuleInfo GetModuleFromAddr(DWORD64 dwAddr) const;
    ustring GetSymFromAddr(DWORD64 dwAddr) const;
    HANDLE GetDbgProc() const;
    HANDLE GetCurrentThread();

protected:
    //读写调试进程内存
    static DWORD __stdcall ReadDbgProcMemory(IN DWORD64 dwAddr, IN DWORD dwReadLength, OUT char *pBuffer);
    static DWORD __stdcall WriteDbgProcMemory(IN DWORD64 dwAddr, IN DWORD dwWriteLength, IN const char *pBuffer);

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
    static DWORD64 CALLBACK GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr);
    static DWORD64 CALLBACK StackTranslateAddressProc64(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr);

protected:
    DEBUG_EVENT *GetDebugProcData();
    HANDLE GetThreadHandle(DWORD dwThreadId);
    bool LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule);
    void ResetCache();
    virtual list<STACKFRAME64> GetStackFrame(const ustring &wstrParam);

protected:
    map<DWORD, DbgProcThreadInfo> m_vThreadMap;
    map<DWORD64, DbgModuleInfo> m_vModuleInfo;
    DWORD m_dwCurDebugProc;
    DbgProcInfo m_vDbgProcInfo;
    DWORD m_dwCurrentThreadId;
    DebuggerStatus m_eDbggerStat;
    HANDLE m_hRunNotify;
    static const DWORD ms_dwDefDisasmSize = 128;

    //断点信息缓存
    vector<ProcDbgBreakPoint> m_vBreakPoint;
    DWORD m_dwLastBpSerial;

    //调试器对应的命令
protected:
    void ClearBreakPoint(DWORD dwSerial = -1);
    bool IsBreakpointSet(DWORD64 dwAddr) const;
    bool DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize) const;
    bool DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr) const;
    bool DisassUntilRet(DWORD64 dwStartAddr) const;
    void GetDisassContentDesc(const ustring &wstrContent, CSyntaxDescHlpr &hlpr) const; //汇编指令着色
    virtual DbgCmdResult OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdBp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdBl(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdBc(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdBu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdClear(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDisass(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdUb(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdUf(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdGo(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdGu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdKv(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDb(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDd(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdDu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdReg(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    DbgCmdResult OnCmdScript(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
};

CProcDbgger *GetProcDbgger();
#endif