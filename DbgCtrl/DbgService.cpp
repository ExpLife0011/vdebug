#include "DbgService.h"
#include <string>
#include "DbgCtrlTool.h"
#include <mq/mq.h>
#include <map>
#include <list>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

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
    virtual const wchar_t *DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content);
    virtual HDbgCtrl RegisterDbgEvent(const wchar_t *event, pfnDbgEventProc pfn, void *param);
    virtual bool SetActivity(DbggerType type);

private:
    bool DispatchToRegister(const wstring &cmd, const wstring &content) const;
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

            if (!root || root->type != cJSON_Object)
            {
                break;
            }

            cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
            cJSON *content = cJSON_GetObjectItem(root, "content");

            pThis->DispatchToRegister(UtoW(cmd->valuestring), UtoW(content->valuestring));
        } else {
        }
    } while (false);

    if (root)
    {
        cJSON_Delete(root);
    }
    return MsgStrCopy(result.c_str());
}

bool DbgService::DispatchToRegister(const wstring &cmd, const wstring &content) const {
    map<wstring, list<DbgServiceCache *>>::const_iterator it = m_RegisterSet.find(cmd);

    if (it == m_RegisterSet.end())
    {
        return false;
    }

    for (list<DbgServiceCache *>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
    {
        DbgServiceCache *ptr = *ij;
        ptr->m_proc(cmd.c_str(), content.c_str(), ptr->m_param);
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

const wchar_t *DbgService::DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "cmd", WtoU(cmd).c_str());
    cJSON_AddStringToObject(root, "content", WtoU(content).c_str());
    LPCWSTR wsz = MsgSendForResult(m_curChannel.c_str(), UtoW(cJSON_PrintUnformatted(root)).c_str());
    cJSON_Delete(root);
    return wsz;
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