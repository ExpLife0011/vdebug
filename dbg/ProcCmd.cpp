#include <DbgCtrl/DbgCtrlCom.h>
#include "ProcCmd.h"
#include "Script.h"
#include "memory.h"
#include "BreakPoint.h"
#include "DbgCommon.h"
#include "./DescParser/DescPrinter.h"

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

CtrlReply CProcCmd::OnCommand(const mstring &cmd, const mstring &cmdParam, const CmdUserParam *pParam) {
    if (cmd == "bp")
    {
        return OnCmdBp(cmdParam, pParam);
    }
    else if (cmd == "bl")
    {
        return OnCmdBl(cmdParam, pParam);
    }
    else if (cmd == "bk")
    {
        DebugBreakProcess(mProcDbgger->GetDbgProc());
    }
    else if (cmd == "bc")
    {
        return OnCmdBc(cmdParam, pParam);
    }
    else if (cmd == "bu")
    {
        return OnCmdBu(cmdParam, pParam);
    }
    else if (cmd == "be")
    {
        return OnCmdBe(cmdParam, pParam);
    }
    //切换到指定线程
    else if (cmd == "tc")
    {
        return OnCmdTc(cmdParam, pParam);
    }
    //展示指定线程
    else if (cmd == "ts")
    {
        return OnCmdTs(cmdParam, pParam);
    }
    //展示模块信息
    else if (cmd == "lm")
    {
        return OnCmdLm(cmdParam, pParam);
    }
    else if (cmd == "u")
    {
        return OnCmdDisass(cmdParam, pParam);
    }
    else if (cmd == "ub")
    {
        return OnCmdUb(cmdParam, pParam);
    }
    else if (cmd == "uf")
    {
        return OnCmdUf(cmdParam, pParam);
    }
    else if (cmd == "g")
    {
        return OnCmdGo(cmdParam, pParam);
    }
    //执行到调用返回
    else if (cmd == "gu")
    {
        return OnCmdGu(cmdParam, pParam);
    }
    else if (cmd == "kv")
    {
        return OnCmdKv(cmdParam, pParam);
    }
    else if (cmd == "db")
    {
        return OnCmdDb(cmdParam, pParam);
    }
    else if (cmd == "dd")
    {
        return OnCmdDd(cmdParam, pParam);
    }
    else if (cmd == "du")
    {
        return OnCmdDu(cmdParam, pParam);
    } else if (cmd == "da")
    {
        return OnCmdDa(cmdParam, pParam);
    }
    else if (cmd == "r")
    {
        return OnCmdReg(cmdParam, pParam);
    }
    else if (cmd == "sc")
    {
        return OnCmdScript(cmdParam, pParam);
    } else if (cmd == "pf")
    {
        return OnCmdPf(cmdParam, pParam);
    }
    else if (cmd == "help" || cmd == "h")
    {
        return OnCmdHelp(cmdParam, pParam);
    }

    CtrlReply reply;
    reply.mShow = mstring("不支持的命令:") + cmd + "\n";
    return reply;
}

CtrlReply CProcCmd::OnCmdBp(const mstring &param, const CmdUserParam *pParam)
{
    CtrlReply result;
    mstring str(param);
    str.trim();

    if (str.empty())
    {
        result.mShow = "bp参数错误\n";
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
            result.mShow = "设置断点成功\n";
        } else {
            result.mShow = GetBreakPointMgr()->GetLastErr() + "\n";
        }
    } else {
        result.mShow = FormatA("未识别的地址 %hs\n", str.c_str());
    }
    return result;
}

