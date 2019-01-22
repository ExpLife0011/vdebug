#include <string>
#include <mq/mq.h>
#include <map>
#include <list>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include "DbgProtocol.h"
#include "DbgService.h"
#include "DbgCtrlTool.h"

using namespace std;
using namespace Json;

struct DbgServiceCache {
    string m_event;
    void *m_param;
    pfnDbgEventProc m_proc;
    int m_idex;
};

class DbgService : public DbgServiceBase, CCriticalSectionLockable {
public:
    DbgService();
    virtual ~DbgService();
    virtual bool InitDbgService(const char *unique);
    virtual std::mstring DispatchCurDbgger(const std::mstring &cmd, const std::mstring &content);
    virtual std::mstring DispatchSpecDbgger(DbggerType type, const std::mstring &cmd, const std::mstring &content);
    virtual HDbgCtrl RegisterDbgEvent(const char *event, pfnDbgEventProc pfn, void *param);
    virtual bool SetActivity(DbggerType type);

private:
    std::mstring GetSpecChannel(DbggerType type) const;
    bool DispatchEventToRegister(const mstring &cmd, const mstring &content) const;
    static LPCSTR WINAPI ServerNotify(LPCSTR wszChannel, LPCSTR wszContent, void *pParam);

private:
    map<string, list<DbgServiceCache *>> m_RegisterSet;
    string m_unique;
    unsigned short m_port;
    long m_curIndex;
    string m_curChannel;
    DbggerType m_DbgClient;
};

DbgService::DbgService() :m_port(0), m_curIndex(0xffea){
}

DbgService::~DbgService() {
}

LPCSTR DbgService::ServerNotify(LPCSTR szChannel, LPCSTR szContent, void *pParam) {
    string result;
    DbgService *pThis = (DbgService *)pParam;

    do 
    {
        if (0 == lstrcmpA(szChannel, MQ_CHANNEL_DBG_SERVER))
        {
            Value root;
            Reader().parse(szContent, root);

            if (root.type() != objectValue)
            {
                break;
            }

            mstring cmd = GetStrFormJson(root, "cmd");
            Value content = root["content"];

            if (cmd == DBG_DBG_EVENT)
            {
                /*
                {
                    "cmd":"event",
                    "content":{
                        "type":"proccreate",
                        "data":{
                            "pid":1234,
                            "image":"d:\\desktop\\1234.exe",
                            "baseAddr":"0x4344353",
                            "entryAddr":"0x4344389"
                        }
                    }
                }*/
                mstring event = GetStrFormJson(content, "type");
                mstring data = GetStrFormJson(content, "data");
                pThis->DispatchEventToRegister(event, data);
            }
        } else {
        }
    } while (false);
    return MsgStrCopy(result.c_str());
}

/*
{
    "cmd":"event",
    "content":{
        "type":"proccreate",
        "data":{
            "pid":1234,
            "image":"d:\\desktop\\1234.exe",
            "baseAddr":"0x4344353",
            "entryAddr":"0x4344389"
        }
    }
}
*/
bool DbgService::DispatchEventToRegister(const mstring &cmd, const mstring &content) const {
    map<string, list<DbgServiceCache *>>::const_iterator it = m_RegisterSet.find(cmd);

    if (it == m_RegisterSet.end())
    {
        return false;
    }

    for (list<DbgServiceCache *>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
    {
        DbgServiceCache *ptr = *ij;
        ptr->m_proc(cmd, content, ptr->m_param);
    }
    return true;
}

bool DbgService::InitDbgService(const char *unique) {
    m_unique = unique;
    m_port = CalPortFormUnique(unique);

    MsgInitServ(m_port);
    MsgInitClient(m_port);
    MsgRegister(MQ_CHANNEL_DBG_SERVER, ServerNotify, this);
    return true;
}

mstring DbgService::DispatchCurDbgger(const mstring &cmd, const mstring &content) {
    return DispatchSpecDbgger(m_DbgClient, cmd, content);
}

mstring DbgService::DispatchSpecDbgger(DbggerType type, const mstring &cmd, const mstring &content) {
    mstring channel = GetSpecChannel(type);

    Value root;
    root["cmd"] = cmd;
    root["content"] = content;

    LPCSTR sz = MsgSendForResult(channel.c_str(), FastWriter().write(root).c_str());
    mstring result = sz;
    MsgStrFree(sz);
    return result;
}

mstring DbgService::GetSpecChannel(DbggerType type) const {
    mstring channel;
    switch (type) {
        case em_dbg_proc86:
            channel = MQ_CHANNEL_DBG_CLIENT32;
            break;
        case em_dbg_proc64:
            channel = MQ_CHANNEL_DBG_CLIENT64;
            break;
        case em_dbg_dump86:
            channel = MQ_CHANNEL_DBG_DUMP32;
            break;
        case em_dbg_dump64:
            channel = MQ_CHANNEL_DBG_DUMP64;
            break;
    }
    return channel;
}

bool DbgService::SetActivity(DbggerType type) {
    m_DbgClient = type;
    m_curChannel = GetSpecChannel(type);
    return true;
}

HDbgCtrl DbgService::RegisterDbgEvent(const char *event, pfnDbgEventProc pfn, void *param) {
    CScopedLocker lock(this);
    DbgServiceCache *cache = new DbgServiceCache();
    cache->m_idex = m_curIndex++;
    cache->m_event = event;
    cache->m_param = param;
    cache->m_proc = pfn;
    m_RegisterSet[event].push_back(cache);
    return cache->m_idex;
}

DbgServiceBase *DbgServiceBase::GetInstance() {
    static DbgService *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new DbgService();
    }
    return s_ptr;
}