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

std::mstring DumpDbgProxy::OpenDump(const std::mstring &cmdParam, const std::mstring &content, void *param) {
    Value json;
    Reader().parse(content, json);
    mstring path = json["path"].asString();

    GetInstance()->mDumpHlpr->LodeDump(path);
    list<DumpModuleInfo> moduleSet;
    GetInstance()->mDumpHlpr->GetModuleSet(moduleSet);

    DbgReplyResult result;

    if (GetInstance()->mDumpHlpr->LodeDump(path))
    {
        result.mCode = 0;
    } else {
        result.mCode = 1;
        result.mReason = FormatA("调试器打开Dump文件失败，错误码:%d", GetLastError());
    }
    return MakeDbgRelpy(result);
}

std::mstring DumpDbgProxy::DumpProc(const std::mstring &cmdParam, const std::mstring &content, void *param) {
    DbgReplyResult result(0, "", "");
    return MakeDbgRelpy(result);
}