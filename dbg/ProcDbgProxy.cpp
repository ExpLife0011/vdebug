#include "ProcDbgProxy.h"
#include <DbgCtrl/DbgCtrl.h>
#include <DbgCtrl/DbgProtocol.h>
#include "ProcDbg.h"

using namespace std;

ProcDbgProxy *ProcDbgProxy::GetInstance() {
    static ProcDbgProxy *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new ProcDbgProxy();
    }
    return s_ptr;
}

ProcDbgProxy::ProcDbgProxy() {
    m_init = false;
    m_pDbgClient = NULL;
    m_x64 = (sizeof((void *)0) == 8);
    m_pProcDbgger = NULL;
}

bool ProcDbgProxy::InitProcDbgProxy(const wchar_t *unique) {
    if (m_init)
    {
        return true;
    }

    m_init = true;
    m_pDbgClient = DbgClientBase::newInstance();
    if (m_x64) {
        m_pDbgClient->InitClient(em_dbg_proc64, unique);
    }
    else {
        m_pDbgClient->InitClient(em_dbg_proc86, unique);
    }
    m_pDbgClient->RegisterCtrlHandler(DBG_CTRL_EXEC, ExecProc, this);
    m_pDbgClient->RegisterCtrlHandler(DBG_CTRL_ATTACH, AttachProc, this);
    m_pDbgClient->RegisterCtrlHandler(DBG_CTRL_RUNCMD, RunCmd, this);
    m_pProcDbgger = CProcDbgger::GetInstance();
    return true;
}

ProcDbgProxy::~ProcDbgProxy() {
}

ustring ProcDbgProxy::RunCmd(const std::ustring &cmd, const std::ustring &content, void *para) {
    return L"";
}

/*
{
    "cmd":"exec",
    "content":{
        "path":"abcdef.exe",
        "param":"abcdefgh"
    }
}
*/
ustring ProcDbgProxy::ExecProc(const std::ustring &cmd, const std::ustring &content, void *para) {
    cJSON *root = cJSON_Parse(WtoU(content).c_str());
    wstring path = UtoW(cJSON_GetObjectItem(root, "path")->valuestring);
    wstring exeParam;
    cJSON * tmp = cJSON_GetObjectItem(root, "param");
    if (tmp && tmp->type == cJSON_String)
    {
        exeParam = UtoW(tmp->valuestring);
    }

    DbgProcUserContext context;
    context.m_wstrCmd = exeParam;
    context.m_wstrPePath = path;
    BOOL ret = GetInstance()->m_pProcDbgger->Connect(path.c_str(), &context);
    cJSON_Delete(root);

    string res;
    if (ret)
    {
        res = MakeDbgRelpy(0, "exec success", "");
    } else {
        res = MakeDbgRelpy(GetLastError(), "exec error", "");
    }
    return UtoW(res);
}

ustring ProcDbgProxy::AttachProc(const std::ustring &cmd, const std::ustring &content, void *para) {
    return L"";
}