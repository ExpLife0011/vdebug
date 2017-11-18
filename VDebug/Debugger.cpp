#include <Windows.h>
//#include <Dbghelp.h>
#include <map>
#include "mstring.h"
#include "TitanEngine/TitanEngine.h"
#include "SyntaxDescHlpr.h"
#include "common.h"
#include "MainView.h"
#include "Command.h"
#include "Debugger.h"
#include "SyntaxDescHlpr.h"
#include "Index.h"
#include "BreakPoint.h"
#include "symbol.h"
#include "ProcDbg.h"

using namespace std;

#if WIN64 || _WIN64
#pragma comment(lib, "TitanEngine/TitanEngine_x64.lib")
#else
#pragma comment(lib, "TitanEngine/TitanEngine_x86.lib")
#endif

#pragma comment(lib, "Dbghelp.lib")

DebuggerInfo CDebuggerEngine::ms_DebugInfo;
DWORD CDebuggerEngine::ms_dwCurDebugProc = 0;
map<DWORD, ThreadInfo> *CDebuggerEngine::ms_pThreadMap = new map<DWORD, ThreadInfo>();;
DWORD CDebuggerEngine::ms_dwCurrentThreadId = 0;
map<DWORD64, ModuleInfo> *CDebuggerEngine::ms_pModuleMap = new map<DWORD64, ModuleInfo>();
HANDLE CDebuggerEngine::ms_hRunNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
DebuggerStatus CDebuggerEngine::ms_eDebuggerStatus = em_dbg_status_init;

void CDebuggerEngine::OnCreateProcess(CREATE_PROCESS_DEBUG_INFO* pCreateProcessInfo)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"进程路径", COLOUR_MSG, 16);
    hlpr.FormatDesc(ms_DebugInfo.m_wstrPePath.c_str(), COLOUR_MSG);
    hlpr.NextLine();
    hlpr.FormatDesc(L"进程基地址", COLOUR_MSG, 16);
    hlpr.FormatDesc(FormatW(L"0x%08x", pCreateProcessInfo->lpBaseOfImage), COLOUR_MSG);
    hlpr.NextLine();
    hlpr.FormatDesc(L"进程入口地址", COLOUR_MSG, 16);
    hlpr.FormatDesc(FormatW(L"0x%08x", pCreateProcessInfo->lpStartAddress), COLOUR_MSG);
    hlpr.AddEmptyLine();
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());

    ThreadInfo tp;
    tp.m_dwThreadId = dwId;
    tp.m_hThread = pCreateProcessInfo->hThread;
    tp.m_dwLocalBase = (DWORD64)pCreateProcessInfo->lpThreadLocalBase;
    tp.m_dwStartAddr = (DWORD64)pCreateProcessInfo->lpStartAddress;
    tp.m_wstrName = L"主线程";

    (*ms_pThreadMap)[dwId] = tp;
    ms_dwCurDebugProc = GetProcessId(pCreateProcessInfo->hProcess);

    ms_DebugInfo.m_hProcess = pCreateProcessInfo->hProcess;
    LoadModuleInfo(pCreateProcessInfo->hFile, (DWORD64)pCreateProcessInfo->lpBaseOfImage);
}

void CDebuggerEngine::OnExitProcess(EXIT_PROCESS_DEBUG_INFO* ExitProcess)
{}

void CDebuggerEngine::OnCreateThread(CREATE_THREAD_DEBUG_INFO* CreateThread)
{
    ThreadInfo newThread;
    newThread.m_dwThreadNum = ms_pThreadMap->size();
    newThread.m_dwThreadId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    newThread.m_dwStartAddr = (DWORD64)CreateThread->lpStartAddress;
    newThread.m_dwLocalBase = (DWORD64)CreateThread->lpThreadLocalBase;
    newThread.m_hThread = CreateThread->hThread;

    (*ms_pThreadMap)[newThread.m_dwThreadId] = newThread;
}

void CDebuggerEngine::OnExitThread(EXIT_THREAD_DEBUG_INFO* ExitThread)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    ms_pThreadMap->erase(dwId);
}

void CDebuggerEngine::OnSystemBreakpoint(void* ExceptionData)
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    ms_dwCurrentThreadId = dwId;
    if (ms_pThreadMap->end() == ms_pThreadMap->find(dwId))
    {
        return;
    }

    HANDLE hThread = (*ms_pThreadMap)[dwId].m_hThread;
    UINT cip = GetContextDataEx(hThread, UE_CIP);
    GetCurrentDbgger()->RunCommand(L"r");
    ((CProcDbgger *)GetCurrentDbgger())->Wait();
}

BOOL CALLBACK CDebuggerEngine::EnumSymbolProc(PSYMBOL_INFOW pSymInfo, ULONG uSymbolSize, PVOID pUserContext)
{
    ModuleInfo *ptr = (ModuleInfo *)pUserContext;
    ustring wstr = ptr->m_wstrDllName;
    size_t pos = wstr.rfind(L'.');
    if (pos != wstring::npos)
    {
        wstr.erase(pos, wstr.size() - pos);
    }
    wstr += L"!";
    wstr += pSymInfo->Name;
    GetCmdEngine()->AddProcMsg(wstr, pSymInfo->Address);
    return TRUE;
}

