#include <Windows.h>
#include <DbgCtrl/DbgProtocol.h>
#include "DumpDbgProxy.h"
#include "DbgCommon.h"

DumpDbgProxy::DumpDbgProxy() {
    mDumpHlpr = NULL;
    mInit = false;
    mx64 = (8 == sizeof(void *));
}

DumpDbgProxy *DumpDbgProxy::GetInstance() {
    static DumpDbgProxy *s_ptr = NULL;

    if (s_ptr == NULL)
    {
        s_ptr = new DumpDbgProxy();
    }
    return s_ptr;
}

bool DumpDbgProxy::InitDumpDbgProxy(const char *unique) {
    if (mInit)
    {
        return true;
    }

    mInit = true;
    mDumpClient = DbgClientBase::newInstance();
    if (mx64) {
        mDumpClient->InitClient(em_dbg_dump64, unique);
    }
    else {
        mDumpClient->InitClient(em_dbg_dump86, unique);
    }

    mDumpClient->RegisterCtrlHandler(DBG_CTRL_OPEN_DUMP, OpenDump, this);
    mDumpClient->RegisterCtrlHandler(DBG_CTRL_DUMP_PROC, DumpProc, this);
    mDumpClient->RegisterCtrlHandler(DBG_CTRL_RUNCMD, RunCmd, this);
    mDumpHlpr = CMiniDumpHlpr::GetInst();
    mCmdRunner = CDumpCmd::GetInst();
    return true;
}

DumpDbgProxy::~DumpDbgProxy() {
}

mstring DumpDbgProxy::GetSystemStr(const DumpSystemInfo &system) const {
    mstring show = CDbgCommon::GetSystemStr(system.m_dwMajVer, system.m_dwMinVer, system.mProductType);
    show += system.m_strSpStr + " " + FormatA("Build %d ", system.m_dwBuildNum);
    if (system.m_eCpuType == em_processor_x86)
    {
        show += "32位操作系统";
    } else {
        show += "64位操作系统";
    }
    return show;
}

mstring DumpDbgProxy::GetExceptionDesc(const DumpException &exception) const {
    mstring result;
    result += "\n";
    result += "dump文件中存在异常信息\n";

    PrintFormater pf;
    pf << "异常类型" << FormatA("0x%08x, %hs", exception.mExceptionCode, CDbgCommon::GetExceptionDesc(exception.mExceptionCode).c_str()) << line_end;

    if (exception.mExceptionFlags == 0)
    {
        pf << "异常标识" << "可继续执行" << line_end;
    } else {
        pf << "异常标识" << "不能继续执行" << line_end;
    }

    DWORD64 addr = exception.mExceptionAddress;
    mstring symbol = CDbgCommon::GetSymFromAddr(addr);
    pf << "异常地址" << FormatA("0x%08x %hs", (DWORD)addr, symbol.c_str()) << line_end;
    return (result + pf.GetResult());
}

CtrlReply DumpDbgProxy::OpenDump(const CtrlRequest &request, void *param) {
    mstring path = request.mContent["dumpPath"].asString();

    CtrlReply result;
    result.mStatus = 1;
    result.mShow = FormatA("dump文件路径:%hs\n", path.c_str());
    if (GetInstance()->mDumpHlpr->LodeDump(path))
    {
        DumpSystemInfo system = GetInstance()->mDumpHlpr->GetSystemInfo();
        result.mStatus = 0;
        result.mShow += GetInstance()->GetSystemStr(system) + "\n";

        DumpException exception = GetInstance()->mDumpHlpr->GetException();
        if (exception.mThreadId != 0)
        {
            GetInstance()->mDumpHlpr->SetCurThread(exception.mThreadId);
            result.mShow += GetInstance()->GetExceptionDesc(exception);
        }
        result.mResult["tid"] = (int)CMiniDumpHlpr::GetInst()->GetCurThread().m_dwThreadId;
    } else {
        result.mStatus = 1;
        result.mShow += FormatA("调试器打开Dump文件失败，错误码:%d\n", GetLastError());
    }
    return result;
}

CtrlReply DumpDbgProxy::DumpProc(const CtrlRequest &request, void *param) {
    return CtrlReply();
}

CtrlReply DumpDbgProxy::RunCmd(const CtrlRequest &request, void *param) {
    mstring data = request.mContent["command"].asString();
    data.trim();

    CtrlReply reply;
    if (data.empty())
    {
        reply.mStatus = DBG_CMD_SYNTAX_ERR;
        reply.mShow = "命令语法错误";
        return reply;
    }

    return GetInstance()->mCmdRunner->RunCommand(data);
}