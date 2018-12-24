#ifndef DBGCLIENT_DBGCTRL_H_H_
#define DBGCLIENT_DBGCTRL_H_H_
#include "DbgCtrlCom.h"

typedef const wchar_t *(_stdcall *pfnDbgClientProc)(const wchar_t *cmd, const wchar_t *content, void *param);

class DbgCtrlApi DbgClientBase {
public:
    virtual ~DbgClientBase() {};
    virtual bool InitClient(DbggerType type, const wchar_t *unique) = 0;
    virtual HDbgCtrl RegisterCtrlHandler(const wchar_t *cmd, pfnDbgClientProc pfn, void *param) = 0;
    virtual bool SendDbgEvent(const wchar_t *cmd, const wchar_t *content) = 0;

    static DbgClientBase *__stdcall GetInstance();
}; 
#endif //DBGCLIENT_DBGCTRL_H_H_