#include "DbgService.h"
#include <string>
#include "DbgCtrlTool.h"
#include <mq/mq.h>

using namespace std;

class DbgService : public DbgServiceBase {
public:
    virtual ~DbgService();
    virtual bool InitDbgService(const wchar_t *unique);
    virtual bool DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content);
    virtual bool SetActivity(DbggerType type);

private:
    static LPCWSTR WINAPI ServerNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam);

private:
    wstring m_unique;
    unsigned short m_port;
};

DbgService::~DbgService() {
}

LPCWSTR DbgService::ServerNotify(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam) {
    return NULL;
}

bool DbgService::InitDbgService(const wchar_t *unique) {
    m_unique = unique;
    m_port = CalPortFormUnique(unique);

    MsgInitServ();
    MsgRegister(MQ_CHANNEL_DBG_SERVER, ServerNotify, NULL);
    return true;
}

bool DbgService::DispatchCurDbgger(const wchar_t *cmd, const wchar_t *content) {
    return true;
}

bool DbgService::SetActivity(DbggerType type) {
    return true;
}

DbgServiceBase *DbgServiceBase::GetInstance() {
    static DbgService *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new DbgService();
    }
    return s_ptr;
}