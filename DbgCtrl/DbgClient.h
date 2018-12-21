#ifndef DBGCLIENT_DBGCTRL_H_H_
#define DBGCLIENT_DBGCTRL_H_H_
#include "DbgCtrlCom.h"

typedef const wchar_t (_stdcall *pfnDbgEventProc)(const wchar_t *cmd, const wchar_t *content, void *param);

class DbgCtrlApi DbgClientBase {
public:
    virtual ~DbgClientBase() = 0;
    virtual bool RegisterClient(DbggerType type, const wchar_t *unique) = 0;
    virtual bool RegisterEventProc(const wchar_t *cmd, pfnDbgEventProc pfn, void *param);
    virtual bool SendDbgEvent(const wchar_t *cmd, const wchar_t *content) = 0;

    static DbgClientBase *__stdcall GetInstance();
}; 
#endif //DBGCLIENT_DBGCTRL_H_H_