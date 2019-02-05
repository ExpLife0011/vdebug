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
    WaitForSingleObject(m_hRunNotify, INFINITE);

    EventDbgInfo eventInfo;
    eventInfo.mEventType = DBG_EVENT_DBG_PROC_RUNNING;
    MsgSend(MQ_CHANNEL_DBG_SERVER, MakeEventRequest(eventInfo).c_str());
}

void CProcDbgger::Run()
{
    SetEvent(m_hRunNotify);
}

void CProcDbgger::GuCmdCallback()
{
    HANDLE hThread = GetInstance()->GetCurrentThread();
    TITAN_ENGINE_CONTEXT_t context = GetInstance()->GetThreadContext(hThread);
    mstring strSymbol = GetInstance()->GetSymFromAddr((void *)context.cip);
    //CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(FormatW(L"进程中断于%ls", wstrSymbol.c_str()));
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
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
    CloseHandle(CreateThread(NULL, 0, DebugThread, NULL, 0, NULL));
    return TRUE;
}

BOOL CProcDbgger::Connect(DWORD dwPid)
{
    InitEngine();
    m_vDbgProcInfo.m_eType = em_dbgproc_attach;
    m_vDbgProcInfo.m_dwPid = dwPid;
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
    m_dwCurrentThreadId = 0;
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
    return GetThreadContext(GetCurrentThread());
}

TITAN_ENGINE_CONTEXT_t CProcDbgger::GetThreadContext(HANDLE hThread)
{
    TITAN_ENGINE_CONTEXT_t context = {0};
    GetFullContextDataEx(hThread, &context);
    return context;
}

HANDLE CProcDbgger::GetCurrentThread()
{
    HANDLE hThread = NULL;
    if (!(hThread = GetThreadById(m_dwCurrentThreadId)))
    {
        m_dwCurrentThreadId = m_vThreadMap.begin()->m_dwThreadId;
        hThread = GetThreadById(m_dwCurrentThreadId);
    }
    return hThread;
}

HANDLE CProcDbgger::GetThreadById(DWORD dwId) const
{
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++)
    {
        if (it->m_dwThreadId == dwId)
        {
            return it->m_hThread;
        }
    }
    return NULL;
}

DbggerStatus CProcDbgger::GetDbggerStatus() {
    return m_eDbggerStat;
}

list<DbgModuleInfo> CProcDbgger::GetModuleInfo() const {
    return mDllSet;
}

list<DbgProcThreadInfo> CProcDbgger::GetThreadCache() const {
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
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++)
    {
        if (it->m_dwThreadId == dwId)
        {
            m_vThreadMap.erase(it);
            return;
        }
    }
}

mstring CProcDbgger::GetSymFromAddr(void *dwAddr) const
{
    CTaskSymbolFromAddr task;
    task.m_dwAddr = (DWORD64)dwAddr;
    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskSymbolFromAddr) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_strfromaddr;

    DbgModuleInfo module = GetModuleFromAddr((DWORD64)dwAddr);
    if (module.m_strDllName.empty())
    {
        return "";
    }
    task.m_ModuleInfo = module;
    header.m_pParam = &task;
    GetSymbolHlpr()->SendTask(&header);

    mstring str = module.m_strDllName;
    size_t pos = str.rfind('.');
    if (mstring::npos != pos)
    {
        str.erase(pos, str.size() - pos);
    }
    return FormatA("%hs!%hs", str.c_str(), task.m_strSymbol.c_str());
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
    CMemoryOperator memory(GetInstance()->GetDbgProc());
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

    EventDbgInfo eventInfo;
    eventInfo.mEventType = DBG_EVENT_DBG_PROC_CREATE;
    PrintFormater pf;
    pf << "进程启动" << space                     << line_end;
    pf << "进程Pid"  << FormatA("%d", info.mPid) << line_end;
    pf << "映像路径" << info.mImage               << line_end;
    pf << "进程基址" << info.mBaseAddr            << line_end;
    pf << "入口地址" << info.mEntryAddr           << line_end;
    eventInfo.mEventShow = pf.GetResult();
    Reader().parse(EncodeProcCreate(info), eventInfo.mEventResult);
    MsgSend(MQ_CHANNEL_DBG_SERVER, MakeEventRequest(eventInfo).c_str());

    CSymbolTaskHeader task;
    CTaskSymbolInit param;
    param.m_hDstProc = pCreateProcessInfo->hProcess;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskSymbolInit);
    task.m_pParam = &param;
    task.m_eTaskType = em_task_initsymbol;

    GetSymbolHlpr()->SendTask(&task);

    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    DbgProcThreadInfo tp;
    tp.m_dwThreadId = dwId;
    tp.m_hThread = pCreateProcessInfo->hThread;
    tp.m_dwLocalBase = (DWORD64)pCreateProcessInfo->lpThreadLocalBase;
    tp.m_dwStartAddr = (DWORD64)pCreateProcessInfo->lpStartAddress;
    tp.m_strName = "主线程";

    (GetInstance()->m_vThreadMap).push_back(tp);
    GetInstance()->m_dwCurDebugProc = GetProcessId(pCreateProcessInfo->hProcess);

    GetInstance()->m_vDbgProcInfo.m_hProcess = pCreateProcessInfo->hProcess;
    GetInstance()->LoadModuleInfo(pCreateProcessInfo->hFile, (DWORD64)pCreateProcessInfo->lpBaseOfImage);
}

