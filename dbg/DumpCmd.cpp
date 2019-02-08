#include "DumpCmd.h"
#include "minidump.h"
#include "DbgCommon.h"

CDumpCmd::CDumpCmd() {
}

CDumpCmd::~CDumpCmd() {
}

CDumpCmd *CDumpCmd::GetInst() {
    static CDumpCmd *s_ptr = NULL;

    if (!s_ptr)
    {
        s_ptr = new CDumpCmd();
    }
    return s_ptr;
}

void CDumpCmd::InitProcCmd() {
}

CtrlReply CDumpCmd::OnCommand(const std::mstring &cmd, const std::mstring &cmdParam, const CmdUserParam *pParam) {
    //切换到指定线程
    if (cmd == "tc")
    {
        return OnCmdTc(cmdParam, pParam);
    }
    //展示指定线程
    else if (cmd == "ts")
    {
        return OnCmdTs(cmdParam, pParam);
    }
    //展示模块信息
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
    else if (cmd == "help" || cmd == "h")
    {
        return OnCmdHelp(cmdParam, pParam);
    }

    CtrlReply reply;
    reply.mShow = mstring("不支持的命令:") + cmd + "\n";
    return reply;
}

CtrlReply CDumpCmd::OnCmdDisass(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdUb(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdUf(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdKv(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdDb(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdDd(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdDu(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdDa(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdReg(const std::mstring &cmd, const CmdUserParam *pParam) {
    list<DumpThreadInfo> threadSet = CMiniDumpHlpr::GetInst()->GetThreadSet();

    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdTs(const std::mstring &cmd, const CmdUserParam *pParam) {
    list<DumpThreadInfo> threadSet = CMiniDumpHlpr::GetInst()->GetThreadSet();

    PrintFormater pf;
    pf << "序号" << "线程ID" << "teb" << "代码位置" << line_end;
    for (list<DumpThreadInfo>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++)
    {
        string a = FormatA("0x%02x", it->mIndex);
        string b = FormatA("0x%08x", it->m_dwThreadId);
        string c = FormatA("0x%08x", (DWORD)it->m_dwTeb);
        string d = FormatA("0x%08x %hs", (DWORD)it->m_context.m_dwCip, it->mCipSymbol.c_str());
        pf << a << b << c << d << line_end;
    }

    CtrlReply result;
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CDumpCmd::OnCmdTc(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdLm(const std::mstring &cmd, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}

CtrlReply CDumpCmd::OnCmdHelp(const std::mstring &param, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}