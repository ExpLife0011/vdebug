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

struct DbgServiceCache {
    wstring m_event;
    void *m_param;
    pfnDbgEventProc m_proc;
    int m_idex;
};

class DbgService : public DbgServiceBase, CCriticalSectionLockable {
public:
    DbgService();
    virtual ~DbgService();
    virtual bool InitDbgService(const wchar_t *unique);
    virtual std::ustring DispatchCurDbgger(const std::ustring &cmd, const std::ustring &content);
    virtual HDbgCtrl RegisterDbgEvent(const wchar_t *event, pfnDbgEventProc pfn, void *param);
    virtual bool SetActivity(DbggerType type);

private:
    bool DispatchEventToRegister(const utf8_mstring &cmd, const utf8_mstring &content) const;
    static LPCWSTR WINAPI ServerNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam);

private:
    map<wstring, list<DbgServiceCache *>> m_RegisterSet;
    wstring m_unique;
    unsigned short m_port;
    long m_curIndex;
    wstring m_curChannel;
};

DbgService::DbgService() :m_port(0), m_curIndex(0xffea){
}

DbgService::~DbgService() {
}

LPCWSTR DbgService::ServerNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam) {
    cJSON *root = NULL;
    wstring result;
    DbgService *pThis = (DbgService *)pParam;

    do 
    {
        if (0 == lstrcmpW(wszChannel, MQ_CHANNEL_DBG_SERVER))
        {
            cJSON *root = cJSON_Parse(WtoU(wszContent).c_str());
            JsonAutoDelete tmp(root);

            if (!root || root->type != cJSON_Object)
            {
                break;
            }

            utf8_mstring cmd = GetStrFormJson(root, "cmd");
            cJSON *content = cJSON_GetObjectItem(root, "content");

            if (cmd == WtoU(DBG_DBG_EVENT))
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
                utf8_mstring event = GetStrFormJson(content, "type");
                utf8_mstring data = GetStrFormJson(content, "data");
                pThis->DispatchEventToRegister(event, data);
            }
        } else {
        }
    } while (false);

    if (root)
    {
        cJSON_Delete(root);
    }
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
bool DbgService::DispatchEventToRegister(const utf8_mstring &cmd, const utf8_mstring &content) const {
    map<wstring, list<DbgServiceCache *>>::const_iterator it = m_RegisterSet.find(UtoW(cmd));

    if (it == m_RegisterSet.end())
    {
        return false;
    }

    for (list<DbgServiceCache *>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
    {
        DbgServiceCache *ptr = *ij;
        ptr->m_proc(UtoW(cmd), UtoW(content), ptr->m_param);
    }
    return true;
}

bool DbgService::InitDbgService(const wchar_t *unique) {
    m_unique = unique;
    m_port = CalPortFormUnique(unique);

    MsgInitServ(m_port);
    MsgInitClient(m_port);
    MsgRegister(MQ_CHANNEL_DBG_SERVER, ServerNotify, this);
    return true;
}

ustring DbgService::DispatchCurDbgger(const ustring &cmd, const ustring &content) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "cmd", WtoU(cmd).c_str());
    cJSON_AddStringToObject(root, "content", WtoU(content).c_str());
    LPCWSTR wsz = MsgSendForResult(m_curChannel.c_str(), UtoW(cJSON_PrintUnformatted(root)).c_str());
    cJSON_Delete(root);
    ustring result = wsz;
    MsgStrFree(wsz);
    return result;
}

bool DbgService::SetActivity(DbggerType type) {
    switch (type) {
        case em_dbg_proc86:
            m_curChannel = MQ_CHANNEL_DBG_CLIENT32;
            break;
        case em_dbg_proc64:
            m_curChannel = MQ_CHANNEL_DBG_CLIENT64;
            break;
        case em_dbg_dump86:
            m_curChannel = MQ_CHANNEL_DBG_DUMP32;
            break;
        case em_dbg_dump64:
            m_curChannel = MQ_CHANNEL_DBG_DUMP64;
            break;
    }
    return true;
}

HDbgCtrl DbgService::RegisterDbgEvent(const wchar_t *event, pfnDbgEventProc pfn, void *param) {
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