CtrlReply CProcCmd::OnCmdBl(const mstring &param, const CmdUserParam *pParam)
{
    vector<BreakPointInfo> bpSet = GetBreakPointMgr()->GetBpSet();

    CtrlReply result;
    if (bpSet.empty())
    {
        result.mShow = "尚未设置任何断点\n";
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
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CProcCmd::OnCmdBc(const mstring &cmdParam, const CmdUserParam *pParam)
{
    CtrlReply tmp;
    mstring str(cmdParam);
    str.makelower();
    if (str == "*")
    {
        GetBreakPointMgr()->DeleteAllBp();
        tmp.mShow = "已清空所有断点";
        return tmp;
    }

    if (!IsNumber(str))
    {
        tmp.mStatus = DBG_CMD_SYNTAX_ERR;
        tmp.mShow = "bc 语法错误";
        return tmp;
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    if (GetBreakPointMgr()->DeleteBpByIndex((int)dwSerial))
    {
        tmp.mShow = FormatA("已清除%02x号断点", dwSerial);
    } else {
        tmp.mShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }

    return tmp;
}

CtrlReply CProcCmd::OnCmdBu(const mstring &cmdParam, const CmdUserParam *pParam) {
    CtrlReply result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mShow = "bu 需要断点序号\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mShow = "bu 格式错误";
        return result;
    }

    if (GetBreakPointMgr()->DisableBpByIndex((int)index))
    {
        result.mShow = FormatA("禁用 % 号断点成功", index);
    } else {
        result.mShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
}

CtrlReply CProcCmd::OnCmdBe(const mstring &cmdParam, const CmdUserParam *pParam) {
    CtrlReply result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mShow = "be 需要断点序号\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mShow = "be 格式错误";
        return result;
    }

    if (GetBreakPointMgr()->EnableBpByIndex((int)index))
    {
        result.mShow = FormatA("启用 % 号断点成功", index);
    } else {
        result.mShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
}

CtrlReply CProcCmd::OnCmdDisass(const mstring &wstrCmdParam, const CmdUserParam *pParam)
{
    mstring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = 0;
    mstring strAddr;

    CtrlReply result;
    dwDisasmSize = GetSizeAndParam(wstr, strAddr);
    if (!dwDisasmSize)
    {
        result.mStatus = DBG_CMD_SYNTAX_ERR;
        result.mShow = "语法错误";
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
        result.mStatus = DBG_CMD_READMEM_ERR;
        result.mShow = FormatA("获取%hs地址失败", strAddr.c_str());
        return result;
    }

    if (!mProcDbgger->DisassWithSize(dwAddr, dwDisasmSize, result))
    {
        result.mShow = FormatA("反汇编地址 0x%08x 失败\n", (DWORD)dwAddr);
    }
    return result;
}

CtrlReply CProcCmd::OnCmdUb(const mstring &param, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CtrlReply reply;
    if (!dwDisasmSize)
    {
        reply.mStatus = DBG_CMD_UNKNOW_ERR;
        reply.mShow = "获取反汇编地址长度失败";
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
        reply.mStatus = DBG_CMD_UNKNOW_ERR;
        reply.mShow = "ub语法错误";
        return reply;
    }

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;

    if (!mProcDbgger->DisassWithAddr(dwStartAddr, dwEndAddr, reply))
    {
        reply.mShow = FormatA("反汇编地址 0x%08x 失败\n", (DWORD)dwStartAddr);
    }
    return reply;
}

CtrlReply CProcCmd::OnCmdUf(const mstring &param, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CtrlReply reply;
    if (!dwDisasmSize)
    {
        reply.mStatus = DBG_CMD_UNKNOW_ERR;
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
        reply.mStatus = DBG_CMD_SYNTAX_ERR;
        return reply;
    }

    if (!mProcDbgger->DisassUntilRet(dwAddr, reply))
    {
        reply.mShow = FormatA("反汇编地址 0x%08x 失败\n", (DWORD)dwAddr);
    }
    return reply;
}

CtrlReply CProcCmd::OnCmdGo(const mstring &param, const CmdUserParam *pParam)
{
    mProcDbgger->Run();
    CtrlReply result;
    result.mShow = "进程继续运行\n";
    return result;
}

CtrlReply CProcCmd::OnCmdGu(const mstring &param, const CmdUserParam *pParam)
{
    mProcDbgger->RunExitProc();

    CtrlReply reply;
    reply.mShow = "执行gu成功";
    return reply;
}

CtrlReply CProcCmd::OnCmdReg(const mstring &cmdParam, const CmdUserParam *pParam)
{
    RegisterContent ctx;
    ctx.mContext = mProcDbgger->GetCurrentContext();
    DbgModuleInfo module = mProcDbgger->GetModuleFromAddr((DWORD64)ctx.mContext.cip);
    ctx.mCipStr = CDbgCommon::GetSymFromAddr((DWORD64)ctx.mContext.cip, module.m_strDllName, module.m_dwBaseOfImage);;

    CtrlReply result;
    result.mResult = EncodeCmdRegister(ctx);

    PrintFormater pf;
    pf << FormatA("eax=0x%08x", ctx.mContext.cax) << FormatA("ebx=0x%08x", ctx.mContext.cbx);
    pf << FormatA("ecx=0x%08x", ctx.mContext.ccx) << FormatA("edx=0x%08x", ctx.mContext.cdx) << line_end;

    pf << FormatA("esi=0x%08x", ctx.mContext.csi) << FormatA("edi=0x%08x", ctx.mContext.cdi);
    pf << FormatA("eip=0x%08x", ctx.mContext.cip) << FormatA("esp=0x%08x", ctx.mContext.csp) << line_end;

    pf << FormatA("ebp=0x%08x", ctx.mContext.cbp) << space << space << space << line_end;
    result.mShow = pf.GetResult();
    pf.Reset();

    pf << ctx.mCipStr << line_end;
    result.mShow += pf.GetResult();
    return result;
}

CtrlReply CProcCmd::OnCmdScript(const mstring &cmdParam, const CmdUserParam *pParam)
{
    return CtrlReply();
}

CtrlReply CProcCmd::OnCmdHelp(const mstring &param, const CmdUserParam *pParam)
{
    mstring strParam(param);
    strParam.makelower();
    strParam.trim();

    CtrlReply result;
    if (strParam.empty())
    {
        result.mShow += "VDebug支持的命令功能概要\n";
        result.mShow += "bp  在指定的内存地址或者函数设置断点\n";
        result.mShow += "bl  打印已存在的断点信息\n";
        result.mShow += "bc  清空指定的断点\n";
        result.mShow += "tc  切换到指定的线程\n";
        result.mShow += "ts  打印所有的线程信息\n";
        result.mShow += "lm  打印所有的模块信息\n";
        result.mShow += "cls 清空页面信息\n";
        result.mShow += "u   反汇编指定的地址或者api\n";
        result.mShow += "ub  向上反汇编指定的地址或者api\n";
        result.mShow += "uf  反汇编指定的函数\n";
        result.mShow += "g   继续运行调试进程\n";
        result.mShow += "gu  运行到调用返回\n";
        result.mShow += "kv  打印调用栈和参数信息\n";
        result.mShow += "db  按字节打印指定内存地址的数据\n";
        result.mShow += "dd  按32整形打印指定内存地址的数据\n";
        result.mShow += "du  按宽字符串打印指定内存地址的数据\n";
        result.mShow += "r   打印或者修改当前线程的寄存器值\n";
        result.mShow += "pf  输出函数类型\n";
        result.mShow += "sc  运行指定的脚本\n";
    } else if (strParam == "bp")
    {
        result.mShow += "在指定的内存地址或者函数设置断点\n";
        result.mShow += "eg:\n";
        result.mShow += "bp 0x1122aabb             在内存0x1122aabb位置下断点\n";
        result.mShow += "bp kernelbase!createfilew 在kernelbase模块导出的createfilew函数上下断点\n";
    } else if (strParam == "bl")
    {
        result.mShow += "打印已存在的断点信息\n";
    }
    else if (strParam == "bc")
    {
        result.mShow += "清除指定的断点\n";
        result.mShow += "eg:\n";
        result.mShow += "bc *  清除当前所有的断点\n";
        result.mShow += "bc 1  清除编号为1的断点\n";
    }
    else if (strParam == "tc")
    {
        result.mShow += "切换到指定线程\n";
        result.mShow += "eg:\n";
        result.mShow += "tc 1028  切换到线程tid为1028的线程\n";
        result.mShow += "tc 1     切换到序号为1的线程\n";
    }
    else if (strParam == "ts")
    {
        result.mShow += "打印当前所有的线程信息\n";
    }
    else if (strParam == "lm")
    {
        result.mShow += "打印当前加载的所有的模块信息\n";
    }
    else if (strParam == "cls")
    {
        result.mShow += "清楚当前屏幕上的信息\n";
    }
    else if (strParam == "u")
    {
        result.mShow += "反汇编指定的地址或者api\n";
        result.mShow += "eg:\n";
        result.mShow += "u 0x1028ffee              从0x1028ffee位置反汇编\n";
        result.mShow += "u kernelbase!createfilew  从kernelbase!createfilew起始的位置反汇编\n";
    }
    else if (strParam == "ub")
    {
        result.mShow += "向上反汇编指定的地址或者api\n";
        result.mShow += "eg:\n";
        result.mShow += "u 0x1028ffee              从0x1028ffee位置向上反汇编\n";
        result.mShow += "u kernelbase!createfilew  从kernelbase!createfilew起始的位置向上反汇编\n";
    }
    else if (strParam == "uf")
    {
        result.mShow += "反汇编指定的函数\n";
        result.mShow += "eg:\n";
        result.mShow += "uf kernel32!createfilew  反汇编函数kernel32!createfilew\n";
        result.mShow += "uf 0x1234abcd            反汇编位于0x1234abcd的函数调用\n";
    }
    else if (strParam == "g")
    {
        result.mShow += "继续运行调试进程\n";
    }
    else if (strParam == "gu")
    {
        result.mShow += "运行到调用返回\n";
    }
    else if (strParam == "kv")
    {
        result.mShow += "打印调用栈和参数信息\n";
    }
    else if (strParam == "db")
    {
        result.mShow += "按字节打印指定内存地址的数据\n";
        result.mShow += "eg:\n";
        result.mShow += "db 0x1234abcd 从地址0x1234abcd按字节打印数据\n";
        result.mShow += "db [csp]      从csp寄存器指向的地址按字节打印数据\n";
    }
    else if (strParam == "dd")
    {
        result.mShow += "按32整形打印指定内存地址的数据\n";
        result.mShow += "eg:\n";
        result.mShow += "dd 0x1234abcd 从地址0x1234abcd按32位整型打印数据\n";
        result.mShow += "dd [csp]      从csp寄存器指向的地址按32位整型打印数据\n";
    }
    else if (strParam == "du")
    {
        result.mShow += "按宽字符串打印指定内存地址的数据\n";
        result.mShow += "eg:\n";
        result.mShow += "dd 0x1234abcd 从地址0x1234abcd按宽字符打印数据\n";
        result.mShow += "dd [csp]      从csp寄存器指向的地址按宽字符打印数据\n";
    }
    else if (strParam == "r")
    {
        result.mShow += "打印或者修改当前线程的寄存器值\n";
        result.mShow += "eg:\n";
        result.mShow += "r csp=0x11223344 将csp寄存器值设置为0x11223344\n";
        result.mShow += "r                展示当前所有的寄存器值\n";
    }
    else if (strParam == "sc")
    {
        result.mShow += "运行指定的脚本\n";
        result.mShow += "eg:\n";
        result.mShow += "sc print  运行脚本目录下名称为print的脚本\n";
    }
    else
    {
        result.mShow += "没有该命令的说明\n";
    }
    return result;
}

CtrlReply CProcCmd::OnCmdDb(const mstring &cmdParam, const CmdUserParam *pParam)
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

    CtrlReply result;
    CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
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
            result.mShow = FormatA("读取内存位置 0x%08x 内容失败\n", dwAddr);
            break;
        }

        result.mShow += FormatA("%08x  ", dwAddr);
        int j = 0;
        for (j = 0 ; j < 16 ; j++)
        {
            if (j < (int)dwRead)
            {
                result.mShow += FormatA("%02x ", (BYTE)szData[j]);
            }
            else
            {
                result.mShow += "   ";
            }
        }

        result.mShow += " ";
        result.mShow += mProcDbgger->GetPrintStr(szData, dwRead);
        result.mShow += "\n";
        dwAddr += 16;
    }
    return result;
}

