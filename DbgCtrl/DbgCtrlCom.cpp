#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include "DbgCtrlCom.h"

using namespace std;
using namespace Json;

/*
{
    "cmd":"event",
    "content":{
        "type":"moduleload",
        "data":{
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
    }
}
*/
std::mstring __stdcall MakeDbgEvent(const std::mstring &event, const std::mstring &data) {
    Value root;
    root["cmd"] = "event";

    Value content;
    content["type"] = event;
    Value data1;
    Reader().parse(data, data1);
    content["data"] = data1;
    root["content"] = content;

    return FastWriter().write(root);
}

std::mstring __stdcall MakeDbgRequest(const std::mstring &cmd, const std::mstring &content) {
    Value root;
    root["cmd"] = cmd;

    Value contentJson;
    Reader().parse(content, contentJson);
    root["content"] = contentJson;

    return FastWriter().write(root);
}

/*
{
    "cmd": "reply",
        "content": {
            "command": "kv",
            "status": 0,
            "reason": "abcdef",
            "result": [{
                "retaddr": "0x0xabcd12ff",
                "param0": "0xabcd1234",
                "param1": "0xabcd1234",
                "param2": "0xabcd1233"
            }]
        }
}
*/
std::mstring __stdcall MakeDbgRelpy(const DbgReplyResult &result) {
    Value root;
    root["cmd"] = "reply";

    Value content;
    content["status"] = result.mCode;
    content["reason"] = result.mReason;

    Value resultJson;
    Reader().parse(result.mResult, resultJson);
    content["result"] = resultJson;
    root["content"] = content;
    return FastWriter().write(root);
}

bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result) {
    return ParserDbgReply(reply, result) && (0 == result.mCode);
}

bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result) {
    Value root;

    Reader().parse(reply, root);
    Value content = root["content"];

    result.mCode = GetIntFromJson(content, "status");
    result.mReason = GetStrFormJson(content, "reason");
    result.mResult = GetStrFormJson(content, "result");
    return true;
}

/*
{
    "cmd":"RunCmd",
    "content":{
        "mode":1,                          //1:仅返回展示字符串 2:返回Json格式的执行结果
        "cmd":"bp kernen32!CreateFileW"    //cmd 内容
    }
}

{
    "cmd": "reply",
    "content": {
        "status": 0,
        "reason": "abcdef",
        "result":{
            "cmdCode":0,
            "mode":1,
            "cmdLabel":"CallStack",                     //展示标签
            "cmdShow":"abcd1234",                       //展示内容
            "cmdResult": [{
                "addr": "0x0xabcd12ff",
                "function":"kernel32!CreateFileW",
                "param0": "0xabcd1234",
                "param1": "0xabcd1234",
                "param2": "0xabcd1233",
                "param3": "0xabcd1233"
            }]
        } 
    }
}
*/
mstring MakeCmdRequest(const CmdRequest &request) {
    Value json;
    json["mode"] = request.mCmdMode;
    json["cmd"] = request.mCmd;
    return FastWriter().write(json);
}

CmdRequest ParserCmdRequest(const mstring &json) {
    Value tmp;
    Reader().parse(json, tmp);

    CmdRequest result;
    result.mCmdMode = tmp["mode"].asInt();
    result.mCmd = tmp["cmd"].asString();
    return result;
}

mstring __stdcall MakeCmdReply(const CmdReplyResult &cmdResult) {
    Value root;
    root["cmd"] = "reply";

    Value content;
    content["status"] = 0;
    content["reason"] = "";

    Value result;
    result["cmdCode"] = cmdResult.mCmdCode;
    result["mode"] = cmdResult.mResultMode;

    Value tmp;
    Reader().parse(cmdResult.mCmdResult, tmp);
    result["cmdResult"] = tmp;
    result["cmdLabel"] = cmdResult.mCmdLabel;
    result["cmdShow"] = cmdResult.mCmdShow;
    content["result"] = result;

    root["content"] = content;
    return FastWriter().write(root);
}

bool __stdcall ParserCmdReply(const std::mstring &reply, CmdReplyResult &cmdResult) {
    Value root;

    Reader().parse(reply, root);
    if (root.type() != objectValue || root["cmd"].asString() != "reply")
    {
        return false;
    }

    Value content = root["content"];
    if (content.type() != objectValue)
    {
        return false;
    }

    Value result = content["result"];
    if (result.type() != objectValue)
    {
        return false;
    }
    cmdResult.mCmdCode = result["cmdCode"].asInt();
    cmdResult.mResultMode = result["mode"].asInt();
    cmdResult.mCmdLabel = result["cmdLabel"].asString();

    if (cmdResult.mResultMode & CMD_MASK_RESULT)
    {
        cmdResult.mCmdResult = FastWriter().write(result["cmdResult"]);
    }

    if (cmdResult.mResultMode & CMD_MASK_SHOW)
    {
        cmdResult.mCmdShow = result["cmdShow"].asString();
    }
    return true;
}