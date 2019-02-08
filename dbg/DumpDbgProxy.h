#ifndef PROC_DBG_PROXY_DBG_H_H_
#define PROC_DBG_PROXY_DBG_H_H_
#include <Windows.h>
#include <DbgCtrl/DbgClient.h>
#include "DumpCmd.h"
#include "minidump.h"

class DumpDbgProxy {
private:
    DumpDbgProxy();
public:
    static DumpDbgProxy *GetInstance();
    bool InitDumpDbgProxy(const char *unique);
    virtual ~DumpDbgProxy();

private:
    //Dbg Ctrl
    static CtrlReply __stdcall OpenDump(const CtrlRequest &request, void *param);
    static CtrlReply __stdcall DumpProc(const CtrlRequest &request, void *param);
    static CtrlReply __stdcall RunCmd(const CtrlRequest &request, void *param);

    std::mstring GetSystemStr(const DumpSystemInfo &system) const;
    std::mstring GetExceptionDesc(const DumpException &exception) const;
private:
    bool mInit;
    CMiniDumpHlpr *mDumpHlpr;
    CDumpCmd *mCmdRunner;
    DbgClientBase *mDumpClient;
    bool mx64;
};
#endif  //PROC_DBG_PROXY_DBG_H_H_