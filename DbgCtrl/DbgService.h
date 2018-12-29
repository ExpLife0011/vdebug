#ifndef DBGSERVICE_DBGCTRL_H_H_
#define DBGSERVICE_DBGCTRL_H_H_
#include <ComStatic/ComStatic.h>
#include "DbgCtrlCom.h"

typedef void (__stdcall *pfnDbgEventProc)(const std::ustring &event, const std::ustring &content, void *param);

class DbgCtrlApi DbgServiceBase {
public:
    virtual ~DbgServiceBase() {};
    virtual bool InitDbgService(const wchar_t *unique) = 0;
    virtual std::ustring DispatchCurDbgger(const std::ustring &cmd, const std::ustring &content) = 0;
    virtual std::ustring DispatchSpecDbgger(DbggerType type, const std::ustring &cmd, const std::ustring &content) = 0;
    virtual HDbgCtrl RegisterDbgEvent(const wchar_t *event, pfnDbgEventProc pfn, void *param) = 0;
    virtual bool SetActivity(DbggerType type) = 0;

    static DbgServiceBase *_stdcall GetInstance();
};
#endif //DBGSERVICE_DBGCTRL_H_H_