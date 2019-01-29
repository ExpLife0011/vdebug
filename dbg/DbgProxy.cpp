#include "DbgProxy.h"

void CDbggerProxy::OnCmdHlprRegister()
{
}

CmdReplyResult CDbggerProxy::OnCmdHelp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
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

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}