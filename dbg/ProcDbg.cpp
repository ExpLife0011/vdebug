#include <ComLib/ComLib.h>
#include <mq/mq.h>
#include <DbgCtrl/DbgCtrl.h>
#include "ProcDbg.h"
#include "TitanEngine/TitanEngine.h"
#include "symbol.h"
#include "BreakPoint.h"
#include "Disasm.h"
#include "Script.h"
#include "memory.h"
#include "DbgCommon.h"

#if WIN64 || _WIN64
#pragma comment(lib, "TitanEngine/TitanEngine_x64.lib")
#else
#pragma comment(lib, "TitanEngine/TitanEngine_x86.lib")
#endif

list<ThreadInformation> CProcDbgger::msCurThreadSet;

CProcDbgger *CProcDbgger::GetInstance()
{
    static CProcDbgger *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new CProcDbgger();
    }

    return s_ptr;
}

CProcDbgger::CProcDbgger()
{
    m_hRunNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_bDetachDbgger = FALSE;
    m_dwCurDebugProc = 0;
}

CProcDbgger::~CProcDbgger()
{}

VOID CProcDbgger::Wait()
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    m_eDbggerStat = em_dbg_status_free;

    DbgStat stat;
    stat.mDbggerType = em_dbg_proc86;
    stat.mCurTid = dwId;
    stat.mDbggerStatus = em_dbg_status_free;
    CDbgStatMgr::GetInst()->ReportDbgStatus(stat);

    ResetEvent(m_hRunNotify);
    WaitForSingleObject(m_hRunNotify, INFINITE);
    //重置当前线程
    GetInstance()->mCurThreadSet = false;

    stat.mDbggerStatus = em_dbg_status_busy;
    CDbgStatMgr::GetInst()->ReportDbgStatus(stat);

    m_eDbggerStat = em_dbg_status_busy;
}

void CProcDbgger::Run()
{
    SetEvent(m_hRunNotify);
}

void CProcDbgger::GuCmdCallback()
{
    HANDLE hThread = GetInstance()->GetCurrentThread().m_hThread;
    TITAN_ENGINE_CONTEXT_t context = GetInstance()->GetThreadContext(hThread);
    DbgModuleInfo module = GetInstance()->GetModuleFromAddr((DWORD64)context.cip);
    mstring strSymbol = CDbgCommon::GetSymFromAddr((DWORD64)context.cip, module.m_strDllName, module.m_dwBaseOfImage);

    Value result;
    result["addr"] = FormatA("0x%08x", (DWORD)context.cip);
    result["symbol"] = strSymbol;
    result["tid"] = (int)((DEBUG_EVENT*)GetDebugData())->dwThreadId;

    EventInfo eventInfo;
    eventInfo.mEvent = DBG_EVENT_USER_BREAKPOINT;
    eventInfo.mContent = result;
    eventInfo.mShow = FormatA("触发用户断点 %hs %hs\n", result["addr"].asString().c_str(), strSymbol.c_str());

    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());
    GetInstance()->Wait();
}

void CProcDbgger::RunExitProc() {
    StepOut(GuCmdCallback, true);
    Run();
}

BOOL CProcDbgger::Connect(LPCSTR target, LPVOID pParam)
{
    InitEngine();

    DbgProcUserContext *ptr = (DbgProcUserContext *)pParam;
    m_vDbgProcInfo.m_eType = em_dbgproc_open;
    m_vDbgProcInfo.m_dwPid = 0;
    m_vDbgProcInfo.m_strPePath = target;

    if (ptr)
    {
        m_vDbgProcInfo.m_strCmd = ptr->m_strCmd;
        m_vDbgProcInfo.m_strCurrentDir = ptr->m_strCurrentDir;
    }
    else
    {
        m_vDbgProcInfo.m_strCmd.clear();
        m_vDbgProcInfo.m_strCurrentDir.clear();
    }
    m_vDbgProcInfo.m_pfn = CustomBreakPoint;
    mCurThreadSet = false;
    CloseHandle(CreateThread(NULL, 0, DebugThread, NULL, 0, NULL));
    return TRUE;
}

