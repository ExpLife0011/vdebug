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

enum DbggerStatus
{
    em_dbg_status_init,
    em_dbg_status_busy,
    em_dbg_status_free
};

typedef long HDbgCtrl;

#ifndef DBGCTRL_EXPORTS
    #define DbgCtrlApi _declspec(dllimport)
#else
    #define DbgCtrlApi _declspec(dllexport)
#endif

#define REPLY_STAT_CMD_CODE_PARAM_ERR       1011

struct CtrlRequest {
    std::mstring mCmd;
    Json::Value mContent;
};

struct CtrlReply {
    int mStatus;
    std::mstring mLabel;
    std::mstring mShow;
    Json::Value mResult;

    CtrlReply() {
        mStatus = 0;
        mLabel = SCI_LABEL_DEFAULT;
    }
};

std::mstring __stdcall MakeRequest(const CtrlRequest &request);
CtrlRequest __stdcall ParserRequest(const std::mstring &cmd);
std::mstring __stdcall MakeReply(const CtrlReply &reply);
CtrlReply __stdcall ParserReply(const std::mstring &reply);

struct EventInfo {
    std::mstring mEvent;
    std::mstring mLabel;
    std::mstring mShow;
    Json::Value mContent;

    EventInfo() {
        mLabel = SCI_LABEL_DEFAULT;
    }
};
std::mstring __stdcall MakeEvent(const EventInfo &eventInfo);
EventInfo __stdcall ParserEvent(const std::mstring &reply);
#endif //COMMON_DBGCTRL_H_H_
