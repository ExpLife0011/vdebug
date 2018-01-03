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
        desc += L"VDebug֧�ֵ�����ܸ�Ҫ";
        desc += L"bp  ��ָ�����ڴ��ַ���ߺ������öϵ�";
        desc += L"bl  ��ӡ�Ѵ��ڵĶϵ���Ϣ";
        desc += L"bc  ���ָ���Ķϵ�";
        desc += L"tc  �л���ָ�����߳�";
        desc += L"ts  ��ӡ���е��߳���Ϣ";
        desc += L"lm  ��ӡ���е�ģ����Ϣ";
        desc += L"cls ���ҳ����Ϣ";
        desc += L"u   �����ָ���ĵ�ַ����api";
        desc += L"ub  ���Ϸ����ָ���ĵ�ַ����";
        desc += L"uf  �����ָ���ĺ���";
        desc += L"g   �������е��Խ���";
        desc += L"gu  ���е����÷���";
        desc += L"kv  ��ӡ����ջ�Ͳ�����Ϣ";
        desc += L"db  ���ֽڴ�ӡָ���ڴ��ַ������";
        desc += L"dd  ��32���δ�ӡָ���ڴ��ַ������";
        desc += L"du  �����ַ�����ӡָ���ڴ��ַ������";
        desc += L"r   ��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ";
        desc += L"sc  ����ָ���Ľű�";
    }
    else if(wstrParam == L"bp")
    {
        desc += L"��ָ�����ڴ��ַ���ߺ������öϵ�";
        desc += L"eg:";
        desc += L"bp 0x1122aabb             ���ڴ�0x1122aabbλ���¶ϵ�";
        desc += L"bp kernelbase!createfilew ��kernelbaseģ�鵼����createfilew�������¶ϵ�";
    }
    else if (wstrParam == L"bl")
    {
        desc += L"��ӡ�Ѵ��ڵĶϵ���Ϣ";
    }
    else if (wstrParam == L"bc")
    {
        desc += L"���ָ���Ķϵ�";
        desc += L"eg:";
        desc += L"bc *  �����ǰ���еĶϵ�";
        desc += L"bc 1  ������Ϊ1�Ķϵ�";
    }
    return DbgCmdResult(em_dbgstat_succ, desc);
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}