BOOL CProcDbgger::Connect(DWORD dwPid)
{
    InitEngine();
    m_vDbgProcInfo.m_eType = em_dbgproc_attach;
    m_vDbgProcInfo.m_dwPid = dwPid;
    mCurThreadSet = false;
    CloseHandle(CreateThread(NULL, 0, DebugThread, NULL, 0, NULL));
    return TRUE;
}

BOOL CProcDbgger::DisConnect()
{
    if (!IsConnect())
    {
        return FALSE;
    }

    GetInstance()->GetDbgProc();
    m_bDetachDbgger = TRUE;
    DebugBreakProcess(GetInstance()->GetDbgProc());
    Run();
    m_dwCurDebugProc = 0;
    return TRUE;
}

BOOL CProcDbgger::IsConnect()
{
    return (0 != m_dwCurDebugProc);
}

void CProcDbgger::ResetCache()
{
    m_vThreadMap.clear();
    m_dwCurDebugProc = 0;
    m_vDbgProcInfo.Clear();
    mDllSet.clear();
    m_eDbggerStat = em_dbg_status_init;
    m_bDetachDbgger = FALSE;
}

void fCustomBreakPoint() {
    int ddd = 123;
}

DWORD CProcDbgger::DebugThread(LPVOID pParam)
{
    if (em_dbgproc_attach == GetInstance()->m_vDbgProcInfo.m_eType)
    {
        DWORD dwPid = GetInstance()->m_vDbgProcInfo.m_dwPid;
        GetInstance()->m_vDbgProcInfo.m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
        AttachDebugger(dwPid, true, NULL, fCustomBreakPoint);
    }
    else if (em_dbgproc_open == GetInstance()->m_vDbgProcInfo.m_eType)
    {
        LPCSTR szDir = NULL;
        if (!GetInstance()->m_vDbgProcInfo.m_strCurrentDir.empty())
        {
            szDir = GetInstance()->m_vDbgProcInfo.m_strCurrentDir.c_str();
        }

        PROCESS_INFORMATION *process = (PROCESS_INFORMATION *)InitDebug(
            GetInstance()->m_vDbgProcInfo.m_strPePath.c_str(),
            GetInstance()->m_vDbgProcInfo.m_strCmd.c_str(),
            NULL
            );

        if (!process)
        {
            GetInstance()->ResetCache();
            return 0;
        }

        BOOL bWowProc = TRUE;
        IsWow64Process(process->hProcess, &bWowProc);
        GetInstance()->m_bX64 = (!bWowProc);
        GetInstance()->m_vDbgProcInfo.m_dwPid = process->dwProcessId;
        GetInstance()->m_vDbgProcInfo.m_hProcess = process->hProcess;

        GetInstance()->m_eStatus = em_dbg_status_busy;
        DebugLoop();
    }
    return 0;
}

DWORD64 CProcDbgger::GetModuelBaseFromAddr(HANDLE hProcress, DWORD64 dwAddr)
{
    CProcDbgger *ptr = GetInstance();
    for (list<DbgModuleInfo>::const_iterator it = ptr->mDllSet.begin() ; it != ptr->mDllSet.end() ; it++)
    {
        if (dwAddr >= it->m_dwBaseOfImage && dwAddr <= it->m_dwEndAddr)
        {
            return it->m_dwBaseOfImage;
        }
    }
    return 0;
}

DWORD64 CProcDbgger::StackTranslateAddressProc64(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr)
{
    return 0;
}

void CProcDbgger::CustomBreakPoint()
{
    return;
}

DEBUG_EVENT *CProcDbgger::GetDebugProcData()
{
    return (DEBUG_EVENT *)GetDebugData();
}

