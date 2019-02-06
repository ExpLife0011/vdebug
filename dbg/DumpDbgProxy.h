#ifndef PROC_DBG_PROXY_DBG_H_H_
#define PROC_DBG_PROXY_DBG_H_H_
#include <Windows.h>
#include <DbgCtrl/DbgClient.h>
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
    static std::mstring __stdcall OpenDump(const std::mstring &cmdParam, const std::mstring &content, void *param);
    static std::mstring __stdcall DumpProc(const std::mstring &cmdParam, const std::mstring &content, void *param);

private:
    bool mInit;
    CMiniDumpHlpr *mDumpHlpr;
    DbgClientBase *mDumpClient;
    bool mx64;
};
#endif  //PROC_DBG_PROXY_DBG_H_H_