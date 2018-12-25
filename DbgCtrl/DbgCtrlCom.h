#ifndef COMMON_DBGCTRL_H_H_
#define COMMON_DBGCTRL_H_H_

enum DbggerType {
    em_dbg_proc86,
    em_dbg_proc64,
    em_dbg_dump86,
    em_dbg_dump64
};

typedef long HDbgCtrl;

#ifndef DBGCTRL_EXPORTS
    #define DbgCtrlApi _declspec(dllimport)
#else
    #define DbgCtrlApi _declspec(dllexport)
#endif //DBGCTRL_EXPORTS

const char *__stdcall MakeDbgRequest(const char *cmd, const char *content);
#endif //COMMON_DBGCTRL_H_H_
