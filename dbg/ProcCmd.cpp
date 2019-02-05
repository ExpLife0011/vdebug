#include <DbgCtrl/DbgCtrlCom.h>
#include "ProcCmd.h"
#include "Script.h"
#include "memory.h"
#include "BreakPoint.h"

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

CmdReplyResult CProcCmd::OnCommand(const mstring &cmd, const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
    if (cmd == "bp")
    {
        return OnCmdBp(cmdParam, mode, pParam);
    }
    else if (cmd == "bl")
    {
        return OnCmdBl(cmdParam, mode, pParam);
    }
    else if (cmd == "bk")
    {
        DebugBreakProcess(mProcDbgger->GetDbgProc());
    }
    else if (cmd == "bc")
    {
        return OnCmdBc(cmdParam, mode, pParam);
    }
    else if (cmd == "bu")
    {
        return OnCmdBu(cmdParam, mode, pParam);
    }
    else if (cmd == "be")
    {
        return OnCmdBe(cmdParam, mode, pParam);
    }
    //�л���ָ���߳�
    else if (cmd == "tc")
    {
        return OnCmdTc(cmdParam, mode, pParam);
    }
    //չʾָ���߳�
    else if (cmd == "ts")
    {
        return OnCmdTs(cmdParam, mode, pParam);
    }
    //չʾģ����Ϣ
    else if (cmd == "lm")
    {
        return OnCmdLm(cmdParam, mode, pParam);
    }
    else if (cmd == "u")
    {
        return OnCmdDisass(cmdParam, mode, pParam);
    }
    else if (cmd == "ub")
    {
        return OnCmdUb(cmdParam, mode, pParam);
    }
    else if (cmd == "uf")
    {
        return OnCmdUf(cmdParam, mode, pParam);
    }
    else if (cmd == "g")
    {
        return OnCmdGo(cmdParam, mode, pParam);
    }
    //ִ�е����÷���
    else if (cmd == "gu")
    {
        return OnCmdGu(cmdParam, mode, pParam);
    }
    else if (cmd == "kv")
    {
        return OnCmdKv(cmdParam, mode, pParam);
    }
    else if (cmd == "db")
    {
        return OnCmdDb(cmdParam, mode, pParam);
    }
    else if (cmd == "dd")
    {
        return OnCmdDd(cmdParam, mode, pParam);
    }
    else if (cmd == "du")
    {
        return OnCmdDu(cmdParam, mode, pParam);
    }
    else if (cmd == "r")
    {
        return OnCmdReg(cmdParam, mode, pParam);
    }
    else if (cmd == "sc")
    {
        return OnCmdScript(cmdParam, mode, pParam);
    }
    else if (cmd == "help" || cmd == "h")
    {
        return OnCmdHelp(cmdParam, mode, pParam);
    }

    return CmdReplyResult(0, mstring("��֧�ֵ�����:") + cmd + "\n", "");
}

