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
    list<STACKFRAME64> callStack = CMiniDumpHlpr::GetInst()->GetCurrentStackFrame();

    CtrlReply result;
    if (callStack.empty())
    {
        return result;
    }

    CallStackData callSet;
    CallStackSingle single;
    PrintFormater pf;
    pf << "内存地址" << "返回地址" << "参数列表" << space << space << space << "符号名称" << line_end;
    for (list<STACKFRAME64>::const_iterator it = callStack.begin() ; it != callStack.end() ; it++)
    {
        single.mAddr = FormatA("%08x", it->AddrPC.Offset);
        single.mReturn = FormatA("%08x", it->AddrReturn);
        single.mParam0 = FormatA("%08x", it->Params[0]);
        single.mParam1 = FormatA("%08x", it->Params[1]);
        single.mParam2 = FormatA("%08x", it->Params[2]);
        single.mParam3 = FormatA("%08x", it->Params[3]);

        DumpModuleInfo module = CMiniDumpHlpr::GetInst()->GetModuleFromAddr((DWORD64)it->AddrPC.Offset);
        single.mFunction = FormatA("%hs", CDbgCommon::GetSymFromAddr((DWORD64)it->AddrPC.Offset, module.m_strModuleName, module.m_dwBaseAddr).c_str());
        callSet.mCallStack.push_back(single);

        pf << single.mAddr << single.mReturn << single.mParam0 << single.mParam1 << single.mParam2 << single.mParam3 << single.mFunction << line_end;
    }
    result.mStatus = 0;
    result.mResult = EncodeCmdCallStack(callSet);
    result.mShow = pf.GetResult();
    return result;
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
    DumpThreadInfo curThread = CMiniDumpHlpr::GetInst()->GetCurThread();

    CONTEXTx64 *pContext = curThread.m_context.mFullContext;
    DumpModuleInfo module = CMiniDumpHlpr::GetInst()->GetModuleFromAddr(pContext->Rip);
    mstring symbol = CDbgCommon::GetSymFromAddr((DWORD64)pContext->Rip, module.m_strModuleName, module.m_dwBaseAddr);;

    CtrlReply result;
    PrintFormater pf;
    pf << FormatA("eax=0x%08x", (DWORD)pContext->Rax) << FormatA("ebx=0x%08x", (DWORD)pContext->Rbx);
    pf << FormatA("ecx=0x%08x", (DWORD)pContext->Rcx) << FormatA("edx=0x%08x", (DWORD)pContext->Rdx) << line_end;

    pf << FormatA("esi=0x%08x", (DWORD)pContext->Rsi) << FormatA("edi=0x%08x", (DWORD)pContext->Rdi);
    pf << FormatA("eip=0x%08x", (DWORD)pContext->Rip) << FormatA("esp=0x%08x", (DWORD)pContext->Rsp) << line_end;

    pf << FormatA("ebp=0x%08x", (DWORD)pContext->Rbp) << space << space << space << line_end;
    result.mShow = pf.GetResult();
    pf.Reset();

    pf << FormatA("当前指令位置 0x%08x %hs", (DWORD)pContext->Rip, symbol.c_str()) << line_end;
    result.mShow += pf.GetResult();
    return result;
}

CtrlReply CDumpCmd::OnCmdTs(const std::mstring &cmd, const CmdUserParam *pParam) {
    vector<DumpThreadInfo> threadSet = CMiniDumpHlpr::GetInst()->GetThreadSet();

    PrintFormater pf;
    pf << "序号" << "线程ID" << "teb" << "代码位置" << line_end;
    for (vector<DumpThreadInfo>::const_iterator it = threadSet.begin() ; it != threadSet.end() ; it++)
    {
        string a = FormatA("0x%02x", it->mIndex);
        string b = FormatA("0x%08x(%d)", it->m_dwThreadId, it->m_dwThreadId);
        string c = FormatA("0x%08x", (DWORD)it->m_dwTeb);
        string d = FormatA("0x%08x %hs", (DWORD)it->m_context.m_dwCip, it->mCipSymbol.c_str());
        pf << a << b << c << d << line_end;
    }

    CtrlReply result;
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CDumpCmd::OnCmdTc(const std::mstring &param, const CmdUserParam *pParam) {
    mstring str(param);
    str.trim();

    DWORD64 dwSerial = 0;
    GetNumFromStr(str, dwSerial);

    CtrlReply reply;
    vector<DumpThreadInfo> threadSet = CMiniDumpHlpr::GetInst()->GetThreadSet();
    if (dwSerial >= threadSet.size())
    {
        reply.mShow = "未找到需要切换的线程";
    } else {
        CMiniDumpHlpr::GetInst()->SetCurThread(threadSet[(int)dwSerial].m_dwThreadId);
        DWORD tid = (DWORD)threadSet[(DWORD)dwSerial].m_dwThreadId;
        reply.mShow = FormatA("切换至0x%04x号线程成功，当前线程0x%08x(%d)\n", (DWORD)dwSerial, tid, tid);
    }
    return reply;
}

CtrlReply CDumpCmd::OnCmdLm(const std::mstring &cmd, const CmdUserParam *pParam) {
    list<DumpModuleInfo> moduleSet = CMiniDumpHlpr::GetInst()->GetModuleSet();

    PrintFormater pf;
    pf << "起始地址" << "结束地址" << "版本信息" << "时间戳" <<"模块路径" << line_end;
    for (list<DumpModuleInfo>::const_iterator it = moduleSet.begin() ; it != moduleSet.end() ; it++)
    {
        string a = FormatA("0x%08x", it->m_dwBaseAddr);
        string b = FormatA("0x%08x", it->m_dwBaseAddr + it->m_dwModuleSize);
        string c = it->mVersion;
        string d = it->mTimeStr;
        string e = it->m_strModulePath;
        pf << a << b << c << d << e << line_end;
    }

    CtrlReply result;
    result.mShow = pf.GetResult();
    return result;
}

CtrlReply CDumpCmd::OnCmdHelp(const std::mstring &param, const CmdUserParam *pParam) {
    CtrlReply reply;
    return reply;
}