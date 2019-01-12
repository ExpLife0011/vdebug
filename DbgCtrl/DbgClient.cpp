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
    wstring m_CtrlCode;
    void *m_param;
    pfnDbgClientProc m_proc;
    int m_idex;
};

class DbgClient : public DbgClientBase, public CCriticalSectionLockable {
public:
    DbgClient();
    virtual ~DbgClient();
    virtual bool InitClient(DbggerType type, const wchar_t *unique);
    virtual HDbgCtrl RegisterCtrlHandler(const wchar_t *cmd, pfnDbgClientProc pfn, void *param);
    virtual bool ReportDbgEvent(const std::utf8_mstring &content);

private:
    static LPCWSTR WINAPI ClientNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam);

private:
    bool m_init;
    DbggerType m_type;
    wstring m_unique;
    unsigned short m_ServPort;
    long m_curIndex;
    map<wstring, list<DbgClientCache *>> m_RegisterSet;
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

bool DbgClient::InitClient(DbggerType type, const wchar_t *unique) {
    if (m_init)
    {
        return true;
    }

    m_unique = unique;
    m_ServPort = CalPortFormUnique(unique);
    m_type = type;

    wstring channel;
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

    MsgInitClient(m_ServPort);
    MsgRegister(channel.c_str(), ClientNotify, this);
    return true;
}

bool DbgClient::ReportDbgEvent(const std::utf8_mstring &content) {
    MsgSend(MQ_CHANNEL_DBG_SERVER, UtoW(content).c_str());
    return true;
}

HDbgCtrl DbgClient::RegisterCtrlHandler(const wchar_t *cmd, pfnDbgClientProc pfn, void *param) {
    CScopedLocker lock(this);
    DbgClientCache *cache = new DbgClientCache();
    cache->m_CtrlCode = cmd;
    cache->m_proc = pfn;
    cache->m_param = param;
    cache->m_idex = m_curIndex++;
    m_RegisterSet[cmd].push_back(cache);
    return cache->m_idex ;
}

LPCWSTR DbgClient::ClientNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam) {
    DbgClient *pThis = (DbgClient *)pParam;

    Value json;
    Reader().parse(WtoU(wszContent), json);
    wstring result;

    do
    {
        if (json.type() != objectValue)
        {
            break;
        }

        ustring cmd = UtoW(json["cmd"].asString());
        ustring content = UtoW(json["content"].asString());

        {
            CScopedLocker lock(pThis);
            map<wstring, list<DbgClientCache *>>::const_iterator it;
            it = pThis->m_RegisterSet.find(cmd);

            if (it == pThis->m_RegisterSet.end())
            {
                break;
            }

            const list<DbgClientCache *> &tmp = it->second;
            for (list<DbgClientCache *>::const_iterator ij = tmp.begin() ; ij != tmp.end() ; ij++)
            {
                DbgClientCache *ptr = *ij;
                ustring p = ptr->m_proc(cmd, content, ptr->m_param);
                result = p;
            }
        }
    } while (FALSE);
    return MsgStrCopy(result.c_str());
}