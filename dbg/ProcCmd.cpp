#include <DbgCtrl/DbgCtrlCom.h>
#include "ProcCmd.h"
#include "Script.h"
#include "memory.h"
#include "BreakPoint.h"

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
    else if (cmd == "bl")
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
    else if (cmd == "bu")
    {
        return OnCmdBu(cmdParam, mode, pParam);
    }
    else if (cmd == "be")
    {
        return OnCmdBe(cmdParam, mode, pParam);
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
    CmdReplyResult result;
    mstring str(param);
    str.trim();

    if (str.empty())
    {
        result.mCmdShow = "bp参数错误\n";
        return result;
    }

    DWORD64 dwProcAddr = 0;
    if (!GetNumFromStr(str, dwProcAddr))
    {
        dwProcAddr = GetFunAddr(str);
    }

    if (dwProcAddr)
    {
        if (GetBreakPointMgr()->SetBreakPoint(dwProcAddr, pParam))
        {
            result.mCmdShow = "设置断点成功\n";
        } else {
            result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
        }
    } else {
        result.mCmdShow = FormatA("未识别的地址 %hs\n", str.c_str());
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdBl(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    vector<BreakPointInfo> bpSet = GetBreakPointMgr()->GetBpSet();

    CmdReplyResult result;
    if (bpSet.empty())
    {
        result.mCmdShow = "尚未设置任何断点\n";
        return result;
    }

    PrintFormater pf;
    pf << "断点序号" << "断点状态" << "断点地址" << "符号位置" << line_end;
    for (vector<BreakPointInfo>::const_iterator it = bpSet.begin() ; it != bpSet.end() ; it++)
    {
        string stat;
        switch (it->mBpStat) {
            case em_bp_enable:
                stat = "启用";
                break;;
            case em_bp_disable:
                stat = "禁用";
                break;
            case em_bp_uneffect:
                stat = "未生效";
                break;
        }
        pf << FormatA("%d", it->mSerial) << stat << FormatA("0x%08x", it->mBpAddr) << it->mSymbol << line_end;
    }
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdBc(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CmdReplyResult tmp;
    mstring str(cmdParam);
    str.makelower();
    if (str == "*")
    {
        GetBreakPointMgr()->DeleteAllBp();
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

    if (GetBreakPointMgr()->DeleteBpByIndex((int)dwSerial))
    {
        tmp.mCmdShow = FormatA("已清除%02x号断点", dwSerial);
    } else {
        tmp.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }

    return tmp;
}

CmdReplyResult CProcCmd::OnCmdBu(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
    CmdReplyResult result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mCmdShow = "bu 需要断点序号\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mCmdShow = "bu 格式错误";
        return result;
    }

    if (GetBreakPointMgr()->DisableBpByIndex((int)index))
    {
        result.mCmdShow = FormatA("禁用 % 号断点成功", index);
    } else {
        result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdBe(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
    CmdReplyResult result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mCmdShow = "be 需要断点序号\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mCmdShow = "be 格式错误";
        return result;
    }

    if (GetBreakPointMgr()->EnableBpByIndex((int)index))
    {
        result.mCmdShow = FormatA("启用 % 号断点成功", index);
    } else {
        result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
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
    CmdReplyResult result;
    result.mCmdShow = "进程继续运行\n";
    return result;
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
    ctx.mCipStr = mProcDbgger->GetSymFromAddr((void *)ctx.mContext.cip).c_str();

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

    CmdReplyResult result;
    if (strParam.empty())
    {
        result.mCmdShow += "VDebug支持的命令功能概要\n";
        result.mCmdShow += "bp  在指定的内存地址或者函数设置断点\n";
        result.mCmdShow += "bl  打印已存在的断点信息\n";
        result.mCmdShow += "bc  清空指定的断点\n";
        result.mCmdShow += "tc  切换到指定的线程\n";
        result.mCmdShow += "ts  打印所有的线程信息\n";
        result.mCmdShow += "lm  打印所有的模块信息\n";
        result.mCmdShow += "cs  清空页面信息\n";
        result.mCmdShow += "u   反汇编指定的地址或者api\n";
        result.mCmdShow += "ub  向上反汇编指定的地址或者api\n";
        result.mCmdShow += "uf  反汇编指定的函数\n";
        result.mCmdShow += "g   继续运行调试进程\n";
        result.mCmdShow += "gu  运行到调用返回\n";
        result.mCmdShow += "kv  打印调用栈和参数信息\n";
        result.mCmdShow += "db  按字节打印指定内存地址的数据\n";
        result.mCmdShow += "dd  按32整形打印指定内存地址的数据\n";
        result.mCmdShow += "du  按宽字符串打印指定内存地址的数据\n";
        result.mCmdShow += "r   打印或者修改当前线程的寄存器值\n";
        result.mCmdShow += "sc  运行指定的脚本\n";
    } else if (strParam == "bp")
    {
        result.mCmdShow += "在指定的内存地址或者函数设置断点\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "bp 0x1122aabb             在内存0x1122aabb位置下断点\n";
        result.mCmdShow += "bp kernelbase!createfilew 在kernelbase模块导出的createfilew函数上下断点\n";
    } else if (strParam == "bl")
    {
        result.mCmdShow += "打印已存在的断点信息\n";
    }
    else if (strParam == "bc")
    {
        result.mCmdShow += "清除指定的断点\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "bc *  清除当前所有的断点\n";
        result.mCmdShow += "bc 1  清除编号为1的断点\n";
    }
    else if (strParam == "tc")
    {
        result.mCmdShow += "切换到指定线程\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "tc 1028  切换到线程tid为1028的线程\n";
        result.mCmdShow += "tc 1     切换到序号为1的线程\n";
    }
    else if (strParam == "ts")
    {
        result.mCmdShow += "打印当前所有的线程信息\n";
    }
    else if (strParam == "lm")
    {
        result.mCmdShow += "打印当前加载的所有的模块信息\n";
    }
    else if (strParam == "cls")
    {
        result.mCmdShow += "清楚当前屏幕上的信息\n";
    }
    else if (strParam == "u")
    {
        result.mCmdShow += "反汇编指定的地址或者api\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "u 0x1028ffee              从0x1028ffee位置反汇编\n";
        result.mCmdShow += "u kernelbase!createfilew  从kernelbase!createfilew起始的位置反汇编\n";
    }
    else if (strParam == "ub")
    {
        result.mCmdShow += "向上反汇编指定的地址或者api\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "u 0x1028ffee              从0x1028ffee位置向上反汇编\n";
        result.mCmdShow += "u kernelbase!createfilew  从kernelbase!createfilew起始的位置向上反汇编\n";
    }
    else if (strParam == "uf")
    {
        result.mCmdShow += "反汇编指定的函数\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "uf kernel32!createfilew  反汇编函数kernel32!createfilew\n";
        result.mCmdShow += "uf 0x1234abcd            反汇编位于0x1234abcd的函数调用\n";
    }
    else if (strParam == "g")
    {
        result.mCmdShow += "继续运行调试进程\n";
    }
    else if (strParam == "gu")
    {
        result.mCmdShow += "运行到调用返回\n";
    }
    else if (strParam == "kv")
    {
        result.mCmdShow += "打印调用栈和参数信息\n";
    }
    else if (strParam == "db")
    {
        result.mCmdShow += "按字节打印指定内存地址的数据\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "db 0x1234abcd 从地址0x1234abcd按字节打印数据\n";
        result.mCmdShow += "db [csp]      从csp寄存器指向的地址按字节打印数据\n";
    }
    else if (strParam == "dd")
    {
        result.mCmdShow += "按32整形打印指定内存地址的数据\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "dd 0x1234abcd 从地址0x1234abcd按32位整型打印数据\n";
        result.mCmdShow += "dd [csp]      从csp寄存器指向的地址按32位整型打印数据\n";
    }
    else if (strParam == "du")
    {
        result.mCmdShow += "按宽字符串打印指定内存地址的数据\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "dd 0x1234abcd 从地址0x1234abcd按宽字符打印数据\n";
        result.mCmdShow += "dd [csp]      从csp寄存器指向的地址按宽字符打印数据\n";
    }
    else if (strParam == "r")
    {
        result.mCmdShow += "打印或者修改当前线程的寄存器值\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "r csp=0x11223344 将csp寄存器值设置为0x11223344\n";
        result.mCmdShow += "r                展示当前所有的寄存器值\n";
    }
    else if (strParam == "sc")
    {
        result.mCmdShow += "运行指定的脚本\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "sc print  运行脚本目录下名称为print的脚本\n";
    }
    else
    {
        result.mCmdShow += "没有该命令的说明\n";
    }
    return result;
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
        if (!dwRead || dwReadSize != dwRead)
        {
            result.mCmdShow = FormatA("读取内存位置 0x%08x 内容失败\n", dwAddr);
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

    CmdReplyResult result;
    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        result.mCmdShow = "语法错误\n";
    } else {
        CMemoryOperator mhlpr(mProcDbgger->GetDbgProc());
        ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mCmdShow = "没有读到有效的字符串数据";
        } else {
            result.mCmdShow = WtoA(strData) + "\n";
        }
    }
    return result;
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
        single.mFunction = FormatA("%hs", mProcDbgger->GetSymFromAddr((void *)it->AddrPC.Offset).c_str());
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
    list<DbgModuleInfo> moduleSet = mProcDbgger->GetModuleInfo();

    PrintFormater pf;
    pf << "起始地址" << "结束地址" << "版本信息" << "模块路径" << line_end;
    for (list<DbgModuleInfo>::const_iterator it = moduleSet.begin() ; it != moduleSet.end() ; it++)
    {
        string a = FormatA("0x%08x", it->m_dwBaseOfImage);
        string b = FormatA("0x%08x", it->m_dwEndAddr);

        char version[128] = {0};
        GetPeVersionA(it->m_strDllPath.c_str(), version, 128);
        string c = version;
        pf << a << b << c << it->m_strDllPath << line_end;
    }

    CmdReplyResult result;
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdTs(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    list<ThreadInformation> threadSet = mProcDbgger->GetCurrentThreadSet();

    PrintFormater pf;
    pf << "线程ID" << "启动时间" << "状态" << "启动位置" << line_end;
    for (list<ThreadInformation>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++)
    {
        string a = FormatA("0x%08x", it->m_dwThreadId);

        SYSTEMTIME time = {0};
        FileTimeToSystemTime(&(it->m_vCreateTime), &time);
        string b = FormatA(
            "%04d-%02d-%02d %02d:%02d:%02d %03d ",
            time.wYear,
            time.wMonth,
            time.wDay,
            time.wHour,
            time.wMinute,
            time.wSecond,
            time.wMilliseconds
            );
        string c = "正常运行";
        string d = FormatA("0x%08x %hs", it->m_dwStartAddr, mProcDbgger->GetSymFromAddr(it->m_dwStartAddr).c_str());
        pf << a << b << c << d << line_end;
    }

    CmdReplyResult result;
    result.mCmdShow = pf.GetResult();
    return result;
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
    //return CmdReplyResult();
}