TITAN_ENGINE_CONTEXT_t CProcDbgger::GetCurrentContext()
{
    if (mCurThreadSet) {
        return GetThreadContext(mCurrentThread.m_hThread);
    } else {
        DWORD tid = GetDebugProcData()->dwThreadId;
        return GetThreadContext(GetThreadById(tid).m_hThread);
    }
}

TITAN_ENGINE_CONTEXT_t CProcDbgger::GetThreadContext(HANDLE hThread)
{
    TITAN_ENGINE_CONTEXT_t context = {0};
    GetFullContextDataEx(hThread, &context);
    return context;
}

DbgProcThreadInfo CProcDbgger::GetCurrentThread()
{
    if (mCurThreadSet)
    {
        return mCurrentThread;
    }  else {
        DWORD tid = GetDebugProcData()->dwThreadId;
        return GetThreadById(tid);
    }
}

DbgProcThreadInfo CProcDbgger::GetThreadById(DWORD dwId) const
{
    for (vector<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++)
    {
        if (it->m_dwThreadId == dwId)
        {
            return *it;
        }
    }
    return DbgProcThreadInfo();
}

DbggerStatus CProcDbgger::GetDbggerStatus() {
    return m_eDbggerStat;
}

list<DbgModuleInfo> CProcDbgger::GetModuleInfo() const {
    return mDllSet;
}

vector<DbgProcThreadInfo> CProcDbgger::GetThreadCache() const {
    return m_vThreadMap;
}

void CProcDbgger::ThreadEnumCallBack(THREAD_ITEM_DATA *threadData) {
    ThreadInformation tmp;
    tmp.m_dwStartAddr = threadData->ThreadStartAddress;
    tmp.m_dwSwitchCount = threadData->ContextSwitches;
    tmp.m_dwTebBase = threadData->TebAddress;
    tmp.m_dwThreadId = threadData->dwThreadId;
    tmp.m_eStat = threadData->ThreadState;
    tmp.m_eWaitReason = threadData->WaitReason;
    tmp.m_Priority = threadData->Priority;

    FILETIME a, b, c, d= {0};
    GetThreadTimes(threadData->hThread, &a, &b, &c, &d);
    FileTimeToLocalFileTime(&a, &tmp.m_vCreateTime);
    msCurThreadSet.push_back(tmp);
}

list<ThreadInformation> CProcDbgger::GetCurrentThreadSet() const {
    msCurThreadSet.clear();
    ThreaderImportRunningThreadData(m_vDbgProcInfo.m_dwPid);
    ThreaderEnumThreadInfo(ThreadEnumCallBack);
    return msCurThreadSet;
}

DbgModuleInfo CProcDbgger::GetModuleFromAddr(DWORD64 dwAddr) const
{
    for (list<DbgModuleInfo>::const_iterator it = mDllSet.begin() ; it != mDllSet.end() ; it++)
    {
        if (dwAddr >= it->m_dwBaseOfImage && dwAddr <= it->m_dwEndAddr)
        {
            return *it;
        }
    }
    return DbgModuleInfo();
}

void CProcDbgger::DeleteThreadById(DWORD dwId)
{
    for (vector<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++)
    {
        if (it->m_dwThreadId == dwId)
        {
            m_vThreadMap.erase(it);
            return;
        }
    }
}

DWORD CProcDbgger::GetCurDbgProcId() const
{
    return m_dwCurDebugProc;
}

HANDLE CProcDbgger::GetDbgProc() const
{
    return m_vDbgProcInfo.m_hProcess;
}

//读调试进程内存
DWORD CProcDbgger::ReadDbgProcMemory(IN DWORD64 dwAddr, IN DWORD dwReadLength, OUT char *pBuffer)
{
    CMemoryProc memory(GetInstance()->GetDbgProc());
    DWORD dwReadSize = 0;
    memory.MemoryReadSafe(dwAddr, pBuffer, dwReadLength, &dwReadSize);
    return dwReadSize;
}

