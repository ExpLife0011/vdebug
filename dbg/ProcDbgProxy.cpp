#include <Windows.h>
#include <DbgCtrl/DbgCtrl.h>
#include <DbgCtrl/DbgProtocol.h>
#include <mq/mq.h>
#include "ProcDbgProxy.h"
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
    m_hProcListener = NULL;
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
    m_pDbgClient->RegisterCtrlHandler(DBG_TASK_GET_PROC, GetProcInfo, this);
    m_pProcDbgger = CProcDbgger::GetInstance();
    return true;
}

ProcDbgProxy::~ProcDbgProxy() {
}

ustring ProcDbgProxy::RunCmd(const ustring &cmd, const ustring &content, void *para) {
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

ustring ProcDbgProxy::GetProcInfo(const ustring &cmd, const ustring &content, void *param) {
    cJSON *root = cJSON_Parse(WtoU(content).c_str());
    JsonAutoDelete abc(root);

    int start = GetIntFromJson(root, "start");
    if (1 == start)
    {
        GetInstance()->m_hProcListener = ProcMonitor::GetInstance()->RegisterListener(GetInstance());
    } else {
        ProcMonitor::GetInstance()->UnRegisterListener(GetInstance()->m_hProcListener);
    }
    return MakeDbgRelpy(0, "success", "");
}

/*
{
    "cmd":"event",
    "content":{
        "type":"proc_add",
        "data":{
            "add":[
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
            "kill":[
                1111,2222,3333
            ]
        }
    }
}
*/
void ProcDbgProxy::OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed) {
    cJSON *data = cJSON_CreateObject();
    cJSON *add = cJSON_CreateArray();
    cJSON *kill = cJSON_CreateArray();
    JsonAutoDelete abc(data);
    for (list<const ProcMonInfo *>::const_iterator it = added.begin() ; it != added.end() ; it++)
    {
        const ProcMonInfo *ptr = *it;
        cJSON *node = cJSON_CreateObject();
        cJSON_AddNumberToObject(node, "unique", ptr->procUnique);
        cJSON_AddNumberToObject(node, "pid", ptr->procPid);
        cJSON_AddStringToObject(node, "procPath", WtoU(ptr->procPath).c_str());
        cJSON_AddStringToObject(node, "procDesc", WtoU(ptr->procDesc).c_str());
        cJSON_AddStringToObject(node, "cmd", WtoU(ptr->procCmd).c_str());
        cJSON_AddStringToObject(node, "startTime", WtoU(ptr->startTime).c_str());
        cJSON_AddNumberToObject(node, "x64", int(ptr->x64));
        cJSON_AddNumberToObject(node, "session", ptr->sessionId);
        cJSON_AddStringToObject(node, "user", WtoU(ptr->procUser).c_str());
        cJSON_AddStringToObject(node, "sid", WtoU(ptr->procUserSid).c_str());
        cJSON_AddItemToArray(add, node);
    }
    cJSON_AddItemToObject(data, "add", add);

    for (list<DWORD>::const_iterator ij = killed.begin() ; ij != killed.end() ; ij++)
    {
        cJSON_AddItemToArray(kill, cJSON_CreateNumber(*ij));
    }
    cJSON_AddItemToObject(data, "kill", kill);
    utf8_mstring packet = MakeDbgEvent(DBG_EVENT_PROC_CHANGED, cJSON_PrintUnformatted(data));
    m_pDbgClient->ReportDbgEvent(packet);
}