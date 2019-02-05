#ifndef PEOPENDBG_VDEBUG_H_H_
#define PEOPENDBG_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include "DbgBase.h"
#include "CmdBase.h"
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


class CProcDbgger : public CDbgBase
{
    friend class CProcCmd;
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
    void RunExitProc();
    DbgModuleInfo GetModuleFromAddr(DWORD64 dwAddr) const;
    mstring GetSymFromAddr(void *dwAddr) const;
    HANDLE GetDbgProc() const;
    DWORD GetCurDbgProcId() const;
    HANDLE GetCurrentThread();
    HANDLE GetThreadById(DWORD dwId) const;
    DbggerStatus GetDbggerStatus();
    list<DbgModuleInfo> GetModuleInfo() const;
    list<DbgProcThreadInfo> GetThreadCache() const;
    list<ThreadInformation> GetCurrentThreadSet() const;
    list<DbgModuleInfo> GetDllSet() const;

protected:
    static list<ThreadInformation> msCurThreadSet;
    static void __cdecl ThreadEnumCallBack(THREAD_ITEM_DATA *threadData);
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

    void PushModule(const DbgModuleInfo &dll);
    void EraseModule(DWORD64 baseAddr);
protected:
    list<DbgProcThreadInfo> m_vThreadMap;
    list<DbgModuleInfo> mDllSet;
    DWORD m_dwCurDebugProc;
    DbgProcInfo m_vDbgProcInfo;
    DWORD m_dwCurrentThreadId;
    DbggerStatus m_eDbggerStat;
    HANDLE m_hRunNotify;
    static const DWORD ms_dwDefDisasmSize = 128;
    BOOL m_bDetachDbgger;

    //调试器对应的命令
protected:
    bool DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize, CmdReplyResult &result) const;
    bool DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr, CmdReplyResult &result) const;
    bool DisassUntilRet(DWORD64 dwStartAddr, CmdReplyResult &result) const;

    static void __cdecl GuCmdCallback();

};
#endif