#ifndef DBGCLIENT_DBGCTRL_H_H_
#define DBGCLIENT_DBGCTRL_H_H_
#include <ComLib/ComLib.h>
#include "DbgCtrlCom.h"

typedef CtrlReply (__stdcall *pfnDbgClientProc)(const CtrlRequest &request, void *param);

class DbgCtrlApi DbgClientBase {
public:
    virtual ~DbgClientBase() {};
    virtual bool InitClient(DbggerType type, const mstring &unique) = 0;
    virtual HDbgCtrl RegisterCtrlHandler(const std::mstring &cmd, pfnDbgClientProc pfn, void *param) = 0;
    virtual bool ReportDbgEvent(const EventInfo &eventInfo) = 0;
    static DbgClientBase *__stdcall newInstance();
}; 
#endif //DBGCLIENT_DBGCTRL_H_H_