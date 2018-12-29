#ifndef DBGCLIENT_DBGCTRL_H_H_
#define DBGCLIENT_DBGCTRL_H_H_
#include <ComStatic/ComStatic.h>
#include "DbgCtrlCom.h"

typedef std::ustring (__stdcall *pfnDbgClientProc)(const std::ustring &cmd, const std::ustring &content, void *param);

class DbgCtrlApi DbgClientBase {
public:
    virtual ~DbgClientBase() {};
    virtual bool InitClient(DbggerType type, const wchar_t *unique) = 0;
    virtual HDbgCtrl RegisterCtrlHandler(const wchar_t *cmd, pfnDbgClientProc pfn, void *param) = 0;

    static DbgClientBase *__stdcall newInstance();
}; 
#endif //DBGCLIENT_DBGCTRL_H_H_