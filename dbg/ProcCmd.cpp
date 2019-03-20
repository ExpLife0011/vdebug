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
    //�л���ָ���߳�
    else if (cmd == "tc")
    {
        return OnCmdTc(cmdParam, pParam);
    }
    //չʾָ���߳�
    else if (cmd == "ts")
    {
        return OnCmdTs(cmdParam, pParam);
    }
    //չʾģ����Ϣ
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
    //ִ�е����÷���
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
    reply.mShow = mstring("��֧�ֵ�����:") + cmd + "\n";
    return reply;
}

CtrlReply CProcCmd::OnCmdBp(const mstring &param, const CmdUserParam *pParam)
{
    CtrlReply result;
    mstring str(param);
    str.trim();

    if (str.empty())
    {
        result.mShow = "bp��������\n";
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
            result.mShow = "���öϵ�ɹ�\n";
        } else {
            result.mShow = GetBreakPointMgr()->GetLastErr() + "\n";
        }
    } else {
        result.mShow = FormatA("δʶ��ĵ�ַ %hs\n", str.c_str());
    }
    return result;
}

CtrlReply CProcCmd::OnCmdBl(const mstring &param, const CmdUserParam *pParam)
{
    vector<BreakPointInfo> bpSet = GetBreakPointMgr()->GetBpSet();

    CtrlReply result;
    if (bpSet.empty())
    {
        result.mShow = "��δ�����κζϵ�\n";
        return result;
    }

    PrintFormater pf;
    pf << "�ϵ����" << "�ϵ�״̬" << "�ϵ��ַ" << "����λ��" << line_end;
    for (vector<BreakPointInfo>::const_iterator it = bpSet.begin() ; it != bpSet.end() ; it++)
    {
        string stat;
        switch (it->mBpStat) {
            case em_bp_enable:
                stat = "����";
                break;;
            case em_bp_disable:
                stat = "����";
                break;
            case em_bp_uneffect:
                stat = "δ��Ч";
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
        tmp.mShow = "��������жϵ�";
        return tmp;
    }

    if (!IsNumber(str))
    {
        tmp.mStatus = DBG_CMD_SYNTAX_ERR;
        tmp.mShow = "bc �﷨����";
        return tmp;
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    if (GetBreakPointMgr()->DeleteBpByIndex((int)dwSerial))
    {
        tmp.mShow = FormatA("�����%02x�Ŷϵ�", dwSerial);
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
        result.mShow = "bu ��Ҫ�ϵ����\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mShow = "bu ��ʽ����";
        return result;
    }

    if (GetBreakPointMgr()->DisableBpByIndex((int)index))
    {
        result.mShow = FormatA("���� % �Ŷϵ�ɹ�", index);
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
        result.mShow = "be ��Ҫ�ϵ����\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mShow = "be ��ʽ����";
        return result;
    }

    if (GetBreakPointMgr()->EnableBpByIndex((int)index))
    {
        result.mShow = FormatA("���� % �Ŷϵ�ɹ�", index);
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
        result.mShow = "�﷨����";
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
        result.mShow = FormatA("��ȡ%hs��ַʧ��", strAddr.c_str());
        return result;
    }

    if (!mProcDbgger->DisassWithSize(dwAddr, dwDisasmSize, result))
    {
        result.mShow = FormatA("������ַ 0x%08x ʧ��\n", (DWORD)dwAddr);
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
        reply.mShow = "��ȡ������ַ����ʧ��";
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
        reply.mShow = "ub�﷨����";
        return reply;
    }

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;

    if (!mProcDbgger->DisassWithAddr(dwStartAddr, dwEndAddr, reply))
    {
        reply.mShow = FormatA("������ַ 0x%08x ʧ��\n", (DWORD)dwStartAddr);
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
        reply.mShow = FormatA("������ַ 0x%08x ʧ��\n", (DWORD)dwAddr);
    }
    return reply;
}

CtrlReply CProcCmd::OnCmdGo(const mstring &param, const CmdUserParam *pParam)
{
    mProcDbgger->Run();
    CtrlReply result;
    result.mShow = "���̼�������\n";
    return result;
}

CtrlReply CProcCmd::OnCmdGu(const mstring &param, const CmdUserParam *pParam)
{
    mProcDbgger->RunExitProc();

    CtrlReply reply;
    reply.mShow = "ִ��gu�ɹ�";
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
        result.mShow += "VDebug֧�ֵ�����ܸ�Ҫ\n";
        result.mShow += "bp  ��ָ�����ڴ��ַ���ߺ������öϵ�\n";
        result.mShow += "bl  ��ӡ�Ѵ��ڵĶϵ���Ϣ\n";
        result.mShow += "bc  ���ָ���Ķϵ�\n";
        result.mShow += "tc  �л���ָ�����߳�\n";
        result.mShow += "ts  ��ӡ���е��߳���Ϣ\n";
        result.mShow += "lm  ��ӡ���е�ģ����Ϣ\n";
        result.mShow += "cls ���ҳ����Ϣ\n";
        result.mShow += "u   �����ָ���ĵ�ַ����api\n";
        result.mShow += "ub  ���Ϸ����ָ���ĵ�ַ����api\n";
        result.mShow += "uf  �����ָ���ĺ���\n";
        result.mShow += "g   �������е��Խ���\n";
        result.mShow += "gu  ���е����÷���\n";
        result.mShow += "kv  ��ӡ����ջ�Ͳ�����Ϣ\n";
        result.mShow += "db  ���ֽڴ�ӡָ���ڴ��ַ������\n";
        result.mShow += "dd  ��32���δ�ӡָ���ڴ��ַ������\n";
        result.mShow += "du  �����ַ�����ӡָ���ڴ��ַ������\n";
        result.mShow += "r   ��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ\n";
        result.mShow += "pf  �����������\n";
        result.mShow += "sc  ����ָ���Ľű�\n";
    } else if (strParam == "bp")
    {
        result.mShow += "��ָ�����ڴ��ַ���ߺ������öϵ�\n";
        result.mShow += "eg:\n";
        result.mShow += "bp 0x1122aabb             ���ڴ�0x1122aabbλ���¶ϵ�\n";
        result.mShow += "bp kernelbase!createfilew ��kernelbaseģ�鵼����createfilew�������¶ϵ�\n";
    } else if (strParam == "bl")
    {
        result.mShow += "��ӡ�Ѵ��ڵĶϵ���Ϣ\n";
    }
    else if (strParam == "bc")
    {
        result.mShow += "���ָ���Ķϵ�\n";
        result.mShow += "eg:\n";
        result.mShow += "bc *  �����ǰ���еĶϵ�\n";
        result.mShow += "bc 1  ������Ϊ1�Ķϵ�\n";
    }
    else if (strParam == "tc")
    {
        result.mShow += "�л���ָ���߳�\n";
        result.mShow += "eg:\n";
        result.mShow += "tc 1028  �л����߳�tidΪ1028���߳�\n";
        result.mShow += "tc 1     �л������Ϊ1���߳�\n";
    }
    else if (strParam == "ts")
    {
        result.mShow += "��ӡ��ǰ���е��߳���Ϣ\n";
    }
    else if (strParam == "lm")
    {
        result.mShow += "��ӡ��ǰ���ص����е�ģ����Ϣ\n";
    }
    else if (strParam == "cls")
    {
        result.mShow += "�����ǰ��Ļ�ϵ���Ϣ\n";
    }
    else if (strParam == "u")
    {
        result.mShow += "�����ָ���ĵ�ַ����api\n";
        result.mShow += "eg:\n";
        result.mShow += "u 0x1028ffee              ��0x1028ffeeλ�÷����\n";
        result.mShow += "u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�÷����\n";
    }
    else if (strParam == "ub")
    {
        result.mShow += "���Ϸ����ָ���ĵ�ַ����api\n";
        result.mShow += "eg:\n";
        result.mShow += "u 0x1028ffee              ��0x1028ffeeλ�����Ϸ����\n";
        result.mShow += "u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�����Ϸ����\n";
    }
    else if (strParam == "uf")
    {
        result.mShow += "�����ָ���ĺ���\n";
        result.mShow += "eg:\n";
        result.mShow += "uf kernel32!createfilew  ����ຯ��kernel32!createfilew\n";
        result.mShow += "uf 0x1234abcd            �����λ��0x1234abcd�ĺ�������\n";
    }
    else if (strParam == "g")
    {
        result.mShow += "�������е��Խ���\n";
    }
    else if (strParam == "gu")
    {
        result.mShow += "���е����÷���\n";
    }
    else if (strParam == "kv")
    {
        result.mShow += "��ӡ����ջ�Ͳ�����Ϣ\n";
    }
    else if (strParam == "db")
    {
        result.mShow += "���ֽڴ�ӡָ���ڴ��ַ������\n";
        result.mShow += "eg:\n";
        result.mShow += "db 0x1234abcd �ӵ�ַ0x1234abcd���ֽڴ�ӡ����\n";
        result.mShow += "db [csp]      ��csp�Ĵ���ָ��ĵ�ַ���ֽڴ�ӡ����\n";
    }
    else if (strParam == "dd")
    {
        result.mShow += "��32���δ�ӡָ���ڴ��ַ������\n";
        result.mShow += "eg:\n";
        result.mShow += "dd 0x1234abcd �ӵ�ַ0x1234abcd��32λ���ʹ�ӡ����\n";
        result.mShow += "dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ��32λ���ʹ�ӡ����\n";
    }
    else if (strParam == "du")
    {
        result.mShow += "�����ַ�����ӡָ���ڴ��ַ������\n";
        result.mShow += "eg:\n";
        result.mShow += "dd 0x1234abcd �ӵ�ַ0x1234abcd�����ַ���ӡ����\n";
        result.mShow += "dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ�����ַ���ӡ����\n";
    }
    else if (strParam == "r")
    {
        result.mShow += "��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ\n";
        result.mShow += "eg:\n";
        result.mShow += "r csp=0x11223344 ��csp�Ĵ���ֵ����Ϊ0x11223344\n";
        result.mShow += "r                չʾ��ǰ���еļĴ���ֵ\n";
    }
    else if (strParam == "sc")
    {
        result.mShow += "����ָ���Ľű�\n";
        result.mShow += "eg:\n";
        result.mShow += "sc print  ���нű�Ŀ¼������Ϊprint�Ľű�\n";
    }
    else
    {
        result.mShow += "û�и������˵��\n";
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
            result.mShow = FormatA("��ȡ�ڴ�λ�� 0x%08x ����ʧ��\n", dwAddr);
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
        result.mShow = FormatA("���� %hs ���ʽʧ��", cmdParam.c_str());
        return result;
    }

    DWORD dwDataSize = 64;
    PrintFormater pf;
    pf << "���ݵ�ַ" << "��������" << space << space << space << line_end;

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
        result.mShow = "�﷨����\n";
    } else {
        CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
        ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mShow = "û�ж�����Ч���ַ�������";
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
        result.mShow = "�﷨����\n";
    } else {
        CMemoryProc mhlpr(mProcDbgger->GetDbgProc());
        mstring strData = mhlpr.MemoryReadStrGbk(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mShow = "û�ж�����Ч���ַ�������";
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
    pf << "�ڴ��ַ" << "���ص�ַ" << "�����б�" << space << space << space << "��������" << line_end;
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
        reply.mShow = "δ�ҵ���Ҫ�л����߳�";
    } else {
        mProcDbgger->mCurrentThread = mProcDbgger->m_vThreadMap[(int)dwSerial];
        reply.mShow = FormatA("�л���0x%04x���̳߳ɹ�����ǰ�߳�0x%x\n", (DWORD)dwSerial, mProcDbgger->mCurrentThread.m_dwThreadId);
    }
    return reply;
}

CtrlReply CProcCmd::OnCmdLm(const mstring &param, const CmdUserParam *pParam)
{
    list<DbgModuleInfo> moduleSet = mProcDbgger->GetModuleInfo();

    PrintFormater pf;
    pf << "��ʼ��ַ" << "������ַ" << "�汾��Ϣ" << "ģ��·��" << line_end;
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
            result.mShow = "û�к�����Ϣ";
        } else {
            result.mShow = CDescPrinter::GetInst()->GetProcStrByName("", fun);
        }
    } else {
        mstring proc = CDbgCommon::GetProcSymFromAddr(ctx.cip, module.m_strDllName, module.m_dwBaseOfImage);
        if (proc.empty()) {
            result.mShow = FormatA("δʶ��ĺ�����ַ 0x%08x\n", ctx.cip);
        } else {
            if (mstring::npos != proc.find("+0x"))
            {
                result.mShow = FormatA("��ǰcipδ���ں�����ʼλ�� %hs 0x%08x\n", proc.c_str(), ctx.cip);
            } else {
                CMemoryProc reader(CProcDbgger::GetInstance()->GetDbgProc());
                CDescPrinter::GetInst()->SetMemoryReader(&reader);
                mstring procStr = CDescPrinter::GetInst()->GetProcStrByName(module.m_strDllName, proc, (LPVOID)((const char *)ctx.csp + 4));

                if (procStr.empty())
                {
                    result.mShow = FormatA("�������в�����%hsģ��%hs����Ϣ\n", module.m_strDllName.c_str(), proc.c_str());
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
    pf << "���" <<"�߳�id" << "����ʱ��" << "״̬" << "����λ��" << line_end;
    int index = 0;
    for (vector<DbgProcThreadInfo>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++, index++)
    {
        string a = FormatA("0x%04x", index);
        string b = FormatA("0x%08x(%d)", it->m_dwThreadId, it->m_dwThreadId);
        string c = it->mStartTimeStr;
        string d = "��������";

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
