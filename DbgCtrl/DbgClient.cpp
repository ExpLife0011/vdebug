#include <Windows.h>
#include <map>
#include <list>
#include "DbgClient.h"
#include "DbgCtrlTool.h"
#include <mq/mq.h>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

using namespace std;
using namespace Json;

struct DbgClientCache {
    string m_CtrlCode;
    void *m_param;
    pfnDbgClientProc m_proc;
    int m_idex;
};

class DbgClient : public DbgClientBase, public CCriticalSectionLockable {
public:
    DbgClient();
    virtual ~DbgClient();
    virtual bool InitClient(DbggerType type, const mstring &unique);
    virtual HDbgCtrl RegisterCtrlHandler(const std::mstring &cmd, pfnDbgClientProc pfn, void *param);
    virtual bool ReportDbgEvent(const EventInfo &eventInfo);

private:
    static LPCSTR WINAPI ClientNotify(LPCSTR szChannel, LPCSTR szContent, void *pParam);

private:
    bool m_init;
    DbggerType m_type;
    mstring m_unique;
    unsigned short m_ServPort;
    long m_curIndex;
    map<mstring, list<DbgClientCache *>> m_RegisterSet;
};

DbgClientBase *DbgClientBase::newInstance() {
    return new DbgClient();
}

DbgClient::DbgClient() :
m_init(false),
m_type(em_dbg_proc86),
m_ServPort(6010),
m_curIndex(0xffab){
}

DbgClient ::~DbgClient() {
}

bool DbgClient::InitClient(DbggerType type, const mstring &unique) {
    if (m_init)
    {
        return true;
    }

    m_unique = unique;
    m_ServPort = CalPortFormUnique(unique);
    m_type = type;

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

    MsgInitClient(m_ServPort);
    MsgRegister(channel.c_str(), ClientNotify, this);
    return true;
}

bool DbgClient::ReportDbgEvent(const EventInfo &eventInfo) {
    MsgSend(CHANNEL_PROC_SERVER, MakeEvent(eventInfo).c_str());
    return true;
}

HDbgCtrl DbgClient::RegisterCtrlHandler(const mstring &cmd, pfnDbgClientProc pfn, void *param) {
    CScopedLocker lock(this);
    DbgClientCache *cache = new DbgClientCache();
    cache->m_CtrlCode = cmd;
    cache->m_proc = pfn;
    cache->m_param = param;
    cache->m_idex = m_curIndex++;
    m_RegisterSet[cmd].push_back(cache);
    return cache->m_idex ;
}

LPCSTR DbgClient::ClientNotify(LPCSTR szChannel, LPCSTR szContent, void *pParam) {
    DbgClient *pThis = (DbgClient *)pParam;

    Value json;
    Reader().parse(szContent, json);
    string result;

    do
    {
        if (json.type() != objectValue)
        {
            break;
        }

        mstring type = json["type"].asString();

        if (type == "ctrl")
        {
            CtrlRequest request = ParserRequest(szContent);
            CScopedLocker lock(pThis);
            map<mstring, list<DbgClientCache *>>::const_iterator it;
            it = pThis->m_RegisterSet.find(request.mCmd);

            if (it == pThis->m_RegisterSet.end())
            {
                break;
            }

            const list<DbgClientCache *> &tmp = it->second;
            for (list<DbgClientCache *>::const_iterator ij = tmp.begin() ; ij != tmp.end() ; ij++)
            {
                DbgClientCache *ptr = *ij;
                CtrlReply reply = ptr->m_proc(request, ptr->m_param);
                result = MakeReply(reply);
            }
        }
    } while (FALSE);
    return MsgStrCopy(result.c_str());
}