#include "ProcDbg.h"
#include "TitanEngine/TitanEngine.h"
#include "view/SyntaxDescHlpr.h"
#include "common/common.h"
#include "view/MainView.h"
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

CProcDbgger *GetProcDbgger()
{
    static CProcDbgger *s_ptr = new CProcDbgger();
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
    SetCmdNotify(em_dbg_status_free, ustring().format(L"线程 %04x >>", dwId).c_str());
    WaitForSingleObject(m_hRunNotify, INFINITE);
    SetCmdNotify(em_dbg_status_busy, ustring().format(L"线程 %04x >>", dwId).c_str());
}

void CProcDbgger::Run()
{
    SetCmdNotify(em_dbg_status_busy, L"正在运行");
    SetEvent(m_hRunNotify);
}

BOOL CProcDbgger::Connect(LPCWSTR wszTarget, LPVOID pParam)
{
    InitEngine();

    DbgProcUserContext *ptr = (DbgProcUserContext *)pParam;
    m_vDbgProcInfo.m_eType = em_dbgproc_open;
    m_vDbgProcInfo.m_dwPid = 0;
    m_vDbgProcInfo.m_wstrPePath = wszTarget;

    if (ptr)
    {
        m_vDbgProcInfo.m_wstrCmd = ptr->m_wstrCmd;
        m_vDbgProcInfo.m_wstrCurrentDir = ptr->m_wstrCurrentDir;
    }
    else
    {
        m_vDbgProcInfo.m_wstrCmd.clear();
        m_vDbgProcInfo.m_wstrCurrentDir.clear();
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

    GetProcDbgger()->GetDbgProc();
    m_bDetachDbgger = TRUE;
    DebugBreakProcess(GetProcDbgger()->GetDbgProc());
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
    if (em_dbgproc_attach == GetProcDbgger()->m_vDbgProcInfo.m_eType)
    {
        DWORD dwPid = GetProcDbgger()->m_vDbgProcInfo.m_dwPid;
        GetProcDbgger()->m_vDbgProcInfo.m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
        AttachDebugger(dwPid, true, NULL, NULL);
    }
    else if (em_dbgproc_open == GetProcDbgger()->m_vDbgProcInfo.m_eType)
    {
        LPCWSTR wszDir = NULL;
        if (!GetProcDbgger()->m_vDbgProcInfo.m_wstrCurrentDir.empty())
        {
            wszDir = GetProcDbgger()->m_vDbgProcInfo.m_wstrCurrentDir.c_str();
        }

        PROCESS_INFORMATION *process = (PROCESS_INFORMATION *)InitDebugW(
            GetProcDbgger()->m_vDbgProcInfo.m_wstrPePath.c_str(),
            GetProcDbgger()->m_vDbgProcInfo.m_wstrCmd.c_str(),
            NULL
            );

        if (!process)
        {
            GetProcDbgger()->ResetCache();
            return 0;
        }

        BOOL bWowProc = TRUE;
        IsWow64Process(process->hProcess, &bWowProc);
        GetProcDbgger()->m_bX64 = (!bWowProc);
        GetProcDbgger()->m_vDbgProcInfo.m_dwPid = process->dwProcessId;
        GetProcDbgger()->m_vDbgProcInfo.m_hProcess = process->hProcess;
        DebugLoop();
    }
    return 0;
}

DWORD64 CProcDbgger::GetModuelBaseFromAddr(HANDLE hProcress, DWORD64 dwAddr)
{
    CProcDbgger *ptr = GetProcDbgger();
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

ustring CProcDbgger::GetSymFromAddr(DWORD64 dwAddr) const
{
    CTaskSymbolFromAddr task;
    task.m_dwAddr = dwAddr;
    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskSymbolFromAddr) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_strfromaddr;

    DbgModuleInfo module = GetModuleFromAddr(dwAddr);
    if (module.m_wstrDllName.empty())
    {
        return L"";
    }
    task.m_ModuleInfo = module;
    header.m_pParam = &task;
    GetSymbolHlpr()->SendTask(&header);

    ustring wstr = module.m_wstrDllName;
    size_t pos = wstr.rfind(L'.');
    if (ustring::npos != pos)
    {
        wstr.erase(pos, wstr.size() - pos);
    }
    return FormatW(L"%ls!%ls", wstr.c_str(), task.m_wstrSymbol.c_str());
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
    CMemoryOperator memory(GetProcDbgger()->GetDbgProc());
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
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"进程路径", COLOUR_MSG, 16);
    hlpr.FormatDesc(GetProcDbgger()->m_vDbgProcInfo.m_wstrPePath.c_str(), COLOUR_MSG);
    hlpr.NextLine();
    hlpr.FormatDesc(L"进程基地址", COLOUR_MSG, 16);
    hlpr.FormatDesc(FormatW(L"0x%08x", pCreateProcessInfo->lpBaseOfImage), COLOUR_MSG);
    hlpr.NextLine();
    hlpr.FormatDesc(L"进程入口地址", COLOUR_MSG, 16);
    hlpr.FormatDesc(FormatW(L"0x%08x", pCreateProcessInfo->lpStartAddress), COLOUR_MSG);
    hlpr.AddEmptyLine();
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());

    CSymbolTaskHeader task;
    CTaskSymbolInit param;
    param.m_hDstProc = pCreateProcessInfo->hProcess;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskSymbolInit);
    task.m_pParam = &param;
    task.m_eTaskType = em_task_initsymbol;

    GetSymbolHlpr()->SendTask(&task);

    DbgProcThreadInfo tp;
    tp.m_dwThreadId = dwId;
    tp.m_hThread = pCreateProcessInfo->hThread;
    tp.m_dwLocalBase = (DWORD64)pCreateProcessInfo->lpThreadLocalBase;
    tp.m_dwStartAddr = (DWORD64)pCreateProcessInfo->lpStartAddress;
    tp.m_wstrName = L"主线程";

    (GetProcDbgger()->m_vThreadMap).push_back(tp);
    GetProcDbgger()->m_dwCurDebugProc = GetProcessId(pCreateProcessInfo->hProcess);

    GetProcDbgger()->m_vDbgProcInfo.m_hProcess = pCreateProcessInfo->hProcess;
    GetProcDbgger()->LoadModuleInfo(pCreateProcessInfo->hFile, (DWORD64)pCreateProcessInfo->lpBaseOfImage);
}

