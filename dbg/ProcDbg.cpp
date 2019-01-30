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
    m_dwLastBpSerial = 0;
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
    //SetCmdNotify(em_dbg_status_free, mstring().format(L"线程 %04x >>", dwId).c_str());
    WaitForSingleObject(m_hRunNotify, INFINITE);
    //SetCmdNotify(em_dbg_status_busy, mstring().format(L"线程 %04x >>", dwId).c_str());
}

void CProcDbgger::Run()
{
    //SetCmdNotify(em_dbg_status_busy, L"正在运行");
    SetEvent(m_hRunNotify);
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
    m_vModuleInfo.clear();
    m_dwCurrentThreadId = 0;
    m_eDbggerStat = em_dbg_status_init;
    m_vBreakPoint.clear();
    m_dwLastBpSerial = 0;
    m_bDetachDbgger = FALSE;
}

DWORD CProcDbgger::DebugThread(LPVOID pParam)
{
    if (em_dbgproc_attach == GetInstance()->m_vDbgProcInfo.m_eType)
    {
        DWORD dwPid = GetInstance()->m_vDbgProcInfo.m_dwPid;
        GetInstance()->m_vDbgProcInfo.m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
        AttachDebugger(dwPid, true, NULL, NULL);
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
    for (map<DWORD64, DbgModuleInfo>::const_iterator it = ptr->m_vModuleInfo.begin() ; it != ptr->m_vModuleInfo.end() ; it++)
    {
        if (dwAddr >= it->second.m_dwBaseOfImage && dwAddr <= it->second.m_dwEndAddr)
        {
            return it->second.m_dwBaseOfImage;
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

DbgModuleInfo CProcDbgger::GetModuleFromAddr(DWORD64 dwAddr) const
{
    for (map<DWORD64, DbgModuleInfo>::const_iterator it = m_vModuleInfo.begin() ; it != m_vModuleInfo.end() ; it++)
    {
        if (dwAddr >= it->second.m_dwBaseOfImage && dwAddr <= it->second.m_dwEndAddr)
        {
            return it->second;
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

mstring CProcDbgger::GetSymFromAddr(DWORD64 dwAddr) const
{
    CTaskSymbolFromAddr task;
    task.m_dwAddr = dwAddr;
    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskSymbolFromAddr) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_strfromaddr;

    DbgModuleInfo module = GetModuleFromAddr(dwAddr);
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

    mstring event = MakeDbgEvent(DBG_EVENT_DBG_PROC_CREATEA, EncodeProcCreate(info));
    MsgSend(MQ_CHANNEL_DBG_SERVER, event.c_str());;

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

    ////CSyntaxDescHlpr hlpr;
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

    mstring package = MakeDbgEvent(DBG_EVENT_SYSTEM_BREAKPOINTA, FormatA("{\"tid\":%d}", dwId));
    MsgSend(MQ_CHANNEL_DBG_SERVER, package.c_str());

    //脱离调试器
    if (GetInstance()->m_bDetachDbgger)
    {
        DetachDebuggerEx(GetInstance()->m_vDbgProcInfo.m_dwPid);
        GetInstance()->OnDetachDbgger();
        return;
    }

    //GetInstance()->RunCommand("r");
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
    m_vModuleInfo[dwBaseOfModule] = loadInfo.m_ModuleInfo;

    DllLoadInfo dllInfo;
    dllInfo.mDllName = loadInfo.m_ModuleInfo.m_strDllName;
    dllInfo.mBaseAddr = FormatA("0x%p", loadInfo.m_ModuleInfo.m_dwBaseOfImage);
    dllInfo.mEndAddr = FormatA("0x%p", loadInfo.m_ModuleInfo.m_dwEndAddr);

    utf8_mstring package = MakeDbgEvent(DBG_EVENT_MODULE_LOADA, EncodeDllLoadInfo(dllInfo));
    MsgSend(MQ_CHANNEL_DBG_SERVER, package.c_str());
    return true;
}

void CProcDbgger::OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll)
{
    GetInstance()->LoadModuleInfo(LoadDll->hFile, (DWORD64)LoadDll->lpBaseOfDll);
}

void CProcDbgger::OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll)
{
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

CmdReplyResult CProcDbgger::OnCommand(const mstring &cmd, const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    if (cmd == "bp")
    {
        return OnCmdBp(cmdParam, mode, pParam);
    }
    else if (cmd == "b")
    {
        return OnCmdBl(cmdParam, mode, pParam);
    }
    else if (cmd == "bk")
    {
        DebugBreakProcess(GetInstance()->GetDbgProc());
    }
    else if (cmd == "bc")
    {
        return OnCmdBc(cmdParam, mode, pParam);
    }
    //切换到指定线程
    else if (cmd == "tc")
    {
        return OnCmdTc(cmdParam, mode, pParam);
    }
    //展示指定线程
    else if (cmd == "ts")
    {
        return OnCmdTs(cmdParam, mode, pParam);
    }
    //展示模块信息
    else if (cmd == "lm")
    {
        return OnCmdLm(cmdParam, mode, pParam);
    }
    else if (cmd == "cls")
    {
        return OnCmdClear(cmdParam, mode, pParam);
    }
    else if (cmd == "u")
    {
        return OnCmdDisass(cmdParam, mode, pParam);
    }
    else if (cmd == "ub")
    {
        return OnCmdUb(cmdParam, mode, pParam);
    }
    else if (cmd == "uf")
    {
        return OnCmdUf(cmdParam, mode, pParam);
    }
    else if (cmd == "g")
    {
        return OnCmdGo(cmdParam, mode, pParam);
    }
    //执行到调用返回
    else if (cmd == "gu")
    {
        return OnCmdGu(cmdParam, mode, pParam);
    }
    else if (cmd == "kv")
    {
        return OnCmdKv(cmdParam, mode, pParam);
    }
    else if (cmd == "db")
    {
        return OnCmdDb(cmdParam, mode, pParam);
    }
    else if (cmd == "dd")
    {
        return OnCmdDd(cmdParam, mode, pParam);
    }
    else if (cmd == "du")
    {
        return OnCmdDu(cmdParam, mode, pParam);
    }
    else if (cmd == "r")
    {
        return OnCmdReg(cmdParam, mode, pParam);
    }
    else if (cmd == "sc")
    {
        return OnCmdScript(cmdParam, mode, pParam);
    }
    else if (cmd == "help" || cmd == "h")
    {
        return OnCmdHelp(cmdParam, mode, pParam);
    }

    return CmdReplyResult(0, mstring("不支持的命令:") + cmd + "\n", "");
}

bool CProcDbgger::IsBreakpointSet(DWORD64 dwAddr) const
{
    for (vector<ProcDbgBreakPoint>::const_iterator it = m_vBreakPoint.begin() ; it != m_vBreakPoint.end() ; it++)
    {
        if (it->m_dwBpAddr == dwAddr)
        {
            return true;
        }
    }
    return false;
}

CmdReplyResult CProcDbgger::OnCmdBp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    /*
    mstring result;
    //CSyntaxDescHlpr hlpr;
    mstring wstr(wstrCmdParam);
    wstr.trim();

    if (wstr.empty())
    {
        hlpr.FormatDesc(L"bp参数错误");
        result.SetResult(hlpr.GetResult());
        return result;
    }

    DWORD64 dwProcAddr = 0;
    if (!GetNumFromStr(wstr, dwProcAddr))
    {
        dwProcAddr = GetFunAddr(wstr);
    }

    if (dwProcAddr)
    {
        if (IsBreakpointSet(dwProcAddr))
        {
            hlpr.FormatDesc(L"地址已存在断点");
            return mstring(em_dbgstat_succ, hlpr.GetResult());
        }

        if (GetBreakPointMgr()->SetBreakPoint(dwProcAddr, pParam))
        {
            ProcDbgBreakPoint p;
            p.m_dwBpAddr = dwProcAddr;
            p.m_wstrSymbol = GetSymFromAddr(dwProcAddr);
            p.m_eStat = em_bp_enable;
            p.m_wstrAddr = FormatW(L"%I64d", dwProcAddr);
            p.m_dwSerial = m_dwLastBpSerial++;
            m_vBreakPoint.push_back(p);
            return mstring(em_dbgstat_succ, L"bp 执行成功");
        }
    }
    return mstring(em_dbgstat_faild, L"bp命令执行失败");
    */
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdBl(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    //CSyntaxDescHlpr hlpr;
    if (m_vBreakPoint.empty())
    {
        //return mstring(em_dbgstat_succ, "尚未设置任何断点");
    }

    //BOOL bx64 = GetCurrentDbgger()->IsDbgProcx64();
    /**
    hlpr.FormatDesc(L"断点序号  ", COLOUR_MSG);
    hlpr.FormatDesc(L"断点状态  ", COLOUR_MSG);

    if (bx64)
    {
        hlpr.FormatDesc(L"断点地址  ", COLOUR_MSG, 18);
    }
    else
    {
        hlpr.FormatDesc(L"断点地址  ", COLOUR_MSG);
    }
    hlpr.FormatDesc(L"符号位置", COLOUR_MSG);
    for (vector<ProcDbgBreakPoint>::const_iterator it = m_vBreakPoint.begin() ; it != m_vBreakPoint.end() ; it++)
    {
        hlpr.NextLine();
        hlpr.FormatDesc(FormatW(L"%02x", it->m_dwSerial), COLOUR_MSG, 10);
        switch (it->m_eStat)
        {
        case em_bp_enable:
            hlpr.FormatDesc(L"启用", COLOUR_MSG, 10);
            break;
        case em_bp_disable:
            hlpr.FormatDesc(L"禁用", COLOUR_MSG, 10);
            break;
        case em_bp_uneffect:
            hlpr.FormatDesc(L"未生效", COLOUR_MSG, 10);
            break;
        }

        if (bx64)
        {
            hlpr.FormatDesc(FormatW(L"%016llx", it->m_dwBpAddr), COLOUR_MSG, 18);
        }
        else
        {
            hlpr.FormatDesc(FormatW(L"%08x", it->m_dwBpAddr), COLOUR_MSG, 10);
        }
        hlpr.FormatDesc(it->m_wstrSymbol, COLOUR_MSG);
    }
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return CmdReplyResult();
}

void CProcDbgger::ClearBreakPoint(DWORD dwSerial)
{
    for (vector<ProcDbgBreakPoint>::const_iterator it = m_vBreakPoint.begin() ; it != m_vBreakPoint.end() ;)
    {
        if (-1 == dwSerial)
        {
            GetBreakPointMgr()->DeleteBp(it->m_dwBpAddr);
            it = m_vBreakPoint.erase(it);
        }
        else if (dwSerial == it->m_dwSerial)
        {
            GetBreakPointMgr()->DeleteBp(it->m_dwBpAddr);
            m_vBreakPoint.erase(it);
            return;
        }
    }
}

CmdReplyResult CProcDbgger::OnCmdBc(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CmdReplyResult tmp;
    mstring str(cmdParam);
    str.makelower();
    if (str == "*")
    {
        ClearBreakPoint(-1);
        tmp.mCmdShow = "已清空所有断点";
        return tmp;
    }

    if (!IsNumber(str))
    {
        tmp.mCmdCode = DBG_CMD_SYNTAX_ERR;
        tmp.mCmdShow = "bc 语法错误";
        return tmp;
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);
    ClearBreakPoint((DWORD)dwSerial);
    tmp.mCmdShow = FormatA("已清除%02x号断点", dwSerial);
    return tmp;
}

mstring CProcDbgger::GetStatusStr(ThreadStat eStat, ThreadWaitReason eWaitReason) const
{
    return "正常运行";
}

CmdReplyResult CProcDbgger::OnCmdTc(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.trim();

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    CmdReplyResult reply;

    DWORD dw = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++, dw++)
    {
        if (dwSerial == dw || dwSerial == it->m_dwThreadId)
        {
            m_dwCurrentThreadId = it->m_dwThreadId;
            reply.mCmdShow = FormatA("切换至%d号线程成功，当前线程%x", dw, it->m_dwThreadId);
            return reply;
        }
    }
    reply.mCmdShow = "未找到需要切换的线程";
    return reply;
}

CmdReplyResult CProcDbgger::OnCmdLm(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    /*
    //CSyntaxDescHlpr hlpr;

    BOOL bx64 = GetInstance()->IsDbgProcx64();
    DWORD dwLength = bx64 ? 20 : 12;
    hlpr.FormatDesc(L"起始位置", COLOUR_MSG, dwLength);
    hlpr.FormatDesc(L"结束位置", COLOUR_MSG, dwLength);
    hlpr.FormatDesc(L"模块名称", COLOUR_MSG, dwLength);

    for (map<DWORD64, DbgModuleInfo>::const_iterator it = m_vModuleInfo.begin() ; it != m_vModuleInfo.end() ; it++)
    {
        hlpr.NextLine();
        if (bx64)
        {
            hlpr.FormatDesc(FormatW(L"0x%016llx", it->second.m_dwBaseOfImage), COLOUR_MSG, dwLength);
            hlpr.FormatDesc(FormatW(L"0x%016llx", it->second.m_dwEndAddr), COLOUR_MSG, dwLength);
            hlpr.FormatDesc(FormatW(L"%ls", it->second.m_wstrDllName.c_str()));
        }
        else
        {
            hlpr.FormatDesc(FormatW(L"0x%08x", it->second.m_dwBaseOfImage), COLOUR_MSG, dwLength);
            hlpr.FormatDesc(FormatW(L"0x%08x", it->second.m_dwEndAddr), COLOUR_MSG, dwLength);
            hlpr.FormatDesc(FormatW(L"%ls", it->second.m_wstrDllName.c_str()));
        }
    }
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdTs(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring res;
    /*
    //CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"序号 ");
    hlpr.FormatDesc(L"线程ID", COLOUR_MSG, 12);
    hlpr.FormatDesc(L"启动时间", COLOUR_MSG, 25);
    hlpr.FormatDesc(L"状态", COLOUR_MSG, 10);
    hlpr.FormatDesc(L"启动位置");

    list<ThreadInformation> vThreads;
    GetThreadInformation(GetInstance()->GetDebugProcData()->dwProcessId, vThreads);
    int iIndex = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++, iIndex++)
    {
        for (list<ThreadInformation>::const_iterator itSingle = vThreads.begin() ; itSingle != vThreads.end() ; itSingle++)
        {
            if (it->m_dwThreadId == itSingle->m_dwThreadId)
            {
                hlpr.NextLine();
                hlpr.FormatDesc(FormatW(L"%02x", iIndex), COLOUR_MSG, 5);
                hlpr.FormatDesc(FormatW(L"%x:%d", it->m_dwThreadId, it->m_dwThreadId), COLOUR_MSG, 12);

                SYSTEMTIME time = {0};
                FileTimeToSystemTime(&(itSingle->m_vCreateTime), &time);
                hlpr.FormatDesc(
                    FormatW(
                    L"%04d-%02d-%02d %02d:%02d:%02d %03d ",
                    time.wYear,
                    time.wMonth,
                    time.wDay,
                    time.wHour,
                    time.wMinute,
                    time.wSecond,
                    time.wMilliseconds
                    ),
                    COLOUR_MSG,
                    25
                    );
                hlpr.FormatDesc(GetStatusStr(itSingle->m_eStat, itSingle->m_eWaitReason), COLOUR_MSG, 10);
                hlpr.FormatDesc(GetSymFromAddr(it->m_dwStartAddr), COLOUR_PROC);
            }
        }
    }
    res.SetResult(hlpr.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdBu(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    return CmdReplyResult();
}

void CProcDbgger::GetDisassContentDesc(const mstring &wstrContent, mstring &data) const
{
    /*
    mstring wstr(wstrContent);
    wstr.trim();
    while (mstring::npos != wstr.find(L" + "))
    {
        wstr.repsub(L" + ", L"+");
    }
    while (mstring::npos != wstr.find(L" - "))
    {
        wstr.repsub(L" - ", L"-");
    }

    vector<WordNode> v = GetWordSet(wstr);
    for (vector<WordNode>::const_iterator it = v.begin() ; it != v.end() ; it++)
    {
        if (IsRegister(it->m_wstrContent))
        {
            hlpr.FormatDesc(it->m_wstrContent, COLOUR_REGISTER);
        }
        else if (IsKeyword(it->m_wstrContent))
        {
            hlpr.FormatDesc(it->m_wstrContent, COLOUR_KEYWORD);
        }
        else if (IsNumber(it->m_wstrContent))
        {
            hlpr.FormatDesc(it->m_wstrContent, COLOUR_NUM);
        }
        else
        {
            hlpr.FormatDesc(it->m_wstrContent, COLOUR_MSG);
        }
    }
    */
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

CmdReplyResult CProcDbgger::OnCmdDisass(const mstring &wstrCmdParam, DWORD mode, const CmdUserParam *pParam)
{
    mstring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = 0;
    mstring strAddr;

    CmdReplyResult result;
    dwDisasmSize = GetSizeAndParam(wstr, strAddr);
    if (!dwDisasmSize)
    {
        result.mCmdCode = DBG_CMD_SYNTAX_ERR;
        result.mCmdShow = "语法错误";
        return result;
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (GetNumFromStr(strAddr, dwAddr))
    {
    }
    else
    {
        dwAddr = GetFunAddr(strAddr);
    }

    if (!dwAddr)
    {
        result.mCmdCode = DBG_CMD_READMEM_ERR;
        result.mCmdShow = FormatA("获取%hs地址失败", strAddr.c_str());
        return result;
    }
    /*
    //CSyntaxDescHlpr hlpr;
    DisassWithSize(dwAddr, dwDisasmSize, hlpr);
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return result;
}

CmdReplyResult CProcDbgger::OnCmdClear(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    //GetSyntaxView()->ClearView();
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdUb(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CmdReplyResult reply;
    if (!dwDisasmSize)
    {
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
        reply.mCmdShow = "获取反汇编地址长度失败";
        return reply;
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (!GetNumFromStr(strAddr, dwAddr))
    {
        dwAddr = GetFunAddr(strAddr);
    }

    if (!dwAddr)
    {
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
        reply.mCmdShow = "ub语法错误";
        return reply;
    }

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;
    ////CSyntaxDescHlpr hlpr;
    //mstring data;
    //DisassWithAddr(dwStartAddr, dwEndAddr, data);
    //return mstring(em_dbgstat_succ, hlpr.GetResult());
    return reply;
}

CmdReplyResult CProcDbgger::OnCmdUf(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CmdReplyResult reply;
    if (!dwDisasmSize)
    {
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
        return reply;
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (!GetNumFromStr(strAddr, dwAddr))
    {
        dwAddr = GetFunAddr(strAddr);
    }

    if (!dwAddr)
    {
        reply.mCmdCode = DBG_CMD_SYNTAX_ERR;
        return reply;
    }

    /*
    //CSyntaxDescHlpr hlpr;
    DisassUntilRet(dwAddr, hlpr);
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return reply;
}

CmdReplyResult CProcDbgger::OnCmdGo(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    Run();
    return CmdReplyResult();
}

void CProcDbgger::GuCmdCallback()
{
    HANDLE hThread = GetInstance()->GetCurrentThread();
    TITAN_ENGINE_CONTEXT_t context = GetInstance()->GetThreadContext(hThread);
    mstring strSymbol = GetInstance()->GetSymFromAddr(context.cip);
    //CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(FormatW(L"进程中断于%ls", wstrSymbol.c_str()));
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    GetInstance()->Wait();
}

CmdReplyResult CProcDbgger::OnCmdGu(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    StepOut(GuCmdCallback, true);
    Run();

    CmdReplyResult reply;
    reply.mCmdShow = "执行gu成功";
    return reply;
}

static BOOL CALLBACK StackReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    CMemoryOperator memory(hProcess);
    return memory.MemoryReadSafe(lpBaseAddress, (char *)lpBuffer, nSize, lpNumberOfBytesRead);
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

CmdReplyResult CProcDbgger::OnCmdKv(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    list<STACKFRAME64> vStack = GetInstance()->GetStackFrame(cmdParam);
    CmdReplyResult result;
    if (vStack.empty())
    {
        return result;
    }

    CallStackData callSet;
    CallStackSingle single;
    PrintFormater pf;
    pf << "内存地址" << "返回地址" << "参数列表" << space << space << space <<"符号名称" << line_end;
    for (list<STACKFRAME64>::const_iterator it = vStack.begin() ; it != vStack.end() ; it++)
    {
        single.mAddr = FormatA("%08x", it->AddrPC.Offset);
        single.mReturn = FormatA("%08x", it->AddrReturn);
        single.mParam0 = FormatA("%08x", it->Params[0]);
        single.mParam1 = FormatA("%08x", it->Params[1]);
        single.mParam2 = FormatA("%08x", it->Params[2]);
        single.mParam3 = FormatA("%08x", it->Params[3]);
        single.mFunction = FormatA("%hs", GetInstance()->GetSymFromAddr(it->AddrPC.Offset).c_str());
        callSet.mCallStack.push_back(single);

        pf << single.mAddr << single.mReturn << single.mParam0 << single.mParam1 << single.mParam2 << single.mParam3 << single.mFunction << line_end;
    }
    result.mCmdCode = 0;
    result.mResultMode = mode;
    result.mCmdResult = EncodeCmdCallStack(callSet);
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcDbgger::OnCmdDb(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    DWORD64 dwDataSize = 64;
    DWORD64 dwAddr = 0;
    mstring str(cmdParam);
    str.makelower();
    str.trim();
    mstring strAddr;
    if (str.startwith("l"))
    {
        size_t pos = str.find(" ");
        mstring wstrSize = str.substr(1, pos - 1);
        GetNumFromStr(wstrSize.c_str(), dwDataSize);
        strAddr = str.c_str() + pos;
    }
    else
    {
        strAddr = cmdParam;
    }
    strAddr.trim();

    CScriptEngine script;
    script.SetContext(GetInstance()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);
    dwAddr = script.Compile(strAddr);

    CmdReplyResult result;
    CMemoryOperator mhlpr(GetInstance()->GetDbgProc());
    for (int i = 0 ; i < dwDataSize ; i += 16)
    {
        DWORD dwReadSize = 0;
        if (dwDataSize < (i + 16))
        {
            dwReadSize = ((DWORD)dwDataSize - i);
        }
        else
        {
            dwReadSize = 16;
        }

        char szData[32] = {0};
        DWORD dwRead = 0;
        mhlpr.MemoryReadSafe(dwAddr, szData, dwReadSize, &dwRead);
        if (!dwRead)
        {
            break;
        }

        result.mCmdShow += FormatA("%08x  ", dwAddr);
        int j = 0;
        for (j = 0 ; j < 16 ; j++)
        {
            if (j < (int)dwRead)
            {
                result.mCmdShow += FormatA("%02x ", (BYTE)szData[j]);
            }
            else
            {
                result.mCmdShow += "   ";
            }
        }

        result.mCmdShow += " ";
        result.mCmdShow += GetPrintStr(szData, dwRead);
        result.mCmdShow += "\n";

        if (dwRead != dwReadSize)
        {
            break;
        }
        dwAddr += 16;
    }
    return result;
}

CmdReplyResult CProcDbgger::OnCmdDd(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(GetInstance()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(cmdParam);
    if (!dwAddr)
    {
        return CmdReplyResult();
    }

    DWORD dwDataSize = 64;
    /*
    //CSyntaxDescHlpr desc;
    CMemoryOperator mhlpr(GetInstance()->GetDbgProc());
    desc.FormatDesc(L"数据地址  ", COLOUR_MSG);
    desc.FormatDesc(L"数据内容", COLOUR_MSG);
    desc.NextLine();
    for (int i = 0 ; i < (int)dwDataSize ; i += 16)
    {
        char szData[16] = {0};
        DWORD dwReadSize = 0;
        mhlpr.MemoryReadSafe(dwAddr, szData, sizeof(szData), &dwReadSize);
        if (!dwReadSize)
        {
            break;
        }
        desc.FormatDesc(FormatW(L"%08x  ", dwAddr), COLOUR_ADDR);
        for (int j = 0 ; j < (int)dwReadSize / 4 ; j += 1)
        {
            desc.FormatDesc(FormatW(L"%08x ", *((DWORD *)szData + j)), COLOUR_DATA);
        }
        desc.NextLine();
        dwAddr += 16;
    }
    return mstring(em_dbgstat_succ, desc.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdDu(const mstring &strCmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(GetInstance()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        return CmdReplyResult();
    }

    CMemoryOperator mhlpr(GetInstance()->GetDbgProc());
    ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

    /*
    //CSyntaxDescHlpr desc;
    desc.FormatDesc(wstrData.c_str(), COLOUR_DATA);
    return mstring(em_dbgstat_succ, desc.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcDbgger::OnCmdReg(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    RegisterContent ctx;
    ctx.mContext = GetInstance()->GetCurrentContext();
    ctx.mCipStr = GetInstance()->GetSymFromAddr(ctx.mContext.cip).c_str();

    CmdReplyResult result;
    result.mCmdResult = EncodeCmdRegister(ctx);

    PrintFormater pf;
    pf << FormatA("eax=0x%08x", ctx.mContext.cax) << FormatA("ebx=0x%08x", ctx.mContext.cbx);
    pf << FormatA("ecx=0x%08x", ctx.mContext.ccx) << FormatA("edx=0x%08x", ctx.mContext.cdx) << line_end;

    pf << FormatA("esi=0x%08x", ctx.mContext.csi) << FormatA("edi=0x%08x", ctx.mContext.cdi);
    pf << FormatA("eip=0x%08x", ctx.mContext.cip) << FormatA("esp=0x%08x", ctx.mContext.csp) << line_end;

    pf << FormatA("ebp=0x%08x", ctx.mContext.cbp) << space << space << space << line_end;
    result.mCmdShow = pf.GetResult();
    pf.Reset();

    pf << ctx.mCipStr << line_end;
    result.mCmdShow += pf.GetResult();
    return result;
}

CmdReplyResult CProcDbgger::OnCmdScript(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    return CmdReplyResult();
}