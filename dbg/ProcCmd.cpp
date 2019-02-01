#include <DbgCtrl/DbgCtrlCom.h>
#include "ProcCmd.h"
#include "Script.h"
#include "memory.h"

CProcCmd *CProcCmd::GetInst() {
    static CProcCmd *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new CProcCmd();
    }

    return s_ptr;
}

CProcCmd::CProcCmd() : mProcDbgger(NULL) {
}

CProcCmd::~CProcCmd() {
}

void CProcCmd::InitProcCmd(CProcDbgger *pDbgger) {
    mProcDbgger = pDbgger;
}

CmdReplyResult CProcCmd::OnCommand(const mstring &cmd, const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
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
        DebugBreakProcess(mProcDbgger->GetDbgProc());
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

CmdReplyResult CProcCmd::OnCmdBp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
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

CmdReplyResult CProcCmd::OnCmdBl(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    /*
    CSyntaxDescHlpr hlpr;
    if (m_vBreakPoint.empty())
    {
        return mstring(em_dbgstat_succ, "尚未设置任何断点");
    }

    BOOL bx64 = GetCurrentDbgger()->IsDbgProcx64();

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

CmdReplyResult CProcCmd::OnCmdBc(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CmdReplyResult tmp;
    mstring str(cmdParam);
    str.makelower();
    if (str == "*")
    {
        mProcDbgger->ClearBreakPoint(-1);
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
    mProcDbgger->ClearBreakPoint((DWORD)dwSerial);
    tmp.mCmdShow = FormatA("已清除%02x号断点", dwSerial);
    return tmp;
}

CmdReplyResult CProcCmd::OnCmdDisass(const mstring &wstrCmdParam, DWORD mode, const CmdUserParam *pParam)
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
        dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
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

CmdReplyResult CProcCmd::OnCmdClear(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    //GetSyntaxView()->ClearView();
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdUb(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
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
        dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
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

CmdReplyResult CProcCmd::OnCmdUf(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
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
        dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
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

CmdReplyResult CProcCmd::OnCmdGo(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mProcDbgger->Run();
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdGu(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mProcDbgger->RunExitProc();

    CmdReplyResult reply;
    reply.mCmdShow = "执行gu成功";
    return reply;
}

CmdReplyResult CProcCmd::OnCmdReg(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    RegisterContent ctx;
    ctx.mContext = mProcDbgger->GetCurrentContext();
    ctx.mCipStr = mProcDbgger->GetSymFromAddr(ctx.mContext.cip).c_str();

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

CmdReplyResult CProcCmd::OnCmdScript(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdHelp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring strParam(param);
    strParam.makelower();
    strParam.trim();

    /*
    SyntaxDesc desc;
    if (wstrParam.empty())
    {
        desc += L"VDebug支持的命令功能概要";
        desc += L"bp  在指定的内存地址或者函数设置断点";
        desc += L"bl  打印已存在的断点信息";
        desc += L"bc  清空指定的断点";
        desc += L"tc  切换到指定的线程";
        desc += L"ts  打印所有的线程信息";
        desc += L"lm  打印所有的模块信息";
        desc += L"cls 清空页面信息";
        desc += L"u   反汇编指定的地址或者api";
        desc += L"ub  向上反汇编指定的地址或者api";
        desc += L"uf  反汇编指定的函数";
        desc += L"g   继续运行调试进程";
        desc += L"gu  运行到调用返回";
        desc += L"kv  打印调用栈和参数信息";
        desc += L"db  按字节打印指定内存地址的数据";
        desc += L"dd  按32整形打印指定内存地址的数据";
        desc += L"du  按宽字符串打印指定内存地址的数据";
        desc += L"r   打印或者修改当前线程的寄存器值";
        desc += L"sc  运行指定的脚本";
    }
    else if(wstrParam == L"bp")
    {
        desc += L"在指定的内存地址或者函数设置断点";
        desc += L"eg:";
        desc += L"bp 0x1122aabb             在内存0x1122aabb位置下断点";
        desc += L"bp kernelbase!createfilew 在kernelbase模块导出的createfilew函数上下断点";
    }
    else if (wstrParam == L"bl")
    {
        desc += L"打印已存在的断点信息";
    }
    else if (wstrParam == L"bc")
    {
        desc += L"清除指定的断点";
        desc += L"eg:";
        desc += L"bc *  清除当前所有的断点";
        desc += L"bc 1  清除编号为1的断点";
    }
    else if (wstrParam == L"tc")
    {
        desc += L"切换到指定线程";
        desc += L"eg:";
        desc += L"tc 1028  切换到线程tid为1028的线程";
        desc += L"tc 1     切换到序号为1的线程";
    }
    else if (wstrParam == L"ts")
    {
        desc += L"打印当前所有的线程信息";
    }
    else if (wstrParam == L"lm")
    {
        desc += L"打印当前加载的所有的模块信息";
    }
    else if (wstrParam == L"cls")
    {
        desc += L"清楚当前屏幕上的信息";
    }
    else if (wstrParam == L"u")
    {
        desc += L"反汇编指定的地址或者api";
        desc += L"eg:";
        desc += L"u 0x1028ffee              从0x1028ffee位置反汇编";
        desc += L"u kernelbase!createfilew  从kernelbase!createfilew起始的位置反汇编";
    }
    else if (wstrParam == L"ub")
    {
        desc += L"向上反汇编指定的地址或者api";
        desc += L"eg:";
        desc += L"u 0x1028ffee              从0x1028ffee位置向上反汇编";
        desc += L"u kernelbase!createfilew  从kernelbase!createfilew起始的位置向上反汇编";
    }
    else if (wstrParam == L"uf")
    {
        desc += L"反汇编指定的函数";
        desc += L"eg:";
        desc += L"uf kernel32!createfilew  反汇编函数kernel32!createfilew";
        desc += L"uf 0x1234abcd            反汇编位于0x1234abcd的函数调用";
    }
    else if (wstrParam == L"g")
    {
        desc += L"继续运行调试进程";
    }
    else if (wstrParam == L"gu")
    {
        desc += L"运行到调用返回";
    }
    else if (wstrParam == L"kv")
    {
        desc += L"打印调用栈和参数信息";
    }
    else if (wstrParam == L"db")
    {
        desc += L"按字节打印指定内存地址的数据";
        desc += L"eg:";
        desc += L"db 0x1234abcd 从地址0x1234abcd按字节打印数据";
        desc += L"db [csp]      从csp寄存器指向的地址按字节打印数据";
    }
    else if (wstrParam == L"dd")
    {
        desc += L"按32整形打印指定内存地址的数据";
        desc += L"eg:";
        desc += L"dd 0x1234abcd 从地址0x1234abcd按32位整型打印数据";
        desc += L"dd [csp]      从csp寄存器指向的地址按32位整型打印数据";
    }
    else if (wstrParam == L"du")
    {
        desc += L"按宽字符串打印指定内存地址的数据";
        desc += L"eg:";
        desc += L"dd 0x1234abcd 从地址0x1234abcd按宽字符打印数据";
        desc += L"dd [csp]      从csp寄存器指向的地址按宽字符打印数据";
    }
    else if (wstrParam == L"r")
    {
        desc += L"打印或者修改当前线程的寄存器值";
        desc += L"eg:";
        desc += L"r csp=0x11223344 将csp寄存器值设置为0x11223344";
        desc += L"r                展示当前所有的寄存器值";
    }
    else if (wstrParam == L"sc")
    {
        desc += L"运行指定的脚本";
        desc += L"eg:";
        desc += L"sc print  运行脚本目录下名称为print的脚本";
    }
    else
    {
        desc += L"没有该命令的说明";
    }
    */
    //return DbgCmdResult(em_dbgstat_succ, desc);
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdDb(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
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
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);
    dwAddr = script.Compile(strAddr);

    CmdReplyResult result;
    CMemoryOperator mhlpr(mProcDbgger->GetDbgProc());
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
        result.mCmdShow += mProcDbgger->GetPrintStr(szData, dwRead);
        result.mCmdShow += "\n";

        if (dwRead != dwReadSize)
        {
            break;
        }
        dwAddr += 16;
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdDd(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

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

CmdReplyResult CProcCmd::OnCmdDu(const mstring &strCmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        return CmdReplyResult();
    }

    CMemoryOperator mhlpr(mProcDbgger->GetDbgProc());
    ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

    /*
    //CSyntaxDescHlpr desc;
    desc.FormatDesc(wstrData.c_str(), COLOUR_DATA);
    return mstring(em_dbgstat_succ, desc.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdKv(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    list<STACKFRAME64> vStack = mProcDbgger->GetStackFrame(cmdParam);
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
        single.mFunction = FormatA("%hs", mProcDbgger->GetSymFromAddr(it->AddrPC.Offset).c_str());
        callSet.mCallStack.push_back(single);

        pf << single.mAddr << single.mReturn << single.mParam0 << single.mParam1 << single.mParam2 << single.mParam3 << single.mFunction << line_end;
    }
    result.mCmdCode = 0;
    result.mResultMode = mode;
    result.mCmdResult = EncodeCmdCallStack(callSet);
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdTc(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.trim();

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    CmdReplyResult reply;

    DWORD dw = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = mProcDbgger->m_vThreadMap.begin() ; it != mProcDbgger->m_vThreadMap.end() ; it++, dw++)
    {
        if (dwSerial == dw || dwSerial == it->m_dwThreadId)
        {
            mProcDbgger->m_dwCurrentThreadId = it->m_dwThreadId;
            reply.mCmdShow = FormatA("切换至%d号线程成功，当前线程%x", dw, it->m_dwThreadId);
            return reply;
        }
    }
    reply.mCmdShow = "未找到需要切换的线程";
    return reply;
}

CmdReplyResult CProcCmd::OnCmdLm(const mstring &param, DWORD mode, const CmdUserParam *pParam)
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

CmdReplyResult CProcCmd::OnCmdTs(const mstring &param, DWORD mode, const CmdUserParam *pParam)
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
