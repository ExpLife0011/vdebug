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

#define REPLY_STAT_CMD_CODE_PARAM_ERR       1011

struct DbgReplyResult {
    int mCode;
    std::mstring mReason;
    std::mstring mResult;

    DbgReplyResult(int a, const std::mstring &b, const std::mstring &c) {
        mCode = a;
        mReason = b;
        mResult = c;
    }
};

std::mstring __stdcall MakeDbgEvent(const std::mstring &eventName, const std::mstring &data);
std::mstring __stdcall MakeDbgRequest(const std::mstring &cmd, const std::mstring &content);
std::mstring __stdcall MakeDbgRelpy(const DbgReplyResult &result);
bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result);
bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result);

/*
调试命令回执解析，Cmd回执是在DbgProtocol之上的又一层封装
*/
struct CmdReplyResult {
    int mCmdCode;
    std::mstring mCmdShow;
    std::mstring mCmdResult;

    CmdReplyResult() {
        mCmdCode = 0;
    }

    CmdReplyResult(int code, const std::mstring &show, const std::mstring &result) {
        mCmdCode = 0;
        mCmdShow = show;
        mCmdResult = result;
    }
};
std::mstring __stdcall MakeCmdReply(const CmdReplyResult &cmdResult);
bool __stdcall ParserCmdReply(const std::mstring &reply, CmdReplyResult &cmdResult);
#endif //COMMON_DBGCTRL_H_H_