void CProcDbgger::OnDetachDbgger()
{
    GetProcDbgger()->ResetCache();
    GetBreakPointMgr()->DeleteAllBp();
    CSymbolTaskHeader task;
    task.m_dwSize = sizeof(CSymbolTaskHeader);
    task.m_eTaskType = em_task_unloadall;
    GetSymbolHlpr()->SendTask(&task);
    SetCmdNotify(em_dbg_status_init, L"初始状态");

    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"已脱离调试器", COLOUR_MSG);
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
}

void CProcDbgger::OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess)
{
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(FormatW(L"调试进程退出,返回码:%08x", ExitProcess->dwExitCode), COLOUR_MSG);
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    GetProcDbgger()->OnDetachDbgger();
}

void CProcDbgger::OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread)
{
    DbgProcThreadInfo newThread;
    newThread.m_dwThreadNum = (DWORD)GetProcDbgger()->m_vThreadMap.size();
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)CreateThread->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)CreateThread->lpThreadLocalBase;
    newThread.m_hThread = CreateThread->hThread;
    GetProcDbgger()->m_vThreadMap.push_back(newThread);
}

void CProcDbgger::OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetProcDbgger()->DeleteThreadById(dwId);
    return;
}

void CProcDbgger::OnSystemBreakpoint(void* ExceptionData)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetProcDbgger()->m_dwCurrentThreadId = dwId;
    HANDLE hThread = GetProcDbgger()->GetThreadById(dwId);

    if (!hThread)
    {
        return;
    }

    //脱离调试器
    if (GetProcDbgger()->m_bDetachDbgger)
    {
        return;
    }
    GetProcDbgger()->RunCommand(L"r");
    GetProcDbgger()->Wait();
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
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"模块加载 ", COLOUR_MSG);

    if (GetCurrentDbgger()->IsDbgProcx64())
    {
        hlpr.FormatDesc(FormatW(L"0x%016llx 0x%016llx  ", loadInfo.m_ModuleInfo.m_dwBaseOfImage, loadInfo.m_ModuleInfo.m_dwEndAddr), COLOUR_MSG);
    }
    else
    {
        hlpr.FormatDesc(FormatW(L"0x%08x 0x%08x  ", loadInfo.m_ModuleInfo.m_dwBaseOfImage, loadInfo.m_ModuleInfo.m_dwEndAddr), COLOUR_MSG);
    }

    hlpr.FormatDesc(loadInfo.m_ModuleInfo.m_wstrDllName, COLOUR_MODULE);
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    return true;
}

