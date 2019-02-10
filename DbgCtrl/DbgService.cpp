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
    virtual bool InitDbgService(const mstring &unique);
    virtual CtrlReply DispatchCurDbgger(const CtrlRequest &request);
    virtual CtrlReply DispatchSpecDbgger(DbggerType type, const CtrlRequest &request);
    virtual HDbgCtrl RegisterDbgEvent(const std::mstring &dbgEvent, pfnDbgEventProc pfn, void *param);
    virtual bool SetActivity(DbggerType type);

private:
    std::mstring GetSpecChannel(DbggerType type) const;
    bool DispatchEventToRegister(const EventInfo &info) const;
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
        if (0 == lstrcmpA(szChannel, CHANNEL_PROC_SERVER))
        {
            Value json;
            Reader().parse(szContent, json);

            mstring type = json["type"].asString();

            if (type == "event")
            {
                EventInfo eventInfo = ParserEvent(szContent);
                pThis->DispatchEventToRegister(eventInfo);
            }
        } else {
        }
    } while (false);
    return MsgStrCopy(result.c_str());
}

bool DbgService::DispatchEventToRegister(const EventInfo &eventInfo) const {
    map<string, list<DbgServiceCache *>>::const_iterator it = m_RegisterSet.find(eventInfo.mEvent);

    if (it == m_RegisterSet.end())
    {
        return false;
    }

    for (list<DbgServiceCache *>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
    {
        DbgServiceCache *ptr = *ij;
        ptr->m_proc(eventInfo, ptr->m_param);
    }
    return true;
}

bool DbgService::InitDbgService(const mstring &unique) {
    m_unique = unique;
    m_port = CalPortFormUnique(unique);

    MsgInitServ(m_port);
    MsgInitClient(m_port);
    MsgRegister(CHANNEL_PROC_SERVER, ServerNotify, this);
    return true;
}

CtrlReply DbgService::DispatchCurDbgger(const CtrlRequest &request) {
    return DispatchSpecDbgger(m_DbgClient, request);
}

CtrlReply DbgService::DispatchSpecDbgger(DbggerType type, const CtrlRequest &request) {
    mstring channel = GetSpecChannel(type);

    LPCSTR sz = MsgSendForResult(channel.c_str(), MakeRequest(request).c_str());
    mstring result = sz;
    MsgStrFree(sz);
    return ParserReply(result);
}

mstring DbgService::GetSpecChannel(DbggerType type) const {
    mstring channel;
    switch (type) {
        case em_dbg_proc86:
            channel = CHANNEL_RPOC32;
            break;
        case em_dbg_proc64:
            channel = CHANNEL_RPOC64;
            break;
        case em_dbg_dump86:
            channel = CHANNEL_DUMP32;
            break;
        case em_dbg_dump64:
            channel = CHANNEL_DUMP64;
            break;
    }
    return channel;
}

bool DbgService::SetActivity(DbggerType type) {
    m_DbgClient = type;
    m_curChannel = GetSpecChannel(type);
    return true;
}

HDbgCtrl DbgService::RegisterDbgEvent(const mstring &eventName, pfnDbgEventProc pfn, void *param) {
    CScopedLocker lock(this);
    DbgServiceCache *cache = new DbgServiceCache();
    cache->m_idex = m_curIndex++;
    cache->m_event = eventName;
    cache->m_param = param;
    cache->m_proc = pfn;
    m_RegisterSet[eventName].push_back(cache);
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