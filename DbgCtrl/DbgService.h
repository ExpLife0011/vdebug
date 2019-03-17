#ifndef DBGSERVICE_DBGCTRL_H_H_
#define DBGSERVICE_DBGCTRL_H_H_
#include <ComLib/ComLib.h>
#include "DbgCtrlCom.h"

typedef void (__stdcall *pfnDbgEventProc)(const EventInfo &eventInfo, void *param);

class DbgCtrlApi DbgServiceBase {
public:
    virtual ~DbgServiceBase() {};
    virtual bool InitDbgService(const std::mstring &unique) = 0;
    virtual CtrlReply DispatchCurDbgger(const CtrlRequest &request) = 0;
    virtual CtrlReply DispatchSpecDbgger(DbggerType type, const CtrlRequest &request) = 0;
    virtual HDbgCtrl RegisterDbgEvent(const std::mstring &dbgEvent, pfnDbgEventProc pfn, void *param) = 0;
    virtual bool SetActivity(DbggerType type) = 0;

    static DbgServiceBase *_stdcall GetInstance();
};
#endif //DBGSERVICE_DBGCTRL_H_H_