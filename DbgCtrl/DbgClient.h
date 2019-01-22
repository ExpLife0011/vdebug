#ifndef DBGCLIENT_DBGCTRL_H_H_
#define DBGCLIENT_DBGCTRL_H_H_
#include <ComStatic/ComStatic.h>
#include "DbgCtrlCom.h"

typedef std::mstring (__stdcall *pfnDbgClientProc)(const std::mstring &cmd, const std::mstring &content, void *param);

class DbgCtrlApi DbgClientBase {
public:
    virtual ~DbgClientBase() {};
    virtual bool InitClient(DbggerType type, const char *unique) = 0;
    virtual HDbgCtrl RegisterCtrlHandler(const char *cmd, pfnDbgClientProc pfn, void *param) = 0;
    virtual bool ReportDbgEvent(const std::mstring &content) = 0;
    static DbgClientBase *__stdcall newInstance();
}; 
#endif //DBGCLIENT_DBGCTRL_H_H_