#ifndef DBGSERVICE_DBGCTRL_H_H_
#define DBGSERVICE_DBGCTRL_H_H_
#include "DbgCtrlCom.h"

typedef void (__stdcall *pfnDbgEventProc)(const wchar_t *event, const wchar_t *content, void *param);

class DbgCtrlApi DbgServiceBase {
public:
    virtual ~DbgServiceBase() {};
    virtual bool InitDbgService(const wchar_t *unique) = 0;
    virtual const wchar_t * DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content) = 0;
    virtual HDbgCtrl RegisterDbgEvent(const wchar_t *event, pfnDbgEventProc pfn, void *param) = 0;
    virtual bool SetActivity(DbggerType type) = 0;

    static DbgServiceBase *_stdcall GetInstance();
};
#endif //DBGSERVICE_DBGCTRL_H_H_