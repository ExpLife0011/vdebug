#include "Command.h"
#include "SyntaxDescHlpr.h"
#include "Debugger.h"
#include "common.h"
#include "MainView.h"
#include "Index.h"
#include "BreakPoint.h"
#include "TitanEngine/TitanEngine.h"
#include "memory.h"
#include "Script.h"
#include "Disasm.h"
#include "ProcDbg.h"

#if WIN64 || _WIN64
#pragma comment(lib, "capstone_x64.lib")
#else
#pragma comment(lib, "capstone_x86.lib")
#endif

map<ustring, DWORD64> CCmdEngine::ms_vProcMap;

BOOL CCmdEngine::CmdRegister(const ustring &wstrCmd, pfnCmdCallbackProc pfnCmdProc)
{
    CScopedLocker lock(this);
    ustring wstr(wstrCmd);
    m_vCmdCallback[wstr.makelower()] = pfnCmdProc;
    //GetIndexEngine()->RegisterIndex(em_index_command, wstrCmd, NULL);
    return TRUE;
}

BOOL CCmdEngine::CmdIsValid(const ustring &wstrCmd)
{
    CScopedLocker lock(this);
    return (m_vCmdCallback.end() != m_vCmdCallback.find(ustring(wstrCmd).makelower()));
}

DWORD64 CCmdEngine::GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr)
{
    return GetCurrentDbgger()->GetModuelBaseFromAddr();
}

//0x43fdad12
//0n12232433
//5454546455
BOOL CCmdEngine::GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwResult)
{
    return StrToInt64ExW(wstrNumber.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
}

DWORD64 CCmdEngine::GetAddrFormStr(const ustring &wstrStr)
{
    DWORD64 dwAddr = 0;
    if (!GetNumFromStr(wstrStr, dwAddr))
    {
        ustring wstr = wstrStr;
        wstr.makelower();
        wstr.trim();

        if (wstr == L"cax")
        {
        }
        else if (wstr == L"cbx")
        {
        }
    }

    return dwAddr;
}

BOOL CCmdEngine::AddProcMsg(const ustring &wstrIdex, DWORD64 dwAddr)
{
    ustring wstr = wstrIdex;
    wstr.makelower();
    ms_vProcMap[wstr] = dwAddr;
    return TRUE;
}

DWORD64 CCmdEngine::GetAddrFromProcStr(const ustring &wstrProc) const
{
    ustring wstr(wstrProc);
    wstr.makelower();

    map<ustring, DWORD64>::const_iterator it = ms_vProcMap.find(wstr);
    if (it == ms_vProcMap.end())
    {
        return 0;
    }
    return it->second;
}

CommandResult CCmdEngine::RunCmd(const ustring &wstrCmd, LPVOID pUserParam, BOOL bShow, pfnCmdNotifyCallback pfn)
{
    ustring wstr(wstrCmd);
    wstr.makelower();
    wstr.trim();
    if (wstr.empty())
    {
        return CommandResult();
    }

    ustring wstrStart;
    ustring wstrParam;
    size_t i = wstr.find(L" ");
    if (ustring::npos == i)
    {
        wstrStart = wstr;
    }
    else
    {
        wstrStart = wstr.substr(0, i);
        wstrParam = wstr.c_str() + i;
        wstrParam.trim();
    }

    CScopedLocker lock(this);
    map<ustring, pfnCmdCallbackProc>::const_iterator it = m_vCmdCallback.find(ustring(wstrStart).makelower());
    if (it == m_vCmdCallback.end())
    {
        return CommandResult(em_status_notfound, ustring().format(L"%ls 非内置命令", wstrStart.c_str()));
    }
    CmdUserContext context;
    context.m_pfn = pfn;
    context.m_pUserParam = pUserParam;
    return it->second(wstrParam, bShow, &context);
}

CommandResult CCmdEngine::CmdDu(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    //DWORD64 dwAddr = GetAddrFormStr(pParam);
    CScriptEngine script;
    script.SetContext(GetCurrentDbgger()->GetCurrentContext());

    DWORD64 dwAddr = script.Compile(pParam);
    if (!dwAddr)
    {
        return CommandResult();
    }

    //CMemoryOperator mhlpr(GetDebugger()->GetDebuggerInfo().m_hProcess);
    //ustring wstrData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

    //CSyntaxDescHlpr desc;
    //desc.FormatDesc(wstrData.c_str(), COLOUR_MSG);
    //GetSyntaxView()->AppendSyntaxDesc(desc.GetResult());
    return CommandResult(em_status_succ, L"");
}

CommandResult CCmdEngine::CmdScript(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    GetScriptEngine()->RunScript(pParam);
    return CommandResult(em_status_succ, L"");
}

VOID CCmdEngine::CmdInit()
{
    CmdRegister(L"r", CmdRegProc);
    CmdRegister(L"u", CmdDisassProc);
    CmdRegister(L"g", CmdGo);
    CmdRegister(L"bp", CmdBp);
    CmdRegister(L"kv", CmdKv);
    CmdRegister(L"du", CmdDu);
    CmdRegister(L"sc", CmdScript);
}

CommandResult CCmdEngine::CmdRegProc(const ustring &pParam, BOOL bShow, CmdUserContext *pUserContext)
{
    TITAN_ENGINE_CONTEXT_t context = GetCurrentDbgger()->GetCurrentContext();
    if (bShow)
    {
        CSyntaxDescHlpr vDescHlpr;
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"CAX=0x%016x ", context.cax), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CBX=0x%016x ", context.cbx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CCX=0x%016x ", context.ccx), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"CDX=0x%016x ", context.cdx), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CSI=0x%016x ", context.csi), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CDI=0x%016x ", context.cdi), COLOUR_MSG);
        vDescHlpr.NextLine();

        vDescHlpr.FormatDesc(FormatW(L"CSP=0x%016x ", context.csp), COLOUR_MSG);
        vDescHlpr.FormatDesc(FormatW(L"CBP=0x%016x ", context.cbp), COLOUR_MSG);
        vDescHlpr.AddEmptyLine();

        GetSyntaxView()->AppendSyntaxDesc(vDescHlpr.GetResult());
    }
    return CommandResult(em_status_succ, L"");
}

