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
        desc += L"VDebug֧�ֵ�����ܸ�Ҫ";
        desc += L"bp  ��ָ�����ڴ��ַ���ߺ������öϵ�";
        desc += L"bl  ��ӡ�Ѵ��ڵĶϵ���Ϣ";
        desc += L"bc  ���ָ���Ķϵ�";
        desc += L"tc  �л���ָ�����߳�";
        desc += L"ts  ��ӡ���е��߳���Ϣ";
        desc += L"lm  ��ӡ���е�ģ����Ϣ";
        desc += L"cls ���ҳ����Ϣ";
        desc += L"u   �����ָ���ĵ�ַ����api";
        desc += L"ub  ���Ϸ����ָ���ĵ�ַ����api";
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
    else if (wstrParam == L"tc")
    {
        desc += L"�л���ָ���߳�";
        desc += L"eg:";
        desc += L"tc 1028  �л����߳�tidΪ1028���߳�";
        desc += L"tc 1     �л������Ϊ1���߳�";
    }
    else if (wstrParam == L"ts")
    {
        desc += L"��ӡ��ǰ���е��߳���Ϣ";
    }
    else if (wstrParam == L"lm")
    {
        desc += L"��ӡ��ǰ���ص����е�ģ����Ϣ";
    }
    else if (wstrParam == L"cls")
    {
        desc += L"�����ǰ��Ļ�ϵ���Ϣ";
    }
    else if (wstrParam == L"u")
    {
        desc += L"�����ָ���ĵ�ַ����api";
        desc += L"eg:";
        desc += L"u 0x1028ffee              ��0x1028ffeeλ�÷����";
        desc += L"u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�÷����";
    }
    else if (wstrParam == L"ub")
    {
        desc += L"���Ϸ����ָ���ĵ�ַ����api";
        desc += L"eg:";
        desc += L"u 0x1028ffee              ��0x1028ffeeλ�����Ϸ����";
        desc += L"u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�����Ϸ����";
    }
    else if (wstrParam == L"uf")
    {
        desc += L"�����ָ���ĺ���";
        desc += L"eg:";
        desc += L"uf kernel32!createfilew  ����ຯ��kernel32!createfilew";
        desc += L"uf 0x1234abcd            �����λ��0x1234abcd�ĺ�������";
    }
    else if (wstrParam == L"g")
    {
        desc += L"�������е��Խ���";
    }
    else if (wstrParam == L"gu")
    {
        desc += L"���е����÷���";
    }
    else if (wstrParam == L"kv")
    {
        desc += L"��ӡ����ջ�Ͳ�����Ϣ";
    }
    else if (wstrParam == L"db")
    {
        desc += L"���ֽڴ�ӡָ���ڴ��ַ������";
        desc += L"eg:";
        desc += L"db 0x1234abcd �ӵ�ַ0x1234abcd���ֽڴ�ӡ����";
        desc += L"db [csp]      ��csp�Ĵ���ָ��ĵ�ַ���ֽڴ�ӡ����";
    }
    else if (wstrParam == L"dd")
    {
        desc += L"��32���δ�ӡָ���ڴ��ַ������";
        desc += L"eg:";
        desc += L"dd 0x1234abcd �ӵ�ַ0x1234abcd��32λ���ʹ�ӡ����";
        desc += L"dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ��32λ���ʹ�ӡ����";
    }
    else if (wstrParam == L"du")
    {
        desc += L"�����ַ�����ӡָ���ڴ��ַ������";
        desc += L"eg:";
        desc += L"dd 0x1234abcd �ӵ�ַ0x1234abcd�����ַ���ӡ����";
        desc += L"dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ�����ַ���ӡ����";
    }
    else if (wstrParam == L"r")
    {
        desc += L"��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ";
        desc += L"eg:";
        desc += L"r csp=0x11223344 ��csp�Ĵ���ֵ����Ϊ0x11223344";
        desc += L"r                չʾ��ǰ���еļĴ���ֵ";
    }
    else if (wstrParam == L"sc")
    {
        desc += L"����ָ���Ľű�";
        desc += L"eg:";
        desc += L"sc print  ���нű�Ŀ¼������Ϊprint�Ľű�";
    }
    else
    {
        desc += L"û�и������˵��";
    }
    */
    //return DbgCmdResult(em_dbgstat_succ, desc);
    return CmdReplyResult();
}

void CDbggerProxy::InitHelpEngine()
{
    OnCmdHlprRegister();
}