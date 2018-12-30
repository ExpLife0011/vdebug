#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include "DbgCtrlCom.h"

using namespace std;

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
    cJSON *root = cJSON_CreateObject();
    JsonAutoDelete abc(root);
    cJSON_AddStringToObject(root, "cmd", "event");
    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "type", event.c_str());
    cJSON *data1 = cJSON_Parse(data.c_str());
    cJSON_AddItemToObject(content, "data", data1);
    cJSON_AddItemToObject(root, "content", content);

    return cJSON_PrintUnformatted(root);
}

std::mstring __stdcall MakeDbgRequest(const std::mstring &cmd, const std::mstring &content) {
    cJSON *root = cJSON_CreateObject();
    JsonAutoDelete abc(root);
    cJSON_AddStringToObject(root, "cmd", cmd.c_str());

    cJSON *content1 = cJSON_Parse(content.c_str());
    cJSON_AddItemToObject(root, "content", content1);

    return cJSON_PrintUnformatted(root);
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
    cJSON *root = cJSON_CreateObject();
    JsonAutoDelete abc(root);
    cJSON_AddStringToObject(root, "cmd", "reply");

    cJSON *content = cJSON_CreateObject();
    cJSON_AddNumberToObject(content, "status", status);
    cJSON_AddStringToObject(content, "reason", reason.c_str());
    cJSON *res = cJSON_Parse(result.c_str());
    cJSON_AddItemToObject(content, "result", res);

    cJSON_AddItemToObject(root, "content", content);
    return cJSON_PrintUnformatted(root);
}

bool __stdcall IsDbgReplySucc(const std::mstring &reply, DbgReplyResult &result) {
    return ParserDbgReply(reply, result) && (0 == result.m_code);
}

bool __stdcall ParserDbgReply(const std::mstring &reply, DbgReplyResult &result) {
    cJSON *root = cJSON_Parse(reply.c_str());
    JsonAutoDelete abc(root);
    cJSON *content = cJSON_GetObjectItem(root, "content");

    result.m_code = GetIntFromJson(content, "status");
    result.m_reason = GetStrFormJson(content, "reason");
    result.m_result = GetStrFormJson(content, "result");
    return true;
}