//写调试进程内存
DWORD CProcDbgger::WriteDbgProcMemory(IN DWORD64 dwAddr, IN DWORD dwWriteLength, IN const char *pBuffer)
{
    return 0;
}

void CProcDbgger::OnCreateProcess(CREATE_PROCESS_DEBUG_INFO* pCreateProcessInfo)
{
    ProcCreateInfo info;
    info.mPid = GetProcessId(pCreateProcessInfo->hProcess);
    info.mImage = GetInstance()->m_vDbgProcInfo.m_strPePath;
    info.mBaseAddr = FormatA("0x%p", pCreateProcessInfo->lpBaseOfImage);
    info.mEntryAddr = FormatA("0x%p", pCreateProcessInfo->lpStartAddress);

    GetInstance()->m_eStatus = em_dbg_status_busy;

    EventInfo eventInfo;
    eventInfo.mEvent = DBG_EVENT_DBG_PROC_CREATE;
    PrintFormater pf;
    pf << "进程启动" << space                     << line_end;
    pf << "进程Pid"  << FormatA("%d", info.mPid) << line_end;
    pf << "映像路径" << info.mImage               << line_end;
    pf << "进程基址" << info.mBaseAddr            << line_end;
    pf << "入口地址" << info.mEntryAddr           << line_end;
    eventInfo.mShow = pf.GetResult();
    Reader().parse(EncodeProcCreate(info), eventInfo.mContent);
    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());

    CSymbolTaskHeader task;
    CTaskSymbolInit param;
    param.m_hDstProc = pCreateProcessInfo->hProcess;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskSymbolInit);
    task.m_pParam = &param;
    task.m_eTaskType = em_task_initsymbol;

    CSymbolHlpr::GetInst()->SendTask(&task);

    DbgProcThreadInfo newThread;
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)pCreateProcessInfo->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)pCreateProcessInfo->lpThreadLocalBase;
    newThread.m_hThread = pCreateProcessInfo->hThread;

    FILETIME a, b, c, d= {0};
    GetThreadTimes(newThread.m_hThread, &a, &b, &c, &d);
    FileTimeToLocalFileTime(&a, &newThread.mStartTime);

    SYSTEMTIME time = {0};
    FileTimeToSystemTime(&a, &time);
    newThread.mStartTimeStr = FormatA(
        "%04d-%02d-%02d %02d:%02d:%02d %03d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds
        );

    dp(L"thread create:%d", newThread.m_dwThreadId);
    GetInstance()->PushThread(newThread);
    GetInstance()->m_dwCurDebugProc = GetProcessId(pCreateProcessInfo->hProcess);

    GetInstance()->m_vDbgProcInfo.m_hProcess = pCreateProcessInfo->hProcess;
    GetInstance()->LoadModuleInfo(pCreateProcessInfo->hFile, (DWORD64)pCreateProcessInfo->lpBaseOfImage);
}

void CProcDbgger::OnDetachDbgger()
{
    GetInstance()->ResetCache();
    GetBreakPointMgr()->DeleteAllBp();
    CSymbolHlpr::GetInst()->UnloadAll();

    EventInfo eventInfo;
    eventInfo.mEvent = DBG_EVENT_DETACH;
    eventInfo.mShow = "进程已脱离调试器";
    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());

    DbgStat stat;
    stat.mDbggerStatus = em_dbg_status_init;
    CDbgStatMgr::GetInst()->ReportDbgStatus(stat);

    m_eDbggerStat = em_dbg_status_init;
}

void CProcDbgger::OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess)
{
    GetInstance()->OnDetachDbgger();
}

void CProcDbgger::PushThread(const DbgProcThreadInfo &newThread) {
    for (vector<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++)
    {
        if (it->m_dwThreadId == newThread.m_dwThreadId)
        {
            return;
        }
    }

    m_vThreadMap.push_back(newThread);
}

