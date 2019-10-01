#include <Windows.h>
#include <ComLib/ComLib.h>
#include <ComLib/ComLib.h>
#include "DbgCtrlCom.h"

using namespace std;
using namespace Json;

/*
协议按应用场景抽象成两类，一问一答的应答方式和单向的数据上报
一问一答的应答方式
请求方：
{
    "type":"ctrl",
    "cmd":"attach",
    "content": {
        "pid":1234
    }
}

应答方：
{
    "status":0,
    "label":"default",
    "show":"showmsg",
    "result":{
        ...
    }
}

单向数据推送
{
    "type":"event",
    "event":"moduleload",
    "label":"default",
    "show":"xxxx 模块加载",
    "content":{
    }
}
*/

std::mstring __stdcall MakeRequest(const CtrlRequest &request) {
    Value root;
    root["type"] = "ctrl";
    root["cmd"] = request.mCmd;
    root["content"] = request.mContent;

    return FastWriter().write(root);
}

CtrlRequest __stdcall ParserRequest(const std::mstring &cmd){
    Value root;
    Reader().parse(cmd, root);

    CtrlRequest result;
    result.mCmd = root["cmd"].asString();
    result.mContent = root["content"];
    return result;
}

std::mstring __stdcall MakeReply(const CtrlReply &reply) {
    Value root;
    root["status"] = reply.mStatus;
    root["label"] = reply.mLabel;
    root["show"] = reply.mShow;
    root["result"] = reply.mResult;
    return FastWriter().write(root);
}

CtrlReply __stdcall ParserReply(const std::mstring &reply) {
    Value root;
    Reader().parse(reply, root);

    CtrlReply result;
    result.mStatus = root["status"].asInt();
    result.mLabel = root["label"].asString();
    result.mShow = root["show"].asString();
    result.mResult = root["result"];
    return result;
}

std::mstring __stdcall MakeEvent(const EventInfo &eventInfo) {
    Value root;
    root["type"] = "event";
    root["event"] = eventInfo.mEvent;
    root["show"] = eventInfo.mShow;
    root["content"] = eventInfo.mContent;

    return FastWriter().write(root);
}

EventInfo __stdcall ParserEvent(const std::mstring &reply) {
    Value root;

    Reader().parse(reply, root);
    EventInfo result;
    result.mEvent = root["event"].asString();
    result.mShow = root["show"].asString();
    result.mContent = root["content"];

    return result;
}