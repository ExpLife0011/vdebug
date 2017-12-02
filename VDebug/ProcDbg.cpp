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

HANDLE CProcDbgger::GetThreadHandle(DWORD dwThreadId)
{
    map<DWORD, DbgProcThreadInfo>::const_iterator it = m_vThreadMap.find(dwThreadId);
    if (m_vThreadMap.end() == it)
    {
        return NULL;
    }
    else
    {
        return it->second.m_hThread;
    }
}

TITAN_ENGINE_CONTEXT_t CProcDbgger::GetCurrentContext()
{
    DEBUG_EVENT *ptr = GetDebugProcData();

    TITAN_ENGINE_CONTEXT_t context = {0};
    map<DWORD, DbgProcThreadInfo>::const_iterator it = m_vThreadMap.find(ptr->dwThreadId);
    if (it == m_vThreadMap.end())
    {
        return context;
    }
    GetFullContextDataEx(it->second.m_hThread, &context);
    return context;
}

HANDLE CProcDbgger::GetCurrentThread()
{
    DEBUG_EVENT *ptr = GetDebugProcData();

    TITAN_ENGINE_CONTEXT_t context = {0};
    map<DWORD, DbgProcThreadInfo>::const_iterator it = m_vThreadMap.find(ptr->dwThreadId);
    if (it != m_vThreadMap.end())
    {
        return it->second.m_hThread;
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

    DbgProcThreadInfo tp;
    tp.m_dwThreadId = dwId;
    tp.m_hThread = pCreateProcessInfo->hThread;
    tp.m_dwLocalBase = (DWORD64)pCreateProcessInfo->lpThreadLocalBase;
    tp.m_dwStartAddr = (DWORD64)pCreateProcessInfo->lpStartAddress;
    tp.m_wstrName = L"主线程";

    (GetProcDbgger()->m_vThreadMap)[dwId] = tp;
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
    newThread.m_dwThreadNum = GetProcDbgger()->m_vThreadMap.size();
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)CreateThread->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)CreateThread->lpThreadLocalBase;
    newThread.m_hThread = CreateThread->hThread;

    GetProcDbgger()->m_vThreadMap[newThread.m_dwThreadId] = newThread;
}

void CProcDbgger::OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetProcDbgger()->m_vThreadMap.erase(dwId);
}

void CProcDbgger::OnSystemBreakpoint(void* ExceptionData)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    GetProcDbgger()->m_dwCurrentThreadId = dwId;
    if (GetProcDbgger()->m_vThreadMap.end() == GetProcDbgger()->m_vThreadMap.find(dwId))
    {
        return;
    }

    //脱离调试器
    if (GetProcDbgger()->m_bDetachDbgger)
    {
        return;
    }

    HANDLE hThread = GetProcDbgger()->m_vThreadMap[dwId].m_hThread;
    UINT cip = GetContextDataEx(hThread, UE_CIP);
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
    WCHAR wszStart[64] = {0};
    WCHAR wszEnd[64] = {0};
    _i64tow_s(loadInfo.m_ModuleInfo.m_dwBaseOfImage, wszStart, 64, 16);
    _i64tow_s(loadInfo.m_ModuleInfo.m_dwEndAddr, wszEnd, 16, 16);
    hlpr.FormatDesc(L"模块加载 ", COLOUR_MSG);
    hlpr.FormatDesc(FormatW(L"0x%08ls 0x%08ls  ", wszStart, wszEnd), COLOUR_MSG);
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
    }
    //展示指定线程
    else if (wstrCmd == L"ts")
    {
    }
    //展示模块信息
    else if (wstrCmd == L"lm")
    {
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
    else if (wstrCmd == L"gu")
    {
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
        OnCmdDd(wstrCmdParam, bShow, pParam);
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
            return DbgCmdResult(em_dbgstat_succ, L"地址已存在断点");
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
            return DbgCmdResult(em_dbgstat_succ, L"");
        }
    }
    return DbgCmdResult(em_dbgstat_faild, L"bp命令执行失败");
}

DbgCmdResult CProcDbgger::OnCmdBl(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CSyntaxDescHlpr hlpr;
    if (m_vBreakPoint.empty())
    {
        hlpr.FormatDesc(L"尚未设置任何断点", COLOUR_MSG);
        GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
        return DbgCmdResult(em_dbgstat_succ);
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
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    return DbgCmdResult(em_dbgstat_succ);
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
    CSyntaxDescHlpr hlpr;
    if (wstr == L"*")
    {
        ClearBreakPoint(-1);
        hlpr.FormatDesc(L"已清空所有断点", COLOUR_MSG);
        GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
        return DbgCmdResult(em_dbgstat_succ);
    }

    if (!IsNumber(wstr))
    {
        return DbgCmdResult(em_dbgstat_syntaxerr);
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(wstr, dwSerial);
    ClearBreakPoint((DWORD)dwSerial);
    hlpr.FormatDesc(FormatW(L"已清除%02x号断点", dwSerial), COLOUR_MSG);
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    return DbgCmdResult(em_dbgstat_succ);
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

bool CProcDbgger::DisassWithSize(DWORD64 dwAddr, DWORD64 dwSize) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    CSyntaxDescHlpr hlpr;
    ustring wstr = GetSymFromAddr(dwAddr);
    wstr += L":";
    hlpr.NextLine();
    hlpr.FormatDesc(wstr, COLOUR_PROC);
    hlpr.NextLine();
    if (Disasm.DisasmWithSize(dwAddr, (DWORD)dwSize, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
            hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
            hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 8);
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
        return true;
    }
    return false;
}

bool CProcDbgger::DisassWithAddr(DWORD64 dwStartAddr, DWORD64 dwEndAddr) const
{
    if (dwEndAddr <= dwStartAddr)
    {
        return false;
    }

    DWORD64 dwSize = (dwEndAddr - dwStartAddr + 16);
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    CSyntaxDescHlpr hlpr;
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
            hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
            hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
            hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 8);
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
        return true;
    }
    return false;
}

