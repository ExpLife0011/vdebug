#include "DbgProxy.h"

void CDbggerProxy::OnCmdHlprRegister()
{
}

DbgCmdResult CDbggerProxy::OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    ustring wstrParam(wstrCmdParam);
    wstrParam.makelower();
    wstrParam.trim();

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
        desc += L"ub  向上反汇编指定的地址或者";
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
    return DbgCmdResult(em_dbgstat_succ, desc);
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}