CmdReplyResult CProcCmd::OnCmdBp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    CmdReplyResult result;
    mstring str(param);
    str.trim();

    if (str.empty())
    {
        result.mCmdShow = "bp��������\n";
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
            result.mCmdShow = "���öϵ�ɹ�\n";
        } else {
            result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
        }
    } else {
        result.mCmdShow = FormatA("δʶ��ĵ�ַ %hs\n", str.c_str());
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdBl(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    vector<BreakPointInfo> bpSet = GetBreakPointMgr()->GetBpSet();

    CmdReplyResult result;
    if (bpSet.empty())
    {
        result.mCmdShow = "��δ�����κζϵ�\n";
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
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdBc(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CmdReplyResult tmp;
    mstring str(cmdParam);
    str.makelower();
    if (str == "*")
    {
        GetBreakPointMgr()->DeleteAllBp();
        tmp.mCmdShow = "��������жϵ�";
        return tmp;
    }

    if (!IsNumber(str))
    {
        tmp.mCmdCode = DBG_CMD_SYNTAX_ERR;
        tmp.mCmdShow = "bc �﷨����";
        return tmp;
    }

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    if (GetBreakPointMgr()->DeleteBpByIndex((int)dwSerial))
    {
        tmp.mCmdShow = FormatA("�����%02x�Ŷϵ�", dwSerial);
    } else {
        tmp.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }

    return tmp;
}

CmdReplyResult CProcCmd::OnCmdBu(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
    CmdReplyResult result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mCmdShow = "bu ��Ҫ�ϵ����\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mCmdShow = "bu ��ʽ����";
        return result;
    }

    if (GetBreakPointMgr()->DisableBpByIndex((int)index))
    {
        result.mCmdShow = FormatA("���� % �Ŷϵ�ɹ�", index);
    } else {
        result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdBe(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam) {
    CmdReplyResult result;
    mstring str(cmdParam);
    str.makelower();

    if (str.empty())
    {
        result.mCmdShow = "be ��Ҫ�ϵ����\n";
        return result;
    }

    DWORD64 index = 0;
    if (!GetNumFromStr(str, index))
    {
        result.mCmdShow = "be ��ʽ����";
        return result;
    }

    if (GetBreakPointMgr()->EnableBpByIndex((int)index))
    {
        result.mCmdShow = FormatA("���� % �Ŷϵ�ɹ�", index);
    } else {
        result.mCmdShow = GetBreakPointMgr()->GetLastErr() + "\n";
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdDisass(const mstring &wstrCmdParam, DWORD mode, const CmdUserParam *pParam)
{
    mstring wstr(wstrCmdParam);
    wstr.makelower();
    wstr.trim();

    DWORD64 dwDisasmSize = 0;
    mstring strAddr;

    CmdReplyResult result;
    dwDisasmSize = GetSizeAndParam(wstr, strAddr);
    if (!dwDisasmSize)
    {
        result.mCmdCode = DBG_CMD_SYNTAX_ERR;
        result.mCmdShow = "�﷨����";
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
        result.mCmdCode = DBG_CMD_READMEM_ERR;
        result.mCmdShow = FormatA("��ȡ%hs��ַʧ��", strAddr.c_str());
        return result;
    }
    /*
    //CSyntaxDescHlpr hlpr;
    DisassWithSize(dwAddr, dwDisasmSize, hlpr);
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return result;
}

CmdReplyResult CProcCmd::OnCmdUb(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CmdReplyResult reply;
    if (!dwDisasmSize)
    {
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
        reply.mCmdShow = "��ȡ������ַ����ʧ��";
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
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
        reply.mCmdShow = "ub�﷨����";
        return reply;
    }

    DWORD64 dwEndAddr = dwAddr;
    dwAddr -= dwDisasmSize;
    DWORD64 dwStartAddr = dwAddr;
    ////CSyntaxDescHlpr hlpr;
    //mstring data;
    //DisassWithAddr(dwStartAddr, dwEndAddr, data);
    //return mstring(em_dbgstat_succ, hlpr.GetResult());
    return reply;
}

CmdReplyResult CProcCmd::OnCmdUf(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.makelower();
    str.trim();

    DWORD64 dwDisasmSize = mProcDbgger->ms_dwDefDisasmSize;
    mstring strAddr;

    dwDisasmSize = GetSizeAndParam(str, strAddr);
    CmdReplyResult reply;
    if (!dwDisasmSize)
    {
        reply.mCmdCode = DBG_CMD_UNKNOW_ERR;
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
        reply.mCmdCode = DBG_CMD_SYNTAX_ERR;
        return reply;
    }

    /*
    //CSyntaxDescHlpr hlpr;
    DisassUntilRet(dwAddr, hlpr);
    return mstring(em_dbgstat_succ, hlpr.GetResult());
    */
    return reply;
}

CmdReplyResult CProcCmd::OnCmdGo(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mProcDbgger->Run();
    CmdReplyResult result;
    result.mCmdShow = "���̼�������\n";
    return result;
}

CmdReplyResult CProcCmd::OnCmdGu(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mProcDbgger->RunExitProc();

    CmdReplyResult reply;
    reply.mCmdShow = "ִ��gu�ɹ�";
    return reply;
}

CmdReplyResult CProcCmd::OnCmdReg(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    RegisterContent ctx;
    ctx.mContext = mProcDbgger->GetCurrentContext();
    ctx.mCipStr = mProcDbgger->GetSymFromAddr((void *)ctx.mContext.cip).c_str();

    CmdReplyResult result;
    result.mCmdResult = EncodeCmdRegister(ctx);

    PrintFormater pf;
    pf << FormatA("eax=0x%08x", ctx.mContext.cax) << FormatA("ebx=0x%08x", ctx.mContext.cbx);
    pf << FormatA("ecx=0x%08x", ctx.mContext.ccx) << FormatA("edx=0x%08x", ctx.mContext.cdx) << line_end;

    pf << FormatA("esi=0x%08x", ctx.mContext.csi) << FormatA("edi=0x%08x", ctx.mContext.cdi);
    pf << FormatA("eip=0x%08x", ctx.mContext.cip) << FormatA("esp=0x%08x", ctx.mContext.csp) << line_end;

    pf << FormatA("ebp=0x%08x", ctx.mContext.cbp) << space << space << space << line_end;
    result.mCmdShow = pf.GetResult();
    pf.Reset();

    pf << ctx.mCipStr << line_end;
    result.mCmdShow += pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdScript(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdHelp(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring strParam(param);
    strParam.makelower();
    strParam.trim();

    CmdReplyResult result;
    if (strParam.empty())
    {
        result.mCmdShow += "VDebug֧�ֵ�����ܸ�Ҫ\n";
        result.mCmdShow += "bp  ��ָ�����ڴ��ַ���ߺ������öϵ�\n";
        result.mCmdShow += "bl  ��ӡ�Ѵ��ڵĶϵ���Ϣ\n";
        result.mCmdShow += "bc  ���ָ���Ķϵ�\n";
        result.mCmdShow += "tc  �л���ָ�����߳�\n";
        result.mCmdShow += "ts  ��ӡ���е��߳���Ϣ\n";
        result.mCmdShow += "lm  ��ӡ���е�ģ����Ϣ\n";
        result.mCmdShow += "cs  ���ҳ����Ϣ\n";
        result.mCmdShow += "u   �����ָ���ĵ�ַ����api\n";
        result.mCmdShow += "ub  ���Ϸ����ָ���ĵ�ַ����api\n";
        result.mCmdShow += "uf  �����ָ���ĺ���\n";
        result.mCmdShow += "g   �������е��Խ���\n";
        result.mCmdShow += "gu  ���е����÷���\n";
        result.mCmdShow += "kv  ��ӡ����ջ�Ͳ�����Ϣ\n";
        result.mCmdShow += "db  ���ֽڴ�ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "dd  ��32���δ�ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "du  �����ַ�����ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "r   ��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ\n";
        result.mCmdShow += "sc  ����ָ���Ľű�\n";
    } else if (strParam == "bp")
    {
        result.mCmdShow += "��ָ�����ڴ��ַ���ߺ������öϵ�\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "bp 0x1122aabb             ���ڴ�0x1122aabbλ���¶ϵ�\n";
        result.mCmdShow += "bp kernelbase!createfilew ��kernelbaseģ�鵼����createfilew�������¶ϵ�\n";
    } else if (strParam == "bl")
    {
        result.mCmdShow += "��ӡ�Ѵ��ڵĶϵ���Ϣ\n";
    }
    else if (strParam == "bc")
    {
        result.mCmdShow += "���ָ���Ķϵ�\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "bc *  �����ǰ���еĶϵ�\n";
        result.mCmdShow += "bc 1  ������Ϊ1�Ķϵ�\n";
    }
    else if (strParam == "tc")
    {
        result.mCmdShow += "�л���ָ���߳�\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "tc 1028  �л����߳�tidΪ1028���߳�\n";
        result.mCmdShow += "tc 1     �л������Ϊ1���߳�\n";
    }
    else if (strParam == "ts")
    {
        result.mCmdShow += "��ӡ��ǰ���е��߳���Ϣ\n";
    }
    else if (strParam == "lm")
    {
        result.mCmdShow += "��ӡ��ǰ���ص����е�ģ����Ϣ\n";
    }
    else if (strParam == "cls")
    {
        result.mCmdShow += "�����ǰ��Ļ�ϵ���Ϣ\n";
    }
    else if (strParam == "u")
    {
        result.mCmdShow += "�����ָ���ĵ�ַ����api\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "u 0x1028ffee              ��0x1028ffeeλ�÷����\n";
        result.mCmdShow += "u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�÷����\n";
    }
    else if (strParam == "ub")
    {
        result.mCmdShow += "���Ϸ����ָ���ĵ�ַ����api\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "u 0x1028ffee              ��0x1028ffeeλ�����Ϸ����\n";
        result.mCmdShow += "u kernelbase!createfilew  ��kernelbase!createfilew��ʼ��λ�����Ϸ����\n";
    }
    else if (strParam == "uf")
    {
        result.mCmdShow += "�����ָ���ĺ���\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "uf kernel32!createfilew  ����ຯ��kernel32!createfilew\n";
        result.mCmdShow += "uf 0x1234abcd            �����λ��0x1234abcd�ĺ�������\n";
    }
    else if (strParam == "g")
    {
        result.mCmdShow += "�������е��Խ���\n";
    }
    else if (strParam == "gu")
    {
        result.mCmdShow += "���е����÷���\n";
    }
    else if (strParam == "kv")
    {
        result.mCmdShow += "��ӡ����ջ�Ͳ�����Ϣ\n";
    }
    else if (strParam == "db")
    {
        result.mCmdShow += "���ֽڴ�ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "db 0x1234abcd �ӵ�ַ0x1234abcd���ֽڴ�ӡ����\n";
        result.mCmdShow += "db [csp]      ��csp�Ĵ���ָ��ĵ�ַ���ֽڴ�ӡ����\n";
    }
    else if (strParam == "dd")
    {
        result.mCmdShow += "��32���δ�ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "dd 0x1234abcd �ӵ�ַ0x1234abcd��32λ���ʹ�ӡ����\n";
        result.mCmdShow += "dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ��32λ���ʹ�ӡ����\n";
    }
    else if (strParam == "du")
    {
        result.mCmdShow += "�����ַ�����ӡָ���ڴ��ַ������\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "dd 0x1234abcd �ӵ�ַ0x1234abcd�����ַ���ӡ����\n";
        result.mCmdShow += "dd [csp]      ��csp�Ĵ���ָ��ĵ�ַ�����ַ���ӡ����\n";
    }
    else if (strParam == "r")
    {
        result.mCmdShow += "��ӡ�����޸ĵ�ǰ�̵߳ļĴ���ֵ\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "r csp=0x11223344 ��csp�Ĵ���ֵ����Ϊ0x11223344\n";
        result.mCmdShow += "r                չʾ��ǰ���еļĴ���ֵ\n";
    }
    else if (strParam == "sc")
    {
        result.mCmdShow += "����ָ���Ľű�\n";
        result.mCmdShow += "eg:\n";
        result.mCmdShow += "sc print  ���нű�Ŀ¼������Ϊprint�Ľű�\n";
    }
    else
    {
        result.mCmdShow += "û�и������˵��\n";
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdDb(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
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

    CmdReplyResult result;
    CMemoryOperator mhlpr(mProcDbgger->GetDbgProc());
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
            result.mCmdShow = FormatA("��ȡ�ڴ�λ�� 0x%08x ����ʧ��\n", dwAddr);
            break;
        }

        result.mCmdShow += FormatA("%08x  ", dwAddr);
        int j = 0;
        for (j = 0 ; j < 16 ; j++)
        {
            if (j < (int)dwRead)
            {
                result.mCmdShow += FormatA("%02x ", (BYTE)szData[j]);
            }
            else
            {
                result.mCmdShow += "   ";
            }
        }

        result.mCmdShow += " ";
        result.mCmdShow += mProcDbgger->GetPrintStr(szData, dwRead);
        result.mCmdShow += "\n";
        dwAddr += 16;
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdDd(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    DWORD64 dwAddr = script.Compile(cmdParam);
    if (!dwAddr)
    {
        return CmdReplyResult();
    }

    DWORD dwDataSize = 64;
    /*
    //CSyntaxDescHlpr desc;
    CMemoryOperator mhlpr(GetInstance()->GetDbgProc());
    desc.FormatDesc(L"���ݵ�ַ  ", COLOUR_MSG);
    desc.FormatDesc(L"��������", COLOUR_MSG);
    desc.NextLine();
    for (int i = 0 ; i < (int)dwDataSize ; i += 16)
    {
        char szData[16] = {0};
        DWORD dwReadSize = 0;
        mhlpr.MemoryReadSafe(dwAddr, szData, sizeof(szData), &dwReadSize);
        if (!dwReadSize)
        {
            break;
        }
        desc.FormatDesc(FormatW(L"%08x  ", dwAddr), COLOUR_ADDR);
        for (int j = 0 ; j < (int)dwReadSize / 4 ; j += 1)
        {
            desc.FormatDesc(FormatW(L"%08x ", *((DWORD *)szData + j)), COLOUR_DATA);
        }
        desc.NextLine();
        dwAddr += 16;
    }
    return mstring(em_dbgstat_succ, desc.GetResult());
    */
    return CmdReplyResult();
}

CmdReplyResult CProcCmd::OnCmdDu(const mstring &strCmdParam, DWORD mode, const CmdUserParam *pParam)
{
    CScriptEngine script;
    script.SetContext(mProcDbgger->GetCurrentContext(), CProcDbgger::ReadDbgProcMemory, CProcDbgger::WriteDbgProcMemory);

    CmdReplyResult result;
    DWORD64 dwAddr = script.Compile(strCmdParam);
    if (!dwAddr)
    {
        result.mCmdShow = "�﷨����\n";
    } else {
        CMemoryOperator mhlpr(mProcDbgger->GetDbgProc());
        ustring strData = mhlpr.MemoryReadStrUnicode(dwAddr, MAX_PATH);

        if (strData.empty())
        {
            result.mCmdShow = "û�ж�����Ч���ַ�������";
        } else {
            result.mCmdShow = WtoA(strData) + "\n";
        }
    }
    return result;
}

CmdReplyResult CProcCmd::OnCmdKv(const mstring &cmdParam, DWORD mode, const CmdUserParam *pParam)
{
    list<STACKFRAME64> vStack = mProcDbgger->GetStackFrame(cmdParam);
    CmdReplyResult result;
    if (vStack.empty())
    {
        return result;
    }

    CallStackData callSet;
    CallStackSingle single;
    PrintFormater pf;
    pf << "�ڴ��ַ" << "���ص�ַ" << "�����б�" << space << space << space <<"��������" << line_end;
    for (list<STACKFRAME64>::const_iterator it = vStack.begin() ; it != vStack.end() ; it++)
    {
        single.mAddr = FormatA("%08x", it->AddrPC.Offset);
        single.mReturn = FormatA("%08x", it->AddrReturn);
        single.mParam0 = FormatA("%08x", it->Params[0]);
        single.mParam1 = FormatA("%08x", it->Params[1]);
        single.mParam2 = FormatA("%08x", it->Params[2]);
        single.mParam3 = FormatA("%08x", it->Params[3]);
        single.mFunction = FormatA("%hs", mProcDbgger->GetSymFromAddr((void *)it->AddrPC.Offset).c_str());
        callSet.mCallStack.push_back(single);

        pf << single.mAddr << single.mReturn << single.mParam0 << single.mParam1 << single.mParam2 << single.mParam3 << single.mFunction << line_end;
    }
    result.mCmdCode = 0;
    result.mResultMode = mode;
    result.mCmdResult = EncodeCmdCallStack(callSet);
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdTc(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    mstring str(param);
    str.trim();

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    CmdReplyResult reply;

    DWORD dw = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = mProcDbgger->m_vThreadMap.begin() ; it != mProcDbgger->m_vThreadMap.end() ; it++, dw++)
    {
        if (dwSerial == dw || dwSerial == it->m_dwThreadId)
        {
            mProcDbgger->m_dwCurrentThreadId = it->m_dwThreadId;
            reply.mCmdShow = FormatA("�л���%d���̳߳ɹ�����ǰ�߳�%x", dw, it->m_dwThreadId);
            return reply;
        }
    }
    reply.mCmdShow = "δ�ҵ���Ҫ�л����߳�";
    return reply;
}

CmdReplyResult CProcCmd::OnCmdLm(const mstring &param, DWORD mode, const CmdUserParam *pParam)
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

    CmdReplyResult result;
    result.mCmdShow = pf.GetResult();
    return result;
}

CmdReplyResult CProcCmd::OnCmdTs(const mstring &param, DWORD mode, const CmdUserParam *pParam)
{
    list<ThreadInformation> threadSet = mProcDbgger->GetCurrentThreadSet();

    PrintFormater pf;
    pf << "�߳�ID" << "����ʱ��" << "״̬" << "����λ��" << line_end;
    for (list<ThreadInformation>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++)
    {
        string a = FormatA("0x%08x", it->m_dwThreadId);

        SYSTEMTIME time = {0};
        FileTimeToSystemTime(&(it->m_vCreateTime), &time);
        string b = FormatA(
            "%04d-%02d-%02d %02d:%02d:%02d %03d ",
            time.wYear,
            time.wMonth,
            time.wDay,
            time.wHour,
            time.wMinute,
            time.wSecond,
            time.wMilliseconds
            );
        string c = "��������";
        string d = FormatA("0x%08x %hs", it->m_dwStartAddr, mProcDbgger->GetSymFromAddr(it->m_dwStartAddr).c_str());
        pf << a << b << c << d << line_end;
    }

    CmdReplyResult result;
    result.mCmdShow = pf.GetResult();
    return result;
    /*
    //CSyntaxDescHlpr hlpr;
    hlpr.FormatDesc(L"��� ");
    hlpr.FormatDesc(L"�߳�ID", COLOUR_MSG, 12);
    hlpr.FormatDesc(L"����ʱ��", COLOUR_MSG, 25);
    hlpr.FormatDesc(L"״̬", COLOUR_MSG, 10);
    hlpr.FormatDesc(L"����λ��");

    list<ThreadInformation> vThreads;
    GetThreadInformation(GetInstance()->GetDebugProcData()->dwProcessId, vThreads);
    int iIndex = 0;
    for (list<DbgProcThreadInfo>::const_iterator it = m_vThreadMap.begin() ; it != m_vThreadMap.end() ; it++, iIndex++)
    {
        for (list<ThreadInformation>::const_iterator itSingle = vThreads.begin() ; itSingle != vThreads.end() ; itSingle++)
        {
            if (it->m_dwThreadId == itSingle->m_dwThreadId)
            {
                hlpr.NextLine();
                hlpr.FormatDesc(FormatW(L"%02x", iIndex), COLOUR_MSG, 5);
                hlpr.FormatDesc(FormatW(L"%x:%d", it->m_dwThreadId, it->m_dwThreadId), COLOUR_MSG, 12);

                SYSTEMTIME time = {0};
                FileTimeToSystemTime(&(itSingle->m_vCreateTime), &time);
                hlpr.FormatDesc(
                    FormatW(
                    L"%04d-%02d-%02d %02d:%02d:%02d %03d ",
                    time.wYear,
                    time.wMonth,
                    time.wDay,
                    time.wHour,
                    time.wMinute,
                    time.wSecond,
                    time.wMilliseconds
                    ),
                    COLOUR_MSG,
                    25
                    );
                hlpr.FormatDesc(GetStatusStr(itSingle->m_eStat, itSingle->m_eWaitReason), COLOUR_MSG, 10);
                hlpr.FormatDesc(GetSymFromAddr(it->m_dwStartAddr), COLOUR_PROC);
            }
        }
    }
    res.SetResult(hlpr.GetResult());
    */
    //return CmdReplyResult();
}
