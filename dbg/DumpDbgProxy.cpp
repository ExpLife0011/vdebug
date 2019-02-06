#include <Windows.h>
#include <DbgCtrl/DbgProtocol.h>
#include "DumpDbgProxy.h"

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
    mDumpHlpr = new CMiniDumpHlpr();
    return true;
}

DumpDbgProxy::~DumpDbgProxy() {
}

CtrlReply DumpDbgProxy::OpenDump(const CtrlRequest &request, void *param) {
    mstring path = request.mContent["dumpPath"].asString();

    GetInstance()->mDumpHlpr->LodeDump(path);
    list<DumpModuleInfo> moduleSet;
    GetInstance()->mDumpHlpr->GetModuleSet(moduleSet);

    CtrlReply result;
    if (GetInstance()->mDumpHlpr->LodeDump(path))
    {
        result.mStatus = 0;
    } else {
        result.mStatus = 1;
        result.mShow = FormatA("调试器打开Dump文件失败，错误码:%d", GetLastError());
    }
    return result;
}

CtrlReply DumpDbgProxy::DumpProc(const CtrlRequest &request, void *param) {
    return CtrlReply();
}