void CProcDbgger::OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll)
{
    GetProcDbgger()->LoadModuleInfo(LoadDll->hFile, (DWORD64)LoadDll->lpBaseOfDll);
}

void CProcDbgger::OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll)
{
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"模块卸载", COLOUR_MSG, 10);
}

void CProcDbgger::OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString)
{}

void CProcDbgger::OnException(EXCEPTION_DEBUG_INFO* ExceptionData)
{
    if (EXCEPTION_BREAKPOINT == ExceptionData->ExceptionRecord.ExceptionCode)
    {
        if (GetProcDbgger()->m_bDetachDbgger)
        {
            DetachDebuggerEx(GetProcDbgger()->m_vDbgProcInfo.m_dwPid);
            GetProcDbgger()->OnDetachDbgger();
        }
    }
}

void CProcDbgger::OnDebugEvent(DEBUG_EVENT* DebugEvent)
{}

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

DbgCmdResult CProcDbgger::OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    if (wstrCmd == L"bp")
    {
        return OnCmdBp(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"bl")
    {
        return OnCmdBl(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"bk")
    {
        DebugBreakProcess(GetProcDbgger()->GetDbgProc());
    }
    else if (wstrCmd == L"bc")
    {
        return OnCmdBc(wstrCmdParam, bShow, pParam);
    }
    //切换到指定线程
    else if (wstrCmd == L"tc")
    {
        return OnCmdTc(wstrCmdParam, bShow, pParam);
    }
    //展示指定线程
    else if (wstrCmd == L"ts")
    {
        return OnCmdTs(wstrCmdParam, bShow, pParam);
    }
    //展示模块信息
    else if (wstrCmd == L"lm")
    {
        return OnCmdLm(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"cls")
    {
        return OnCmdClear(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"u")
    {
        return OnCmdDisass(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"ub")
    {
        return OnCmdUb(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"uf")
    {
        return OnCmdUf(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"g")
    {
        return OnCmdGo(wstrCmdParam, bShow, pParam);
    }
    //执行到调用返回
    else if (wstrCmd == L"gu")
    {
        return OnCmdGu(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"kv")
    {
        return OnCmdKv(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"db")
    {
        return OnCmdDb(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"dd")
    {
        return OnCmdDd(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"du")
    {
        return OnCmdDu(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"r")
    {
        return OnCmdReg(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"sc")
    {
        return OnCmdScript(wstrCmdParam, bShow, pParam);
    }
    else if (wstrCmd == L"help" || wstrCmd == L"h")
    {
        return OnCmdHelp(wstrCmdParam, bShow, pParam);
    }
    return res;
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

DbgCmdResult CProcDbgger::OnCmdBp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult result;
    CSyntaxDescHlpr hlpr;
    ustring wstr(wstrCmdParam);
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
            return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
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
            return DbgCmdResult(em_dbgstat_succ, L"bp 执行成功");
        }
    }
    return DbgCmdResult(em_dbgstat_faild, L"bp命令执行失败");
}

DbgCmdResult CProcDbgger::OnCmdBl(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CSyntaxDescHlpr hlpr;
    if (m_vBreakPoint.empty())
    {
        return DbgCmdResult(em_dbgstat_succ, L"尚未设置任何断点");
    }

    hlpr.FormatDesc(L"断点序号  ", COLOUR_MSG);
    hlpr.FormatDesc(L"断点状态  ", COLOUR_MSG);
    hlpr.FormatDesc(L"断点地址  ", COLOUR_MSG);
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
        hlpr.FormatDesc(FormatW(L"%08x", it->m_dwBpAddr), COLOUR_MSG, 10);
        hlpr.FormatDesc(it->m_wstrSymbol, COLOUR_MSG);
    }
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
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

DbgCmdResult CProcDbgger::OnCmdBc(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstr(wstrCmdParam);
    wstr.makelower();
    if (wstr == L"*")
    {
        ClearBreakPoint(-1);
        return DbgCmdResult(em_dbgstat_succ, L"已清空所有断点");
    }

    if (!IsNumber(wstr))
    {
        return DbgCmdResult(em_dbgstat_syntaxerr, L"bc 语法错误");
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(wstr, dwSerial);
    ClearBreakPoint((DWORD)dwSerial);
    return DbgCmdResult(em_dbgstat_succ, FormatW(L"已清除%02x号断点", dwSerial));
}

ustring CProcDbgger::GetStatusStr(ThreadStat eStat, ThreadWaitReason eWaitReason) const
{
    return L"正常运行";
}

DbgCmdResult CProcDbgger::OnCmdTc(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstr(wstrCmdParam);
    wstr.trim();
    if (wstr.empty())
    {
        return DbgCmdResult(em_dbgstat_faild, L"tc语法错误");
    }
    DWORD64 dwSerial = 0;
    GetNumFromStr(wstr, dwSerial);

    DWORD dw = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++, dw++)
    {
        if (dwSerial == dw || dwSerial == it->m_dwThreadId)
        {
            m_dwCurrentThreadId = it->m_dwThreadId;
            return DbgCmdResult(em_dbgstat_succ, FormatW(L"切换至%d号线程成功，当前线程%x", dw, it->m_dwThreadId));
        }
    }
    return DbgCmdResult(em_dbgstat_faild, L"未找到需要切换的线程");
}

DbgCmdResult CProcDbgger::OnCmdLm(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CSyntaxDescHlpr hlpr;

    BOOL bx64 = GetProcDbgger()->IsDbgProcx64();
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
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdTs(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"序号 ");
    hlpr.FormatDesc(L"线程ID", COLOUR_MSG, 12);
    hlpr.FormatDesc(L"启动时间", COLOUR_MSG, 25);
    hlpr.FormatDesc(L"状态", COLOUR_MSG, 10);
    hlpr.FormatDesc(L"启动位置");

    list<ThreadInformation> vThreads;
    GetThreadInformation(GetProcDbgger()->GetDebugProcData()->dwProcessId, vThreads);
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
    return res;
}

DbgCmdResult CProcDbgger::OnCmdBu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    return DbgCmdResult(em_dbgstat_succ);
}

void CProcDbgger::GetDisassContentDesc(const ustring &wstrContent, CSyntaxDescHlpr &hlpr) const
{
    ustring wstr(wstrContent);
    wstr.trim();
    while (ustring::npos != wstr.find(L" + "))
    {
        wstr.repsub(L" + ", L"+");
    }
    while (ustring::npos != wstr.find(L" - "))
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
}

bool CProcDbgger::DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize, CSyntaxDescHlpr &hlpr) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    ustring wstr = GetSymFromAddr(dwAddr);
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
}

bool CProcDbgger::DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr, CSyntaxDescHlpr &hlpr) const
{
    if (dwEndAddr <= dwStartAddr)
    {
        return false;
    }

    DWORD64 dwSize = (dwEndAddr - dwStartAddr + 16);
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    ustring wstr = GetSymFromAddr(dwStartAddr);
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
    return false;
}

bool CProcDbgger::DisassUntilRet(DWORD64 dwStartAddr, CSyntaxDescHlpr &hlpr) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    ustring wstr = GetSymFromAddr(dwStartAddr);
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
}

DbgCmdResult CProcDbgger::OnCmdDisass(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = 0;
    ustring wstrAddr;

    dwDisasmSize = GetSizeAndParam(wstr, wstrAddr);
    if (!dwDisasmSize)
    {
        return DbgCmdResult(em_dbgstat_syntaxerr);
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (GetNumFromStr(wstrAddr, dwAddr))
    {
    }
    else
    {
        dwAddr = GetFunAddr(wstrAddr);
    }

    if (!dwAddr)
    {
        return DbgCmdResult(em_dbgstat_faild, FormatW(L"获取%ls地址失败", wstrAddr.c_str()));
    }
    CSyntaxDescHlpr hlpr;
    DisassWithSize(dwAddr, dwDisasmSize, hlpr);
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdClear(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    GetSyntaxView()->ClearView();
    return DbgCmdResult(em_dbgstat_succ, L"");
}

DbgCmdResult CProcDbgger::OnCmdUb(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = ms_dwDefDisasmSize;
    ustring wstrAddr;

    dwDisasmSize = GetSizeAndParam(wstr, wstrAddr);
    if (!dwDisasmSize)
    {
        return DbgCmdResult(em_dbgstat_faild, L"获取反汇编地址长度失败");
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (!GetNumFromStr(wstrAddr, dwAddr))
    {
        dwAddr = GetFunAddr(wstrAddr);
    }

    if (!dwAddr)
    {
        return DbgCmdResult(em_dbgstat_syntaxerr, L"ub语法错误");
    }

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;
    CSyntaxDescHlpr hlpr;
    DisassWithAddr(dwStartAddr, dwEndAddr, hlpr);
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdUf(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = ms_dwDefDisasmSize;
    ustring wstrAddr;

    dwDisasmSize = GetSizeAndParam(wstr, wstrAddr);
    if (!dwDisasmSize)
    {
        return DbgCmdResult(em_dbgstat_faild, L"");
    }

    if (-1 == dwDisasmSize)
    {
        dwDisasmSize = ms_dwDefDisasmSize;
    }

    DWORD64 dwAddr = 0;
    if (!GetNumFromStr(wstrAddr, dwAddr))
    {
        dwAddr = GetFunAddr(wstrAddr);
    }

    if (!dwAddr)
    {
        return DbgCmdResult(em_dbgstat_syntaxerr);
    }

    CSyntaxDescHlpr hlpr;
    DisassUntilRet(dwAddr, hlpr);
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdGo(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    Run();
    return DbgCmdResult(em_dbgstat_succ, L"");
}

void CProcDbgger::GuCmdCallback()
{
    HANDLE hThread = GetProcDbgger()->GetCurrentThread();
    TITAN_ENGINE_CONTEXT_t context = GetProcDbgger()->GetThreadContext(hThread);
    ustring wstrSymbol = GetProcDbgger()->GetSymFromAddr(context.cip);
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(FormatW(L"进程中断于%ls", wstrSymbol.c_str()));
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    GetProcDbgger()->Wait();
}

DbgCmdResult CProcDbgger::OnCmdGu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    StepOut(GuCmdCallback, true);
    Run();
    return DbgCmdResult(em_dbgstat_succ, L"执行gu成功");
}

static BOOL CALLBACK StackReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    CMemoryOperator memory(hProcess);
    return memory.MemoryReadSafe(lpBaseAddress, (char *)lpBuffer, nSize, lpNumberOfBytesRead);
}

list<STACKFRAME64> CProcDbgger::GetStackFrame(const ustring &wstrParam)
{
    const int iMaxWalks = 1024;
    HANDLE hCurrentThread = GetProcDbgger()->GetCurrentThread();
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
    info.m_hDstProcess = GetProcDbgger()->GetDbgProc();
    info.m_hDstThread = hCurrentThread;
    info.m_pThreadContext = &context;

    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskStackWalkInfo) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_stackwalk;
    header.m_pParam = &info;
    GetSymbolHlpr()->SendTask(&header);
    return info.m_FrameSet;
}

DbgCmdResult CProcDbgger::OnCmdKv(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    list<STACKFRAME64> vStack = GetCurrentDbgger()->GetStackFrame(wstrCmdParam);
    if (vStack.empty())
    {
        return DbgCmdResult();
    }

    CSyntaxDescHlpr hlpr;
    hlpr.NextLine();
    hlpr.FormatDesc(L"返回地址 ", COLOUR_MSG, 17);
    hlpr.FormatDesc(L"参数列表 ", COLOUR_MSG, 17);
    hlpr.NextLine();
    for (list<STACKFRAME64>::const_iterator it = vStack.begin() ; it != vStack.end() ; it++)
    {
        hlpr.FormatDesc(FormatW(L"%016llx ", it->AddrPC.Offset), COLOUR_ADDR);
        for (int j = 0 ; j < 4 ; j++)
        {
            hlpr.FormatDesc(FormatW(L"%016llx ", it->Params[j]), COLOUR_PARAM);
        }

        hlpr.FormatDesc(FormatW(L"%ls", GetProcDbgger()->GetSymFromAddr(it->AddrPC.Offset).c_str()), COLOUR_PROC);
        hlpr.NextLine();
    }
    return DbgCmdResult(em_dbgstat_succ, hlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdDb(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DWORD64 dwDataSize = 64;
    DWORD64 dwAddr = 0;
    ustring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();
    ustring wstrAddr;
    if (wstr.startwith(L"l"))
    {
        size_t pos = wstr.find(L" ");
        ustring wstrSize = wstr.substr(1, pos - 1);
        GetNumFromStr(wstrSize.c_str(), dwDataSize);
        wstrAddr = wstr.c_str() + pos;
    }
    else
    {
        wstrAddr = wstrCmdParam;
    }
    wstrAddr.trim();

    CScriptEngine script;
    script.SetContext(GetProcDbgger()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);
    dwAddr = script.Compile(wstrAddr);

    CSyntaxDescHlpr desc;
    CMemoryOperator mhlpr(GetProcDbgger()->GetDbgProc());
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

        desc.FormatDesc(FormatW(L"%08x  ", dwAddr), COLOUR_ADDR);
        int j = 0;
        for (j = 0 ; j < 16 ; j++)
        {
            if (j < (int)dwRead)
            {
                desc.FormatDesc(FormatW(L"%02x ", (BYTE)szData[j]), COLOUR_HEX);
            }
            else
            {
                desc.FormatDesc(L"   ", COLOUR_MSG);
            }
        }

        desc.FormatDesc(L" ", COLOUR_MSG);
        desc.FormatDesc(FormatW(L"%hs", GetPrintStr(szData, dwRead).c_str()), COLOUR_DATA);
        desc.NextLine();

        if (dwRead != dwReadSize)
        {
            break;
        }
        dwAddr += 16;
    }
    return DbgCmdResult(em_dbgstat_succ, desc.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdDd(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(GetProcDbgger()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(wstrCmdParam);
    if (!dwAddr)
    {
        return DbgCmdResult();
    }

    DWORD dwDataSize = 64;
    CSyntaxDescHlpr desc;
    CMemoryOperator mhlpr(GetProcDbgger()->GetDbgProc());
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
    return DbgCmdResult(em_dbgstat_succ, desc.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdDu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(GetProcDbgger()->GetCurrentContext(), ReadDbgProcMemory, WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(wstrCmdParam);
    if (!dwAddr)
    {
        return DbgCmdResult();
    }

    CMemoryOperator mhlpr(GetProcDbgger()->GetDbgProc());
    ustring wstrData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

    CSyntaxDescHlpr desc;
    desc.FormatDesc(wstrData.c_str(), COLOUR_DATA);
    return DbgCmdResult(em_dbgstat_succ, desc.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdReg(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    TITAN_ENGINE_CONTEXT_t context = GetCurrentDbgger()->GetCurrentContext();
    CSyntaxDescHlpr vDescHlpr;

    if (GetCurrentDbgger()->IsDbgProcx64())
    {
        vDescHlpr.FormatDesc(FormatW(L"rax=0x%016llx ", context.cax), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"rbx=0x%016llx ", context.cbx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"rcx=0x%016llx ", context.ccx), COLOUR_HIGHT);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"rdx=0x%016llx ", context.cdx), COLOUR_HIGHT);
        vDescHlpr.FormatDesc(FormatW(L"rsi=0x%016llx ", context.csi), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"rdi=0x%016llx ", context.cdi), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"rip=0x%016llx ", context.cip), COLOUR_HIGHT);
        vDescHlpr.FormatDesc(FormatW(L"rsp=0x%016llx ", context.csp), COLOUR_HIGHT);
        vDescHlpr.FormatDesc(FormatW(L"rbp=0x%016llx ", context.cbp), COLOUR_HIGHT);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L" r8=0x%016llx ", context.r8), COLOUR_HIGHT);
        vDescHlpr.FormatDesc(FormatW(L" r9=0x%016llx ", context.r9), COLOUR_HIGHT);
        vDescHlpr.FormatDesc(FormatW(L"r10=0x%016llx ", context.r10), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"r11=0x%016llx ", context.r11), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"r12=0x%016llx ", context.r12), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"r13=0x%016llx ", context.r13), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"r14=0x%016llx ", context.r14), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"r15=0x%016llx ", context.r15), COLOUR_MSG);
        vDescHlpr.NextLine();
    }
    else
    {
        vDescHlpr.FormatDesc(FormatW(L"eax=0x%08x ", context.cax), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"ebx=0x%08x ", context.cbx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"ecx=0x%08x ", context.ccx), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"edx=0x%08x ", context.cdx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"esi=0x%08x ", context.csi), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"edi=0x%08x ", context.cdi), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"esp=0x%08x ", context.csp), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"ebp=0x%08x ", context.cbp), COLOUR_MSG);
    }
    vDescHlpr.FormatDesc(FormatW(L"cs=0x%04x  ", context.cs), COLOUR_MSG);
    vDescHlpr.FormatDesc(FormatW(L"ss=0x%04x  ", context.ss), COLOUR_MSG);
    vDescHlpr.FormatDesc(FormatW(L"ds=0x%04x  ", context.ds), COLOUR_MSG);
    vDescHlpr.FormatDesc(FormatW(L"es=0x%04x  ", context.es), COLOUR_MSG);
    vDescHlpr.FormatDesc(FormatW(L"fs=0x%04x  ", context.fs), COLOUR_MSG);
    vDescHlpr.FormatDesc(FormatW(L"gs=0x%04x  ", context.gs), COLOUR_MSG);
    vDescHlpr.NextLine();
    ustring wstrAddr = GetSymFromAddr(context.cip);
    vDescHlpr.FormatDesc(wstrAddr, COLOUR_PROC);
    return DbgCmdResult(em_dbgstat_succ, vDescHlpr.GetResult());
}

DbgCmdResult CProcDbgger::OnCmdScript(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    return res;
}