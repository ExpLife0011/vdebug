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
std::mstring __stdcall MakeDbgRelpy(int status, const std::mstring &reason, const std::mstring &result) {
    Value root;
    root["cmd"] = "reply";

    Value content;
    content["status"] = status;
    content["reason"] = reason;

    Value resultJson;
    Reader().parse(result, resultJson);
    content["result"] = resultJson;
    root["content"] = content;
    return FastWriter().write(root);
}

bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result) {
    return ParserDbgReply(reply, result) && (0 == result.m_code);
}

bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result) {
    Value root;

    Reader().parse(reply, root);
    Value content = root["content"];

    result.m_code = GetIntFromJson(content, "status");
    result.m_reason = GetStrFormJson(content, "reason");
    result.m_result = GetStrFormJson(content, "result");
    return true;
}