CtrlReply CProcCmd::OnCmdDd(const mstring &cmdParam, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    CtrlReply result;
    DWORD64 dwAddr = script.Compile(cmdParam);
    if (!dwAddr)
    {
        result.mShow = FormatA("编译 %hs 表达式失败", cmdParam.c_str());
        return result;
    }

    DWORD dwDataSize = 64;
    PrintFormater pf;
    pf << "数据地址" << "数据内容" << space << space << space << line_end;

    CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
    for (int i = 0 ; i < (int)dwDataSize ; i += 16)
    {
        char szData[16] = {0};
        DWORD dwReadSize = 0;
        mhlpr.MemoryReadSafe(dwAddr, szData, sizeof(szData), &dwReadSize);
        if (!dwReadSize)
        {
            break;
        }

        pf << FormatA("0x%08x", dwAddr);

        for (int j = 0 ; j < (int)dwReadSize / 4 ; j += 1)
        {
            pf << FormatA("%08x ", *((DWORD *)szData + j));
        }
        pf << line_end;
        dwAddr += 16;
    }
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CProcCmd::OnCmdDu(const mstring &strCmdParam, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    CtrlReply result;
    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        result.mShow = "语法错误\n";
    } else {
        CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
        ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mShow = "没有读到有效的字符串数据";
        } else {
            result.mShow = WtoA(strData) + "\n";
        }
    }
    return result;
}

