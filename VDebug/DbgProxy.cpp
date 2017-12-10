#include "DbgProxy.h"

list<CCmdHelpDesc> *CDbggerProxy::ms_pHelpDesc = NULL;

void CDbggerProxy::OnCmdHlprRegister()
{
    CSyntaxDescHlpr hlpr;
    CCmdHelpDesc desc;
    //bp
    desc.m_wstrName = L"bp";
    hlpr.FormatDesc(L"bp "), hlpr.FormatDesc(L"在指定的内存地址或者函数设置断点");
    desc.m_vCmdDesc = hlpr.GetResult();
    hlpr.Clear();
    hlpr.FormatDesc(L"bp 0x1234abcd             "), hlpr.FormatDesc(L"在内存地址0x1234abcd处设置断点");
    hlpr.NextLine();
    hlpr.FormatDesc(L"bp kernelbase!createfilew "), hlpr.FormatDesc(L"在函数kernelbase模块导出的createfilew函数上设置断点");
    desc.m_vCmdExample = hlpr.GetResult();
    hlpr.Clear();
    RegisterCmdHlpr(desc);
    //
}

void CDbggerProxy::RegisterCmdHlpr(const CCmdHelpDesc &vDesc)
{
    if (!ms_pHelpDesc)
    {
        ms_pHelpDesc = new list<CCmdHelpDesc>();
    }
    ms_pHelpDesc->push_back(vDesc);
}

DbgCmdResult CDbggerProxy::GetAllCmdDesc()
{
    SyntaxDesc desc;
    for (list<CCmdHelpDesc>::const_iterator it = ms_pHelpDesc->begin() ; it != ms_pHelpDesc->end() ; it++)
    {
        desc += it->m_vCmdExample;
    }
    return DbgCmdResult(em_dbgstat_succ, desc);
}

DbgCmdResult CDbggerProxy::OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    return GetAllCmdDesc();
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}