#ifndef COMMON_DBGCTRL_H_H_
#define COMMON_DBGCTRL_H_H_
#include <ComLib/ComLib.h>
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

    DbgReplyResult() {
        mCode = 0;
    }

    DbgReplyResult(int a, const std::mstring &b, const std::mstring &c) {
        mCode = a;
        mReason = b;
        mResult = c;
    }
};

//std::mstring __stdcall MakeDbgEvent(const std::mstring &eventName, const std::mstring &data);
std::mstring __stdcall MakeDbgRequest(const std::mstring &cmd, const std::mstring &content);
std::mstring __stdcall MakeDbgRelpy(const DbgReplyResult &result);
bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result);
bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result);

/*
调试命令回执解析，Cmd回执是在DbgProtocol之上的又一层封装
*/
#define CMD_MASK_SHOW       (1 << 0)    //展示字符串
#define CMD_MASK_RESULT     (1 << 1)    //结果Json

struct CmdRequest {
    std::mstring mCmd;
    int mCmdMode;

    CmdRequest() {
        mCmdMode = (CMD_MASK_SHOW | CMD_MASK_RESULT);
    }

    CmdRequest(const std::mstring &cmd) {
        mCmdMode = CMD_MASK_SHOW;
        mCmd = cmd;
    }
};
std::mstring MakeCmdRequest(const CmdRequest &request);
CmdRequest ParserCmdRequest(const std::mstring &json);

struct CmdReplyResult {
    int mCmdCode;               //状态码
    int mResultMode;            //展示状态 
    std::mstring mCmdLabel;     //命令标签
    std::mstring mCmdShow;      //展示字符串
    std::mstring mCmdResult;    //命令执行结果集

    CmdReplyResult() {
        mCmdLabel = SCI_LABEL_DEFAULT;
        mResultMode = (CMD_MASK_SHOW | CMD_MASK_RESULT);
        mCmdCode = 0;
    }

    CmdReplyResult(int code, const std::mstring &show, const std::mstring &result) {
        mCmdLabel = SCI_LABEL_DEFAULT;
        mResultMode = (CMD_MASK_SHOW | CMD_MASK_RESULT);
        mCmdCode = 0;
        mCmdShow = show;
        mCmdResult = result;
    }
};
std::mstring __stdcall MakeCmdReply(const CmdReplyResult &cmdResult);
bool __stdcall ParserCmdReply(const std::mstring &reply, CmdReplyResult &cmdResult);

/*
{
    "cmd":"event",
    "content":{
        "eventType":"moduleload",
        "mode":1,                                           //1:展示信息，2:结果信息
        "eventLabel":"Default",                             //展示标签
        "eventShow":"0xffaabbcc 0x11223344 kernel32.dll",   //展示内容
        "eventResult": {
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
}
*/
struct EventDbgInfo {
    std::mstring mEventType;
    int mEventMode;
    std::mstring mEventLabel;
    std::mstring mEventShow;
    Json::Value mEventResult;

    EventDbgInfo() {
        mEventLabel = SCI_LABEL_DEFAULT;
        mEventMode = (CMD_MASK_SHOW | CMD_MASK_RESULT);
        Json::Reader().parse("{}", mEventResult);
    }
};
std::mstring __stdcall MakeEventRequest(const EventDbgInfo &info);
bool __stdcall ParserEventRequest(const std::mstring eventStr, EventDbgInfo &info);
#endif //COMMON_DBGCTRL_H_H_