CtrlReply CProcCmd::OnCmdDa(const mstring &strCmdParam, const CmdUserParam *pParam) {
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    CtrlReply result;
    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        result.mShow = "语法错误\n";
    } else {
        CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
        mstring strData = mhlpr.MemoryReadStrGbk(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mShow = "没有读到有效的字符串数据";
        } else {
            result.mShow = strData + "\n";
        }
    }
    return result;
}

CtrlReply CProcCmd::OnCmdKv(const mstring &cmdParam, const CmdUserParam *pParam)
{
    list<STACKFRAME64> vStack = mProcDbgger->GetStackFrame(cmdParam);
    CtrlReply result;
    if (vStack.empty())
    {
        return result;
    }

    CallStackData callSet;
    CallStackSingle single;
    PrintFormater pf;
    pf << "内存地址" << "返回地址" << "参数列表" << space << space << space << "符号名称" << line_end;
    for (list<STACKFRAME64>::const_iterator it = vStack.begin() ; it != vStack.end() ; it++)
    {
        single.mAddr = FormatA("%08x", it->AddrPC.Offset);
        single.mReturn = FormatA("%08x", it->AddrReturn);
        single.mParam0 = FormatA("%08x", it->Params[0]);
        single.mParam1 = FormatA("%08x", it->Params[1]);
        single.mParam2 = FormatA("%08x", it->Params[2]);
        single.mParam3 = FormatA("%08x", it->Params[3]);

        DbgModuleInfo module = mProcDbgger->GetModuleFromAddr((DWORD64)it->AddrPC.Offset);
        single.mFunction = FormatA("%hs", CDbgCommon::GetSymFromAddr((DWORD64)it->AddrPC.Offset, module.m_strDllName, module.m_dwBaseOfImage).c_str());
        callSet.mCallStack.push_back(single);

        pf << single.mAddr << single.mReturn << single.mParam0 << single.mParam1 << single.mParam2 << single.mParam3 << single.mFunction << line_end;
    }
    result.mStatus = 0;
    result.mResult = EncodeCmdCallStack(callSet);
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CProcCmd::OnCmdTc(const mstring &param, const CmdUserParam *pParam)
{
    mstring str(param);
    str.trim();

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    CtrlReply reply;
    if (dwSerial >= mProcDbgger->m_vThreadMap.size())
    {
        reply.mShow = "未找到需要切换的线程";
    } else {
        mProcDbgger->mCurrentThread = mProcDbgger->m_vThreadMap[(int)dwSerial];
        reply.mShow = FormatA("切换至0x%04x号线程成功，当前线程0x%x\n", (DWORD)dwSerial, mProcDbgger->mCurrentThread.m_dwThreadId);
    }
    return reply;
}

CtrlReply CProcCmd::OnCmdLm(const mstring &param, const CmdUserParam *pParam)
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

    CtrlReply result;
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CProcCmd::OnCmdPf(const mstring &param, const CmdUserParam *pParam) {
    CtrlReply result;
    TITAN_ENGINE_CONTEXT_t ctx = mProcDbgger->GetCurrentContext();
    DbgModuleInfo module = mProcDbgger->GetModuleFromAddr(ctx.cip);

    mstring pp = param;
    if (pp.startwith("-s") || pp.startwith("/s")) {
        mstring fun = pp.substr(2, pp.size() - 2);
        fun.trim();

        if (fun.empty())
        {
            result.mShow = "没有函数信息";
        } else {
            result.mShow = CDescPrinter::GetInst()->GetProcStrByName("", fun);
        }
    } else {
        mstring proc = CDbgCommon::GetProcSymFromAddr(ctx.cip, module.m_strDllName, module.m_dwBaseOfImage);
        if (proc.empty()) {
            result.mShow = FormatA("未识别的函数地址 0x%08x\n", ctx.cip);
        } else {
            if (mstring::npos != proc.find("+0x"))
            {
                result.mShow = FormatA("当前cip未处于函数起始位置 %hs 0x%08x\n", proc.c_str(), ctx.cip);
            } else {
                CMemoryProc reader(CProcDbgger::GetInstance()->GetDbgProc());
                CDescPrinter::GetInst()->SetMemoryReader(&reader);
                mstring procStr = CDescPrinter::GetInst()->GetProcStrByName(module.m_strDllName, proc, (LPVOID)((const char *)ctx.csp + 4));

                if (procStr.empty())
                {
                    result.mShow = FormatA("调试器中不存在%hs模块%hs的信息\n", module.m_strDllName.c_str(), proc.c_str());
                } else {
                    result.mShow = procStr;
                }
            }
        }
    }
    return result;
}

CtrlReply CProcCmd::OnCmdTs(const mstring &param, const CmdUserParam *pParam)
{
    vector<DbgProcThreadInfo> threadSet = mProcDbgger->GetThreadCache();

    PrintFormater pf;
    pf << "序号" <<"线程id" << "启动时间" << "状态" << "启动位置" << line_end;
    int index = 0;
    for (vector<DbgProcThreadInfo>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++, index++)
    {
        string a = FormatA("0x%04x", index);
        string b = FormatA("0x%08x(%d)", it->m_dwThreadId, it->m_dwThreadId);
        string c = it->mStartTimeStr;
        string d = "正常运行";

        DbgModuleInfo module = mProcDbgger->GetModuleFromAddr((DWORD64)it->m_dwStartAddr);
        mstring symbol = CDbgCommon::GetSymFromAddr((DWORD64)it->m_dwStartAddr, module.m_strDllName, module.m_dwBaseOfImage);
        mstring tmp = symbol;

        string e = FormatA("0x%08x %hs", (DWORD)it->m_dwStartAddr, tmp.c_str());
        pf << a << b << c << d << e << line_end;
    }

    CtrlReply result;
    result.mShow = pf.GetResult();
    return result;
}