void CProcDbgger::OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread)
{
    DbgProcThreadInfo newThread;
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)CreateThread->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)CreateThread->lpThreadLocalBase;
    newThread.m_hThread = CreateThread->hThread;

    FILETIME a, b, c, d= {0};
    GetThreadTimes(newThread.m_hThread, &a, &b, &c, &d);
    FileTimeToLocalFileTime(&a, &newThread.mStartTime);

    SYSTEMTIME time = {0};
    FileTimeToSystemTime(&a, &time);
    newThread.mStartTimeStr = FormatA(
        "%04d-%02d-%02d %02d:%02d:%02d %03d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds
        );

    dp(L"thread create:%d", newThread.m_dwThreadId);
    GetInstance()->PushThread(newThread);
}

void CProcDbgger::OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetInstance()->DeleteThreadById(dwId);
    return;
}

void CProcDbgger::OnSystemBreakpoint(void* ExceptionData)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetInstance()->mCurrentThread = GetInstance()->GetThreadById(dwId);
    HANDLE hThread = GetInstance()->mCurrentThread.m_hThread;

    if (!hThread)
    {
        return;
    }

    EventInfo eventInfo;
    eventInfo.mEvent = DBG_EVENT_SYSTEM_BREAKPOINT;
    eventInfo.mContent["tid"] = (int)dwId;
    eventInfo.mShow = "系统断点触发调试器中断\n";
    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());

    //脱离调试器
    if (GetInstance()->m_bDetachDbgger)
    {
        DetachDebuggerEx(GetInstance()->m_vDbgProcInfo.m_dwPid);
        GetInstance()->OnDetachDbgger();
        return;
    }

    GetInstance()->Wait();
}

bool CProcDbgger::LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule)
{
    CSymbolTaskHeader task;
    CTaskLoadSymbol loadInfo;
    loadInfo.m_dwBaseOfModule = dwBaseOfModule;
    loadInfo.m_hImgaeFile = hFile;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskLoadSymbol);
    task.m_pParam = &loadInfo;
    task.m_eTaskType = em_task_loadsym;

    CSymbolHlpr::GetInst()->SendTask(&task);
    //两个结构完全一样，考虑到和dump可能有区别分别命名
    PushModule(loadInfo.m_ModuleInfo);

    DllLoadInfo dllInfo;
    dllInfo.mDllName = loadInfo.m_ModuleInfo.m_strDllName;
    dllInfo.mBaseAddr = FormatA("0x%08x", loadInfo.m_ModuleInfo.m_dwBaseOfImage);
    dllInfo.mEndAddr = FormatA("0x%08x", loadInfo.m_ModuleInfo.m_dwEndAddr);

    EventInfo eventInfo;
    eventInfo.mEvent = DBG_EVENT_MODULE_LOAD;
    eventInfo.mShow = FormatA("模块加载  %hs  %hs  %hs\n", dllInfo.mBaseAddr.c_str(), dllInfo.mEndAddr.c_str(), dllInfo.mDllName.c_str());

    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());
    return true;
}

void CProcDbgger::OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll)
{
    GetInstance()->LoadModuleInfo(LoadDll->hFile, (DWORD64)LoadDll->lpBaseOfDll);
}

void CProcDbgger::OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll)
{
    CSymbolTaskHeader task;
    CTaskUnloadSymbol unLoadInfo;
    unLoadInfo.m_dwModuleAddr = (DWORD64)UnloadDll->lpBaseOfDll;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskUnloadSymbol);
    task.m_pParam = &unLoadInfo;
    task.m_eTaskType = em_task_unloadsym;

    CSymbolHlpr::GetInst()->SendTask(&task);
    GetInstance()->EraseModule((DWORD64)UnloadDll->lpBaseOfDll);
}

void CProcDbgger::OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString)
{}

