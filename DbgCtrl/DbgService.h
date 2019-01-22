#ifndef DBGSERVICE_DBGCTRL_H_H_
#define DBGSERVICE_DBGCTRL_H_H_
#include <ComStatic/ComStatic.h>
#include "DbgCtrlCom.h"

typedef void (__stdcall *pfnDbgEventProc)(const std::mstring &event, const std::mstring &content, void *param);

class DbgCtrlApi DbgServiceBase {
public:
    virtual ~DbgServiceBase() {};
    virtual bool InitDbgService(const char *unique) = 0;
    virtual std::mstring DispatchCurDbgger(const std::mstring &cmd, const std::mstring &content) = 0;
    virtual std::mstring DispatchSpecDbgger(DbggerType type, const std::mstring &cmd, const std::mstring &content) = 0;
    virtual HDbgCtrl RegisterDbgEvent(const char *event, pfnDbgEventProc pfn, void *param) = 0;
    virtual bool SetActivity(DbggerType type) = 0;

    static DbgServiceBase *_stdcall GetInstance();
};
#endif //DBGSERVICE_DBGCTRL_H_H_