CommandResult CCmdEngine::CmdDisassProc(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    //ustring wstr(pParam);
    //wstr.makelower();

    //map<ustring, DWORD64>::const_iterator it = ms_vProcMap.find(wstr);
    //if (ms_vProcMap.end() == (it = ms_vProcMap.find(wstr)))
    //{
    //    return CommandResult();
    //}

    //CDisasmParser Disasm(GetDebugger()->GetDebugProc());
    //vector<DisasmInfo> vDisasmSet;
    //if (Disasm.Disasm(it->second, 1024, vDisasmSet))
    //{
    //    CSyntaxDescHlpr hlpr;
    //    for (vector<DisasmInfo>::const_iterator it = vDisasmSet.begin() ; it != vDisasmSet.end() ; it++)
    //    {
    //        hlpr.FormatDesc(it->m_wstrAddr, COLOUR_MSG, 17);
    //        hlpr.FormatDesc(it->m_wstrOpt, COLOUR_MSG, 8);
    //        hlpr.FormatDesc(it->m_wstrContent, COLOUR_MSG);
    //        hlpr.NextLine();
    //    }
    //    GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    //}
    return CommandResult();
}

CommandResult CCmdEngine::CmdGo(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    ((CProcDbgger *)GetCurrentDbgger())->Run();
    return CommandResult(em_status_succ, L"");
}

/*
{
    "addr" = "12345678"
}
*/
CommandResult CCmdEngine::CmdBp(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    ustring wstr(pParam);
    wstr.trim();

    if (wstr.empty())
    {
        return CommandResult();
    }

    DWORD64 dwProcAddr = 0;
    if (!GetNumFromStr(wstr, dwProcAddr))
    {
        map<ustring, DWORD64>::const_iterator it = ms_vProcMap.find(wstr);
        if (it != ms_vProcMap.end())
        {
            dwProcAddr = it->second;
        }
    }

    //if (dwProcAddr)
    //{
    //    if (GetBreakPointMgr()->SetBreakPoint(dwProcAddr, pUserCountext))
    //    {
    //        return CommandResult(em_status_succ, L"");
    //    }
    //}
    return CommandResult(em_status_faild, L"bp命令执行失败");
}

BOOL CALLBACK ReadProcessMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    return TRUE;
}

static DWORD64 CALLBACK StackTranslateAddressProc64(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr)
{
    return 0;
}

CommandResult CCmdEngine::CmdKv(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext)
{
    const int iMaxWalks = 1024;

    CSyntaxDescHlpr hlpr;
    TITAN_ENGINE_CONTEXT_t context = GetDebugger()->GetCurrentContext();
    STACKFRAME64 frame = {0};

    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = context.cip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.cbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.csp;
    frame.AddrStack.Mode = AddrModeFlat;

    //HANDLE hProcess = GetDebugger()->GetDebuggerInfo().m_hProcess;
    //DEBUG_EVENT *pDebugEvent = GetDebugger()->GetDebugProcData();
    //hlpr.FormatDesc(L"函数返回地址 ", COLOUR_MSG, 17);
    //hlpr.FormatDesc(L"参数列表", COLOUR_MSG, 17 * 4);
    //hlpr.FormatDesc(L"函数位置", COLOUR_MSG);

    //for (int i = 0 ; i < iMaxWalks ; i++)
    //{
    //    if (!StackWalk64(
    //        machineType,
    //        hProcess,
    //        GetDebugger()->GetThreadFromId(pDebugEvent->dwThreadId),
    //        &frame,
    //        NULL,
    //        NULL,
    //        SymFunctionTableAccess64,
    //        GetModuelBaseFromAddr,
    //        StackTranslateAddressProc64
    //        ))
    //    {
    //        break;
    //    }

    //    hlpr.NextLine();
    //    hlpr.FormatDesc(FormatW(L"%016x ", frame.AddrReturn), COLOUR_MSG);
    //    for (int j = 0 ; j < 4 ; j++)
    //    {
    //        hlpr.FormatDesc(FormatW(L"%016x ", frame.Params[j]), COLOUR_MSG);
    //    }

    //    hlpr.FormatDesc(FormatW(L"%ls", GetDebugger()->GetSymFromAddr(frame.AddrPC.Offset).c_str()), COLOUR_MSG);
    //}
    //hlpr.AddEmptyLine();
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    list<STACKFRAME64> vStack = GetCurrentDbgger()->GetStackFrame(frame);
    return CommandResult();
}

CCmdEngine *GetCmdEngine()
{
    static CCmdEngine *s_pInstance = NULL;
    if (!s_pInstance)
    {
        s_pInstance = new CCmdEngine();
        s_pInstance->CmdInit();
    }
    return s_pInstance;
}