void CProcDbgger::OnProgramException(EXCEPTION_DEBUG_INFO* ExceptionData) {
    mstring exceptionDesc = CDbgCommon::GetExceptionDesc(ExceptionData->ExceptionRecord.ExceptionCode);

    EventInfo eventInfo;
    eventInfo.mShow = "被调试进程因异常中断\n";
    PrintFormater pf;
    pf << "异常类型" << FormatA("0x%08x, %hs", ExceptionData->ExceptionRecord.ExceptionCode, exceptionDesc.c_str()) << line_end;

    if (ExceptionData->ExceptionRecord.ExceptionFlags == 0)
    {
        pf << "异常标识" << "可继续执行" << line_end;
    } else {
        pf << "异常标识" << "不能继续执行" << line_end;
    }
    pf << "关联异常" << FormatA("0x%08x", ExceptionData->ExceptionRecord.ExceptionRecord) << line_end;

    void *addr = ExceptionData->ExceptionRecord.ExceptionAddress;

    DbgModuleInfo module = GetInstance()->GetModuleFromAddr((DWORD64)addr);
    mstring symbol = CDbgCommon::GetSymFromAddr((DWORD64)addr, module.m_strDllName, module.m_dwBaseOfImage);
    pf << "异常地址" << FormatA("0x%08x %hs", (DWORD)addr, symbol.c_str()) << line_end;

    GetInstance()->m_eDbggerStat = em_dbg_status_free;
    eventInfo.mEvent = DBG_EVENT_EXCEPTION;
    eventInfo.mShow += pf.GetResult();
    eventInfo.mContent["tid"] = (int)((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    mstring package = MakeEvent(eventInfo);
    MsgSend(CHANNEL_PROC_SERVER, package.c_str());

    GetInstance()->Wait();
}

void CProcDbgger::OnException(EXCEPTION_DEBUG_INFO* ExceptionData)
{
    if (EXCEPTION_BREAKPOINT == ExceptionData->ExceptionRecord.ExceptionCode)
    {
        if (GetInstance()->m_bDetachDbgger)
        {
            DetachDebuggerEx(GetInstance()->m_vDbgProcInfo.m_dwPid);
            GetInstance()->OnDetachDbgger();
        } else {
            OnSystemBreakpoint(NULL);
        }
    } else {
        //异常信息
        OnProgramException(ExceptionData);
    }
}

void CProcDbgger::OnDebugEvent(DEBUG_EVENT* DebugEvent)
{
    int dd = 123;
}

VOID CProcDbgger::InitEngine()
{
    SetCustomHandler(UE_CH_CREATEPROCESS, (void *)OnCreateProcess);
    SetCustomHandler(UE_CH_EXITPROCESS, (void *)OnExitProcess);
    SetCustomHandler(UE_CH_CREATETHREAD, (void*)OnCreateThread);
    SetCustomHandler(UE_CH_EXITTHREAD, (void*)OnExitThread);
    SetCustomHandler(UE_CH_SYSTEMBREAKPOINT, (void*)OnSystemBreakpoint);
    SetCustomHandler(UE_CH_LOADDLL, (void*)OnLoadDll);
    SetCustomHandler(UE_CH_UNLOADDLL, (void*)OnUnloadDll);
    SetCustomHandler(UE_CH_OUTPUTDEBUGSTRING, (void*)OnOutputDebugString);
    SetCustomHandler(UE_CH_UNHANDLEDEXCEPTION, (void*)OnException);
    SetCustomHandler(UE_CH_DEBUGEVENT, (void*)OnDebugEvent);
}

bool CProcDbgger::DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize, CtrlReply &result) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;

    DbgModuleInfo module = GetInstance()->GetModuleFromAddr(dwAddr);
    mstring str = CDbgCommon::GetSymFromAddr(dwAddr, module.m_strDllName, module.m_dwBaseOfImage);
    str += ":";

    result.mShow = str + "\n";

    PrintFormater pf;
    if (Disasm.DisasmWithSize(dwAddr, (DWORD)dwSize, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            pf << it->mAddrStr << it->mByteCode << it->mOpt << it->mContent << line_end;
        }
    }
    result.mShow += pf.GetResult();
    return true;
}

