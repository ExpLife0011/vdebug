#include "DbgProxy.h"

list<CCmdHelpDesc> *CDbggerProxy::ms_pHelpDesc = NULL;

void CDbggerProxy::OnCmdHlprRegister()
{
    CSyntaxDescHlpr hlpr;
    CCmdHelpDesc desc;
    //bp
    hlpr.FormatWithLength(L"bp", 12), hlpr.FormatDesc(L"在指定的内存地址或者函数设置断点");
    desc.m_vCmdDesc = hlpr.GetResult();
    hlpr.Clear();
    hlpr.FormatDesc(L"bp 0x1234abcd", COLOUR_MSG, 36), hlpr.FormatDesc(L"在内存地址0x1234abcd处设置断点");
    hlpr.NextLine();
    hlpr.FormatDesc(L"bp kernelbase!createfilew", COLOUR_MSG, 36), hlpr.FormatDesc(L"在函数kernelbase模块导出的createfilew函数上设置断点");
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
    return DbgCmdResult();
}

DbgCmdResult CDbggerProxy::OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    return DbgCmdResult(em_dbgstat_succ, L"");
}