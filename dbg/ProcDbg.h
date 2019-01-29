#ifndef PEOPENDBG_VDEBUG_H_H_
#define PEOPENDBG_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
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
    mstring m_strPePath;
    mstring m_strCmd;
    mstring m_strCurrentDir;
};

struct DbgProcInfo
{
    DbgProcType m_eType;
    mstring m_strPePath;
    mstring m_strCmd;
    mstring m_strCurrentDir;
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
    mstring m_strName;

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
    mstring m_strAddr;
    mstring m_strSymbol;
    ProcDbgBreakPointStat m_eStat;
};

class CProcDbgger : public CDbggerProxy
{
private:
    CProcDbgger();
public:
    static CProcDbgger *GetInstance();
    virtual ~CProcDbgger();

    virtual BOOL Connect(LPCSTR target, LPVOID pParam);
    virtual BOOL Connect(DWORD dwPid);
    virtual BOOL DisConnect();
    virtual BOOL IsConnect();
    virtual TITAN_ENGINE_CONTEXT_t GetCurrentContext();
    TITAN_ENGINE_CONTEXT_t GetThreadContext(HANDLE hThread);
    VOID Wait();
    void Run();
    DbgModuleInfo GetModuleFromAddr(DWORD64 dwAddr) const;
    mstring GetSymFromAddr(DWORD64 dwAddr) const;
    HANDLE GetDbgProc() const;
    DWORD GetCurDbgProcId() const;
    HANDLE GetCurrentThread();
    HANDLE GetThreadById(DWORD dwId) const;
    DbggerStatus GetDbggerStatus();

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
    void DeleteThreadById(DWORD dwId);
    DEBUG_EVENT *GetDebugProcData();
    bool LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule);
    void ResetCache();
    void OnDetachDbgger();
    virtual list<STACKFRAME64> GetStackFrame(const mstring &wstrParam);

protected:
    list<DbgProcThreadInfo> m_vThreadMap;
    map<DWORD64, DbgModuleInfo> m_vModuleInfo;
    DWORD m_dwCurDebugProc;
    DbgProcInfo m_vDbgProcInfo;
    DWORD m_dwCurrentThreadId;
    DbggerStatus m_eDbggerStat;
    HANDLE m_hRunNotify;
    static const DWORD ms_dwDefDisasmSize = 128;
    BOOL m_bDetachDbgger;

    //断点信息缓存
    vector<ProcDbgBreakPoint> m_vBreakPoint;
    DWORD m_dwLastBpSerial;

    //调试器对应的命令
protected:
    mstring GetStatusStr(ThreadStat eStat, ThreadWaitReason eWaitReason) const;
    void ClearBreakPoint(DWORD dwSerial = -1);
    bool IsBreakpointSet(DWORD64 dwAddr) const;
    bool DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize, mstring &data) const;
    bool DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr, mstring &data) const;
    bool DisassUntilRet(DWORD64 dwStartAddr, mstring &data) const;
    void GetDisassContentDesc(const mstring &wstrContent, mstring &data) const; //汇编指令着色

    virtual CmdReplyResult OnCommand(const mstring &cmd, const mstring &param, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBp(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBl(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBc(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdClear(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDisass(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdUb(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdUf(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdGo(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdGu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdKv(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDb(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDd(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdReg(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdScript(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdTs(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdTc(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdLm(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);

    static void __cdecl GuCmdCallback();
};
#endif