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
    //����
    hlpr.AppendWord(L"bp"), hlpr.AppendWord(L"��ָ�����ڴ��ַ���ߺ������öϵ�");
    hlpr.NextLine();
    hlpr.AppendWord(L"bl"), hlpr.AppendWord(L"��ӡ�Ѵ��ڵĶϵ���Ϣ");
    hlpr.NextLine();
    hlpr.AppendWord(L"bc"), hlpr.AppendWord(L"���ָ���Ķϵ�");
    hlpr.NextLine();
    hlpr.AppendWord(L"tc"), hlpr.AppendWord(L"�л���ָ�����߳�");
    hlpr.NextLine();
    hlpr.AppendWord(L"ts"), hlpr.AppendWord(L"��ӡ���е��߳���Ϣ");
    hlpr.NextLine();
    hlpr.AppendWord(L"lm"), hlpr.AppendWord(L"��ӡ���е�ģ����Ϣ");
    hlpr.NextLine();
    hlpr.AppendWord(L"cls"), hlpr.AppendWord(L"���ҳ����Ϣ");
    hlpr.NextLine();
    hlpr.AppendWord(L"u"), hlpr.AppendWord(L"�����ָ���ĵ�ַ����api");
    hlpr.NextLine();
    hlpr.AppendWord(L"ub"), hlpr.AppendWord(L"���Ϸ����ָ���ĵ�ַ����");
    hlpr.NextLine();
    hlpr.AppendWord(L"uf"), hlpr.AppendWord(L"�����ָ���ĺ���");
    hlpr.NextLine();
    hlpr.AppendWord(L"g"), hlpr.AppendWord(L"�������е��Խ���");
    hlpr.NextLine();
    hlpr.AppendWord(L"gu"), hlpr.AppendWord(L"���е����÷���");
    hlpr.NextLine();
    hlpr.AppendWord(L"kv"), hlpr.AppendWord(L"��ӡ����ջ�Ͳ�����Ϣ");
    hlpr.NextLine();
    hlpr.AppendWord(L"db"), hlpr.AppendWord(L"���ֽڴ�ӡָ���ڴ��ַ������");
    hlpr.NextLine();
    hlpr.AppendWord(L"dd"), hlpr.AppendWord(L"��32���δ�ӡָ���ڴ��ַ������");
    hlpr.NextLine();
    hlpr.AppendWord(L"du"), hlpr.AppendWord(L"�����ַ�����ӡָ���ڴ��ַ������");
    hlpr.NextLine();
    hlpr.AppendWord(L"r"), hlpr.AppendWord(L"��ӡ��ǰ�̵߳ļĴ���ֵ");
    hlpr.NextLine();
    hlpr.AppendWord(L"sc"), hlpr.AppendWord(L"����ָ���Ľű�");
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
    hlpr.FormatDesc(L"VDebug֧�ֵ�����ܸ�Ҫ");
    SyntaxDesc desc = hlpr.GetResult();
    desc += ms_vCmdSummary;
    return DbgCmdResult(em_dbgstat_succ, desc);
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}