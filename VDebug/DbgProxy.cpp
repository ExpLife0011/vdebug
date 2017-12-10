#include "DbgProxy.h"

list<CCmdHelpDesc> CDbggerProxy::ms_vHelpDesc;
SyntaxDesc CDbggerProxy::ms_vCmdSummary;

void CDbggerProxy::OnCmdHlprRegister()
{
    /*
    else if (wstrCmd == L"bl")
    else if (wstrCmd == L"bk")
    else if (wstrCmd == L"bc")
    else if (wstrCmd == L"tc")
    else if (wstrCmd == L"ts")
    else if (wstrCmd == L"lm")
    else if (wstrCmd == L"cls")
    else if (wstrCmd == L"u")
    else if (wstrCmd == L"ub")
    else if (wstrCmd == L"uf")
    else if (wstrCmd == L"g")
    else if (wstrCmd == L"gu")
    else if (wstrCmd == L"kv")
    else if (wstrCmd == L"db")
    else if (wstrCmd == L"dd")
    else if (wstrCmd == L"du")
    else if (wstrCmd == L"r")
    else if (wstrCmd == L"sc")
    else if (wstrCmd == L"help" || wstrCmd == L"h")
    */
    CEasySyntaxHlpr hlpr;
    //简述
    hlpr.AppendWord(L"bp"), hlpr.AppendWord(L"在指定的内存地址或者函数设置断点");
    hlpr.NextLine();
    hlpr.AppendWord(L"bl"), hlpr.AppendWord(L"打印已存在的断点信息");
    hlpr.NextLine();
    hlpr.AppendWord(L"bc"), hlpr.AppendWord(L"清空指定的断点");
    hlpr.NextLine();
    hlpr.AppendWord(L"tc"), hlpr.AppendWord(L"切换到指定的线程");
    hlpr.NextLine();
    hlpr.AppendWord(L"ts"), hlpr.AppendWord(L"打印所有的线程信息");
    hlpr.NextLine();
    hlpr.AppendWord(L"lm"), hlpr.AppendWord(L"打印所有的模块信息");
    hlpr.NextLine();
    hlpr.AppendWord(L"cls"), hlpr.AppendWord(L"清空页面信息");
    hlpr.NextLine();
    hlpr.AppendWord(L"u"), hlpr.AppendWord(L"反汇编指定的地址或者api");
    hlpr.NextLine();
    hlpr.AppendWord(L"ub"), hlpr.AppendWord(L"向上反汇编指定的地址或者");
    hlpr.NextLine();
    hlpr.AppendWord(L"uf"), hlpr.AppendWord(L"反汇编指定的函数");
    hlpr.NextLine();
    hlpr.AppendWord(L"g"), hlpr.AppendWord(L"继续运行调试进程");
    hlpr.NextLine();
    hlpr.AppendWord(L"gu"), hlpr.AppendWord(L"运行到调用返回");
    hlpr.NextLine();
    hlpr.AppendWord(L"kv"), hlpr.AppendWord(L"打印调用栈和参数信息");
    hlpr.NextLine();
    hlpr.AppendWord(L"db"), hlpr.AppendWord(L"按字节打印指定内存地址的数据");
    hlpr.NextLine();
    hlpr.AppendWord(L"dd"), hlpr.AppendWord(L"按32整形打印指定内存地址的数据");
    hlpr.NextLine();
    hlpr.AppendWord(L"du"), hlpr.AppendWord(L"按宽字符串打印指定内存地址的数据");
    hlpr.NextLine();
    hlpr.AppendWord(L"r"), hlpr.AppendWord(L"打印当前线程的寄存器值");
    hlpr.NextLine();
    hlpr.AppendWord(L"sc"), hlpr.AppendWord(L"运行指定的脚本");
    hlpr.NextLine();
    ms_vCmdSummary = hlpr.GetResult();
    CCmdHelpDesc desc;
    RegisterCmdHlpr(desc);
    //
}

void CDbggerProxy::RegisterCmdHlpr(const CCmdHelpDesc &vDesc)
{
}

DbgCmdResult CDbggerProxy::GetAllCmdDesc()
{
    SyntaxDesc desc;
    for (list<CCmdHelpDesc>::const_iterator it = ms_vHelpDesc.begin() ; it != ms_vHelpDesc.end() ; it++)
    {
        desc += it->m_vCmdExample;
    }
    return DbgCmdResult(em_dbgstat_succ, desc);
}

DbgCmdResult CDbggerProxy::OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"VDebug支持的命令功能概要");
    SyntaxDesc desc = hlpr.GetResult();
    desc += ms_vCmdSummary;
    return DbgCmdResult(em_dbgstat_succ, desc);
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}