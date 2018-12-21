#ifndef DBGSERVICE_DBGCTRL_H_H_
#define DBGSERVICE_DBGCTRL_H_H_
#include "DbgCtrlCom.h"

class DbgCtrlApi DbgServiceBase {
public:
    virtual ~DbgServiceBase() {};
    virtual bool InitDbgService(const wchar_t *unique) = 0;
    virtual bool DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content) = 0;
    virtual bool SetActivity(DbggerType type) = 0;

    static DbgServiceBase *_stdcall GetInstance();
};
#endif //DBGSERVICE_DBGCTRL_H_H_