void CDebuggerEngine::AddProcIndex(const ustring &wstrModuleName, const ustring &wstrProcName, LPVOID ptr)
{
    ustring wstr = FormatW(L"%ls!%ls", wstrModuleName.c_str(), wstrProcName.c_str());
    Value vContent;
    vContent["addr"] = WtoU(FormatW(L"%I64u", (DWORD64)ptr));
    GetIndexEngine()->RegisterIndex(em_index_symbol, wstrProcName, wstr, vContent);
    GetIndexEngine()->RegisterIndex(em_index_symbol, wstr, wstr, vContent);
}

void CDebuggerEngine::DeleteIndex(LPVOID ptr)
{
    map<DWORD64, ModuleInfo>::const_iterator it = ms_pModuleMap->find((DWORD64)ptr);
    if (ms_pModuleMap->end() != it)
    {
        map<DWORD64, ModuleProcInfo>::const_iterator itProc;
        for (itProc = it->second.m_vProcInfo.begin() ; itProc != it->second.m_vProcInfo.end() ; itProc++)
        {
            //GetIndexEngine()->Erase(em_index_symbol, itProc->second.m_wstrName);
            //GetIndexEngine()->Erase(em_index_symbol, FormatW(L"%ls!%ls", it->second.m_wstrDllName.c_str(), itProc->second.m_wstrName));
        }
        ms_pModuleMap->erase(it);
    }
}

bool CDebuggerEngine::LoadModuleInfo(HANDLE hFile, DWORD64 dwBaseOfModule)
{
    CSymbolTaskHeader task;
    CTaskLoadSymbol loadInfo;
    loadInfo.m_dwBaseOfModule = dwBaseOfModule;
    loadInfo.m_hImgaeFile = hFile;
    task.m_dwSize = sizeof(CSymbolTaskHeader) + sizeof(CTaskLoadSymbol);
    task.m_pParam = &loadInfo;
    task.m_eTaskType = em_task_loadsym;

    GetSymbolHlpr()->SendTask(&task);

    CSyntaxDescHlpr hlpr;
    WCHAR wszStart[64] = {0};
    WCHAR wszEnd[64] = {0};
    _i64tow_s(loadInfo.m_ModuleInfo.m_dwBaseOfImage, wszStart, 64, 16);
    _i64tow_s(loadInfo.m_ModuleInfo.m_dwEndAddr, wszEnd, 16, 16);
    hlpr.FormatDesc(
        FormatW(L"模块加载 0x%016ls 0x%016ls  %ls", wszStart, wszEnd, loadInfo.m_ModuleInfo.m_wstrDllName.c_str()).c_str(),
        COLOUR_MSG
        );
    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    return true;
}

void CDebuggerEngine::OnLoadDll(LOAD_DLL_DEBUG_INFO* LoadDll)
{
    LoadModuleInfo(LoadDll->hFile, (DWORD64)LoadDll->lpBaseOfDll);
}

void CDebuggerEngine::OnUnloadDll(UNLOAD_DLL_DEBUG_INFO* UnloadDll)
{
    //CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(L"模块卸载", COLOUR_MSG, 10);

    //DeleteIndex(UnloadDll->lpBaseOfDll);
}

void CDebuggerEngine::OnOutputDebugString(OUTPUT_DEBUG_STRING_INFO* DebugString)
{}

void CDebuggerEngine::OnException(EXCEPTION_DEBUG_INFO* ExceptionData)
{
    if (EXCEPTION_BREAKPOINT == ExceptionData->ExceptionRecord.ExceptionCode)
    {
        int d = 0;
    }
}

void CDebuggerEngine::OnDebugEvent(DEBUG_EVENT* DebugEvent)
{}

void CDebuggerEngine::CustomBreakPoint()
{
    return;
}

DWORD CDebuggerEngine::DebugThread(LPVOID pParam)
{
    if (em_debugger_attach == ms_DebugInfo.m_eType)
    {
        AttachDebugger((DWORD)ms_DebugInfo.m_dwPid, true, NULL, NULL);
    }
    else if (em_debugger_open == ms_DebugInfo.m_eType)
    {
        LPCWSTR wszDir = NULL;
        if (!ms_DebugInfo.m_wstrCurrentDir.empty())
        {
            wszDir = ms_DebugInfo.m_wstrCurrentDir.c_str();
        }

        PROCESS_INFORMATION *process = (PROCESS_INFORMATION *)InitDebugW(
            ms_DebugInfo.m_wstrPePath.c_str(),
            ms_DebugInfo.m_wstrCmd.c_str(),
            NULL
            );
        DebugLoop();
    }
    return 0;
}