bool CProcDbgger::DisassUntilRet(DWORD64 dwStartAddr) const
{
    CDisasmParser Disasm(GetDbgProc());
    vector<DisasmInfo> vDisasmSet;
    CSyntaxDescHlpr hlpr;
    ustring wstr = GetSymFromAddr(dwStartAddr);
    wstr += L":";
    hlpr.NextLine();
    hlpr.FormatDesc(wstr, COLOUR_PROC);
    hlpr.NextLine();
    if (Disasm.DisasmUntilReturn(dwStartAddr, vDisasmSet))
    {
        for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
        {
            hlpr.FormatDesc(it->m_wstrAddr, COLOUR_ADDR, 10);
            hlpr.FormatDesc(it->m_wstrByteCode, COLOUR_BYTE, 18);
            hlpr.FormatDesc(it->m_wstrOpt, COLOUR_INST, 8);
            GetDisassContentDesc(it->m_wstrContent, hlpr);
            hlpr.NextLine();
        }
        GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
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
        return DbgCmdResult(em_dbgstat_faild, L"");
    }
    DisassWithSize(dwAddr, dwDisasmSize);
    return DbgCmdResult(em_dbgstat_succ, L"");
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

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;
    DisassWithAddr(dwStartAddr, dwEndAddr);
    return DbgCmdResult(em_dbgstat_succ);
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
    DisassUntilRet(dwAddr);
    return DbgCmdResult(em_dbgstat_succ);
}

DbgCmdResult CProcDbgger::OnCmdGo(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    Run();
    return DbgCmdResult(em_dbgstat_succ, L"");
}

DbgCmdResult CProcDbgger::OnCmdGu(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    Run();
    return DbgCmdResult(em_dbgstat_succ, L"");
}

list<STACKFRAME64> CProcDbgger::GetStackFrame(const ustring &wstrParam)
{
    const int iMaxWalks = 1024;
    TITAN_ENGINE_CONTEXT_t context = GetProcDbgger()->GetCurrentContext();
    STACKFRAME64 frame = {0};

    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context.cip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.cbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.csp;
    frame.AddrStack.Mode = AddrModeFlat;

    CTaskStackWalkInfo info;
    info.m_context = frame;
    info.m_dwMachineType = IMAGE_FILE_MACHINE_I386;
    info.m_pfnGetModuleBaseProc = GetModuelBaseFromAddr;
    info.m_pfnReadMemoryProc = NULL;
    info.m_pfnStackTranslateProc = StackTranslateAddressProc64;
    info.m_hDstProcess = GetProcDbgger()->GetDbgProc();
    info.m_hDstThread = GetProcDbgger()->GetCurrentThread();

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
    hlpr.FormatDesc(L"返回地址 ", COLOUR_MSG, 8);
    hlpr.FormatDesc(L"参数列表 ", COLOUR_MSG, 8);
    hlpr.NextLine();
    for (list<STACKFRAME64>::const_iterator it = vStack.begin() ; it != vStack.end() ; it++)
    {
        hlpr.FormatDesc(FormatW(L"%08x ", (DWORD)it->AddrReturn.Offset), COLOUR_ADDR);
        for (int j = 0 ; j < 4 ; j++)
        {
            hlpr.FormatDesc(FormatW(L"%08x ", (DWORD)it->Params[j]), COLOUR_PARAM);
        }

        hlpr.FormatDesc(FormatW(L"%ls", GetProcDbgger()->GetSymFromAddr(it->AddrPC.Offset).c_str()), COLOUR_PROC);
        hlpr.NextLine();
    }
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    return DbgCmdResult(em_dbgstat_succ, L"");
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
    GetSyntaxView()->AppendSyntaxDesc(desc.GetResult());
    return DbgCmdResult();
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
    GetSyntaxView()->AppendSyntaxDesc(desc.GetResult());
    return DbgCmdResult(em_dbgstat_succ, L"");
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
    GetSyntaxView()->AppendSyntaxDesc(desc.GetResult());
    return DbgCmdResult(em_dbgstat_succ, L"");
}

DbgCmdResult CProcDbgger::OnCmdReg(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    TITAN_ENGINE_CONTEXT_t context = GetCurrentDbgger()->GetCurrentContext();
    if (bShow)
    {
        CSyntaxDescHlpr vDescHlpr;
        vDescHlpr.FormatDesc(FormatW(L"CAX=0x%08x ", context.cax), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CBX=0x%08x ", context.cbx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CCX=0x%08x ", context.ccx), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"CDX=0x%08x ", context.cdx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CSI=0x%08x ", context.csi), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CDI=0x%08x ", context.cdi), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"CSP=0x%08x ", context.csp), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CBP=0x%08x ", context.cbp), COLOUR_MSG);
        GetSyntaxView()->AppendSyntaxDesc(vDescHlpr.GetResult());
    }
    return DbgCmdResult(em_dbgstat_succ, L"");
}

DbgCmdResult CProcDbgger::OnCmdScript(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    return res;
}