void CProcDbgger::OnDetachDbgger()
{
    GetInstance()->ResetCache();
    GetBreakPointMgr()->DeleteAllBp();
    CSymbolTaskHeader task;
    task.m_dwSize = sizeof(CSymbolTaskHeader);
    task.m_eTaskType = em_task_unloadall;
    GetSymbolHlpr()->SendTask(&task);
    //SetCmdNotify(em_dbg_status_init, L"初始状态");

    //CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(L"已脱离调试器", COLOUR_MSG);
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    m_eDbggerStat = em_dbg_status_init;
}

void CProcDbgger::OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess)
{
    ////CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(FormatW(L"调试进程退出,返回码:%08x", ExitProcess->dwExitCode), COLOUR_MSG);
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    GetInstance()->OnDetachDbgger();
}

void CProcDbgger::OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread)
{
    DbgProcThreadInfo newThread;
    newThread.m_dwThreadNum = (DWORD)GetInstance()->m_vThreadMap.size();
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)CreateThread->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)CreateThread->lpThreadLocalBase;
    newThread.m_hThread = CreateThread->hThread;
    GetInstance()->m_vThreadMap.push_back(newThread);
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
    GetInstance()->m_dwCurrentThreadId = dwId;
    HANDLE hThread = GetInstance()->GetThreadById(dwId);

    if (!hThread)
    {
        return;
    }

    EventDbgInfo eventInfo;
    eventInfo.mEventLabel = SCI_LABEL_DEFAULT;
    eventInfo.mEventType = DBG_EVENT_SYSTEM_BREAKPOINT;
    eventInfo.mEventResult["tid"] = (int)dwId;
    eventInfo.mEventShow = "系统断点触发调试器中断\n";
    MsgSend(MQ_CHANNEL_DBG_SERVER, MakeEventRequest(eventInfo).c_str());

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

    GetSymbolHlpr()->SendTask(&task);
    //两个结构完全一样，考虑到和dump可能有区别分别命名
    PushModule(loadInfo.m_ModuleInfo);

    DllLoadInfo dllInfo;
    dllInfo.mDllName = loadInfo.m_ModuleInfo.m_strDllName;
    dllInfo.mBaseAddr = FormatA("0x%08x", loadInfo.m_ModuleInfo.m_dwBaseOfImage);
    dllInfo.mEndAddr = FormatA("0x%08x", loadInfo.m_ModuleInfo.m_dwEndAddr);

    EventDbgInfo eventInfo;
    eventInfo.mEventType = DBG_EVENT_MODULE_LOAD;
    eventInfo.mEventLabel = SCI_LABEL_DEFAULT;
    eventInfo.mEventShow = FormatA("模块加载  %hs  %hs  %hs\n", dllInfo.mBaseAddr.c_str(), dllInfo.mEndAddr.c_str(), dllInfo.mDllName.c_str());

    MsgSend(MQ_CHANNEL_DBG_SERVER, MakeEventRequest(eventInfo).c_str());
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

    GetSymbolHlpr()->SendTask(&task);
    GetInstance()->EraseModule((DWORD64)UnloadDll->lpBaseOfDll);
}

void CProcDbgger::OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString)
{}

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

bool CProcDbgger::DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize, mstring &data) const
{
    /*
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    mstring wstr = GetSymFromAddr(dwAddr);
    wstr += L":";
    hlpr.NextLine();
    hlpr.FormatDesc(wstr, COLOUR_PROC);
    hlpr.NextLine();
    if (Disasm.DisasmWithSize(dwAddr, (DWORD)dwSize, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            if (GetCurrentDbgger()->IsDbgProcx64())
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 18);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 28);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            else
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        return true;
    }
    return false;
    */
    return true;
}

bool CProcDbgger::DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr, mstring &data) const
{
    /*
    if (dwEndAddr <= dwStartAddr)
    {
        return false;
    }

    DWORD64 dwSize = (dwEndAddr - dwStartAddr + 16);
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    mstring wstr = GetSymFromAddr(dwStartAddr);
    wstr += L":";
    hlpr.NextLine();
    hlpr.FormatDesc(wstr, COLOUR_PROC);
    hlpr.NextLine();
    if (Disasm.DisasmWithSize(dwStartAddr, (DWORD)dwSize, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            if (it->m_dwAddr > dwEndAddr)
            {
                break;
            }

            if (GetCurrentDbgger()->IsDbgProcx64())
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 18);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 28);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            else
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        return true;
    }
    */
    return false;
}

bool CProcDbgger::DisassUntilRet(DWORD64 dwStartAddr, mstring &data) const
{
    /*
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    mstring wstr = GetSymFromAddr(dwStartAddr);
    wstr += L":";
    hlpr.FormatDesc(wstr, COLOUR_PROC);
    hlpr.NextLine();
    if (Disasm.DisasmUntilReturn(dwStartAddr, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            if (GetCurrentDbgger()->IsDbgProcx64())
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 18);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 28);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            else
            {
                hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
                hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
                hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 16);
            }
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        return true;
    }
    return false;
    */
    return true;
}

static BOOL CALLBACK StackReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    CMemoryOperator memory(hProcess);
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
    HANDLE hCurrentThread = GetInstance()->GetCurrentThread();
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
    GetSymbolHlpr()->SendTask(&header);
    return info.m_FrameSet;
}