bool CProcDbgger::DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr, CtrlReply &result) const
{
    if (dwEndAddr <= dwStartAddr)
    {
        return false;
    }

    DWORD64 dwSize = (dwEndAddr - dwStartAddr + 16);
    CDisasmParser Disasm(GetDbgProc());
    DbgModuleInfo module = GetModuleFromAddr(dwStartAddr);
    mstring str = CDbgCommon::GetSymFromAddr(dwStartAddr, module.m_strDllName, module.m_dwBaseOfImage);
    str += ":";
    result.mShow = str + "\n";

    vector<DisasmInfo> disasmSet;
    PrintFormater pf;
    if (Disasm.DisasmWithSize(dwStartAddr, (DWORD)dwSize, disasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = disasmSet.begin() ; it != disasmSet.end() ; it++)
        {
            if (it->mAddr > dwEndAddr)
            {
                break;
            }

            pf << it->mAddrStr << it->mByteCode << it->mOpt << it->mContent << line_end;
        }
    }
    result.mShow += pf.GetResult();
    return true;
}

bool CProcDbgger::DisassUntilRet(DWORD64 dwStartAddr, CtrlReply &result) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> disasmSet;
    DbgModuleInfo module = GetModuleFromAddr(dwStartAddr);
    mstring str = CDbgCommon::GetSymFromAddr(dwStartAddr, module.m_strDllName, module.m_dwBaseOfImage);
    str += ":";
    result.mShow = str + "\n";

    PrintFormater pf;
    if (!Disasm.DisasmUntilReturn(dwStartAddr, disasmSet))
    {
        return false;
    }

    for (vector<DisasmInfo>::const_iterator it = disasmSet.begin() ; it != disasmSet.end() ; it++)
    {
        pf << it->mAddrStr << it->mByteCode << it->mOpt << it->mContent << line_end;
    }
    result.mShow += pf.GetResult();
    return true;
}

static BOOL CALLBACK StackReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    CMemoryProc memory(hProcess);
    return memory.MemoryReadSafe(lpBaseAddress, (char *)lpBuffer, nSize, lpNumberOfBytesRead);
}

void CProcDbgger::PushModule(const DbgModuleInfo &dll) {
    mDllSet.push_back(dll);
}

void CProcDbgger::EraseModule(DWORD64 baseAddr) {
    for (list<DbgModuleInfo>::const_iterator it = mDllSet.begin() ; it != mDllSet.end() ;)
    {
        if (it->m_dwBaseOfImage == baseAddr)
        {
            it = mDllSet.erase(it);
        } else {
            it++;
        }
    }
}

list<DbgModuleInfo> CProcDbgger::GetDllSet() const {
    return mDllSet;
}

list<STACKFRAME64> CProcDbgger::GetStackFrame(const mstring &wstrParam)
{
    const int iMaxWalks = 1024;
    HANDLE hCurrentThread = GetInstance()->GetCurrentThread().m_hThread;
    CONTEXT context = {0};

    context.ContextFlags = CONTEXT_FULL;
    ::GetThreadContext(hCurrentThread, &context);
    STACKFRAME64 frame = {0};

#ifndef _WIN64
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
#else
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrPC.Offset = context.Rip;
    frame.AddrFrame.Offset = context.Rsp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#endif
    CTaskStackWalkInfo info;
    info.m_context = frame;
    info.m_dwMachineType = machineType;
    info.m_pfnGetModuleBaseProc = GetModuelBaseFromAddr;
    info.m_pfnReadMemoryProc = StackReadProcessMemoryProc64;
    info.m_pfnStackTranslateProc = StackTranslateAddressProc64;
    info.m_hDstProcess = GetInstance()->GetDbgProc();
    info.m_hDstThread = hCurrentThread;
    info.m_pThreadContext = &context;

    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskStackWalkInfo) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_stackwalk;
    header.m_pParam = &info;
    CSymbolHlpr::GetInst()->SendTask(&header);
    return info.m_FrameSet;
}