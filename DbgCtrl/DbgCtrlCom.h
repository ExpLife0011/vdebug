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
���������ִ������Cmd��ִ����DbgProtocol֮�ϵ���һ���װ
*/
#define CMD_MASK_SHOW       (1 << 0)    //չʾ�ַ���
#define CMD_MASK_RESULT     (1 << 1)    //���Json

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
    int mCmdCode;               //״̬��
    int mResultMode;            //չʾ״̬ 
    std::mstring mCmdLabel;     //�����ǩ
    std::mstring mCmdShow;      //չʾ�ַ���
    std::mstring mCmdResult;    //����ִ�н����

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
#endif //COMMON_DBGCTRL_H_H_
