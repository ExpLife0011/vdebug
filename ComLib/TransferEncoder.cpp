#include "TransferEncoder.h"
#include "json/json.h"

using namespace std;
using namespace Json;

/*
{
    "cmd":"event",
        "content":{
            "type":"proc_add",
                "data":{
                    "added":[
                    {
                        "unique":12345,
                        "pid":1234,
                        "procPath":"d:\\abcdef.exe",
                        "procDesc":"desc",
                        "cmd":"abcdef",
                        "startTime":"2018-11-11 11:11:11:123",
                        "x64":1,
                        "session":1,
                        "user":"DESKTOP-DCTRL5K\\Administrator",
                        "sid":"S-1-5-21-2669793992-3689076831-3814312677-500"
                    },
                    ...
                    ],
                    "killed":[
                        1111,2222,3333
                    ]
            }
    }
}
*/
utf8_mstring __stdcall EncodeProcMon(const ProcInfoSet &procSet) {
    Value root, added(arrayValue), killed(arrayValue);
    for (list<ProcMonInfo>::const_iterator it = procSet.mAddSet.begin() ; it != procSet.mAddSet.end() ; it++)
    {
        Value node;
        node["unique"] = (UINT)it->procUnique;
        node["pid"] = (UINT)it->procPid;
        node["procPath"] = WtoU(it->procPath);
        node["procDesc"] = WtoU(it->procDesc);
        node["cmd"] = WtoU(it->procCmd);
        node["startTime"] = WtoU(it->startTime);
        node["x64"] = int(it->x64);
        node["session"] = (UINT)it->sessionId;
        node["user"] = WtoU(it->procUser);
        node["sid"] = WtoU(it->procUserSid);
        added.append(node);
    }
    root["added"] = added;

    for (list<DWORD>::const_iterator ij = procSet.mKillSet.begin() ; ij != procSet.mKillSet.end() ; ij++)
    {
        killed.append((UINT)*ij);
    }
    root["killed"] = killed;
    return FastWriter().write(root);
}

ProcInfoSet __stdcall DecodeProcMon(const utf8_mstring &json) {
    ProcInfoSet result;
    Value root, added, killed;
    Reader().parse(json, root);
    added = root["added"], killed = root["killed"];

    for (size_t i = 0 ; i != added.size() ; i++)
    {
        Value node = added[i];
        ProcMonInfo info;
        info.procUnique = node["unique"].asUInt();
        info.procPid = node["pid"].asUInt();
        info.procPath = UtoW(node["procPath"].asString());
        info.procDesc = UtoW(node["procDesc"].asString());
        info.procCmd = UtoW(node["cmd"].asString());
        info.startTime = UtoW(node["startTime"].asString());
        info.x64 = node["x64"].asInt();
        info.sessionId = node["session"].asInt();
        info.procUser = UtoW(node["user"].asString());
        info.procUserSid = UtoW(node["sid"].asString());
        result.mAddSet.push_back(info);
    }

    for (size_t j = 0 ; j < killed.size() ; j++)
    {
        result.mKillSet.push_back(killed[j].asUInt());
    }
    return result;
}

std::utf8_mstring _declspec(dllexport) __stdcall EncodeProcCreate(const ProcCreateInfo &info) {
    Value content;
    content["pid"] = (UINT)info.mPid;
    content["image"] = WtoU(info.mImage);
    content["baseAddr"] = WtoU(info.mBaseAddr);
    content["entryAddr"] = WtoU(info.mEntryAddr);
    return FastWriter().write(content);
}

ProcCreateInfo _declspec(dllexport) __stdcall DecodeProcCreate(const std::utf8_mstring &json) {
    ProcCreateInfo info;
    Value content;
    Reader().parse(json, content);

    info.mPid = content["pid"].asUInt();
    info.mImage = UtoW(content["image"].asString());
    info.mBaseAddr = UtoW(content["baseAddr"].asString());
    info.mEntryAddr = UtoW(content["entryAddr"].asString());
    return info;
}

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
std::utf8_mstring _declspec(dllexport) __stdcall EncodeDllLoadInfo(const DllLoadInfo &info) {
    Value data;
    data["name"] = WtoU(info.mDllName);
    data["baseAddr"] = WtoU(info.mBaseAddr);
    data["endAddr"] = WtoU(info.mEndAddr);

    return FastWriter().write(data);
}

DllLoadInfo _declspec(dllexport) __stdcall DecodeDllLoadInfo(const std::utf8_mstring &json) {
    DllLoadInfo info;
    Value content;
    Reader().parse(json, content);

    info.mDllName = UtoW(content["name"].asString());
    info.mBaseAddr = UtoW(content["baseAddr"].asString());
    info.mEndAddr = UtoW(content["endAddr"].asString());
    return info;
}