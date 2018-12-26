#ifndef COMMON_DBGCTRL_H_H_
#define COMMON_DBGCTRL_H_H_
#include <ComStatic/ComStatic.h>

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
#endif

struct DbgReplyResult {
    int m_code;
    std::ustring m_reason;
    std::ustring m_result;
};

std::mstring __stdcall MakeDbgEvent(const std::mstring &event, const std::mstring &data);
std::mstring __stdcall MakeDbgRequest(const std::mstring &cmd, const std::mstring &content);
std::mstring __stdcall MakeDbgRelpy(int status, const std::mstring &reason, const std::mstring &result);
bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result);
bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result);
#endif //COMMON_DBGCTRL_H_H_