VOID CDebuggerEngine::Attach(DWORD dwPid)
{
    ms_DebugInfo.m_eType = em_debugger_attach;
    ms_DebugInfo.m_dwPid = dwPid;
    CloseHandle(CreateThread(NULL, 0, DebugThread, (LPVOID)&ms_DebugInfo, 0, NULL));
}

VOID CDebuggerEngine::Detach()
{
    DetachDebugger(ms_dwCurDebugProc);
    ms_dwCurDebugProc = 0;
    ms_pThreadMap->clear();
}

VOID CDebuggerEngine::Wait()
{
    DWORD dwId = ((DEBUG_EVENT*)GetDebugData())->dwThreadId;
    ms_eDebuggerStatus = em_dbg_status_free;
    SetCmdNotify(em_dbg_status_free, ustring().format(L"线程 %04x >>", dwId).c_str());
    WaitForSingleObject(ms_hRunNotify, INFINITE);
    SetCmdNotify(em_dbg_status_busy, ustring().format(L"线程 %04x >>", dwId).c_str());
}

VOID CDebuggerEngine::Run()
{
    SetEvent(ms_hRunNotify);
}

DebuggerStatus CDebuggerEngine::Status()
{
    return ms_eDebuggerStatus;
}

bool CDebuggerEngine::OpenExe(LPCWSTR wszPeName, LPCWSTR wszCommand, LPCWSTR wszCurrentDir)
{
    ms_DebugInfo.m_eType = em_debugger_open;
    ms_DebugInfo.m_dwPid = 0;
    ms_DebugInfo.m_wstrPePath = wszPeName;
    ms_DebugInfo.m_wstrCmd = wszCommand;
    ms_DebugInfo.m_wstrCurrentDir = wszCurrentDir;
    ms_DebugInfo.m_pfn = CustomBreakPoint;
    CloseHandle(CreateThread(NULL, 0, DebugThread, (LPVOID)&ms_DebugInfo, 0, NULL));
    return true;
}

TITAN_ENGINE_CONTEXT_t CDebuggerEngine::GetCurrentContext()
{
    DEBUG_EVENT *ptr = GetDebugger()->GetDebugProcData();

    TITAN_ENGINE_CONTEXT_t context = {0};
    map<DWORD, ThreadInfo>::const_iterator it = ms_pThreadMap->find(ptr->dwThreadId);
    if (it == ms_pThreadMap->end())
    {
        return context;
    }
    GetFullContextDataEx(it->second.m_hThread, &context);
    return context;
}

DEBUG_EVENT *CDebuggerEngine::GetDebugProcData()
{
    return (DEBUG_EVENT *)GetDebugData();
}

ModuleInfo CDebuggerEngine::GetModuleFromAddr(DWORD64 dwAddr)
{
    for (map<DWORD64, ModuleInfo>::const_iterator it = ms_pModuleMap->begin() ; it != ms_pModuleMap->end() ; it++)
    {
        if (dwAddr >= it->second.m_dwBaseOfImage && dwAddr <= it->second.m_dwEndAddr)
        {
            return it->second;
        }
    }

    return ModuleInfo();
}

DebuggerInfo CDebuggerEngine::GetDebuggerInfo()
{
    return ms_DebugInfo;
}

HANDLE CDebuggerEngine::GetDebugProc()
{
    return ms_DebugInfo.m_hProcess;
}

HANDLE CDebuggerEngine::GetThreadFromId(DWORD dwThreadId)
{
    map<DWORD, ThreadInfo>::const_iterator it = ms_pThreadMap->find(dwThreadId);
    if (it != ms_pThreadMap->end())
    {
        return it->second.m_hThread;
    }

    return NULL;
}

ustring CDebuggerEngine::GetSymFromAddr(DWORD64 dwAddr)
{
    CTaskSymbolFromAddr task;
    task.m_dwAddr = dwAddr;
    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskSymbolFromAddr) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_symaddr;
    header.m_pParam = &task;
    GetSymbolHlpr()->SendTask(&header);

    ModuleInfo module = GetModuleFromAddr(dwAddr);
    if (module.m_wstrDllName.empty())
    {
        return L"";
    }

    ustring wstr = module.m_wstrDllName;
    size_t pos = wstr.rfind(L'.');
    if (ustring::npos != pos)
    {
        wstr.erase(pos, wstr.size() - pos);
    }
    return FormatW(L"%ls!%ls", wstr.c_str(), task.m_wstrSymbol.c_str());
}

static CommandResult WINAPI _CmdDisassProc(LPVOID pParam, BOOL bShow)
{
    return CommandResult();
}

VOID CDebuggerEngine::InitEngine()
{
    //此处的回调调用约定不是固定的，貌似是跟IDE默认调用约定保持一致
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

CDebuggerEngine *GetDebugger()
{
    static CDebuggerEngine *gs_pDebugger = NULL;
    if (!gs_pDebugger)
    {
        gs_pDebugger = new CDebuggerEngine();
        gs_pDebugger->InitEngine();
    }

    return gs_pDebugger;
}