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

bool ProcDbgProxy::InitProcDbgProxy(const char *unique) {
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
    m_pDbgClient->RegisterCtrlHandler(DBG_CTRL_DETACH, DetachProc, this);
    m_pProcDbgger = CProcDbgger::GetInstance();
    return true;
}

ProcDbgProxy::~ProcDbgProxy() {
}

/*
{
    "cmd":"RunCmd",
        "content":{
            "cmd":"bp kernen32!CreateFileW"
    }
}
*/
mstring ProcDbgProxy::RunCmd(const mstring &cmd, const mstring &content, void *param) {
    Value json;
    Reader().parse(content, json);

    mstring data = json["cmd"].asString();
    data.trim();

    CmdReplyResult reply;
    if (data.empty())
    {
        reply.mCmdCode = DBG_CMD_SYNTAX_ERR;
        reply.mCmdShow = "ÃüÁîÓï·¨´íÎó";
        return MakeCmdReply(reply);
    }

    CmdRequest request = ParserCmdRequest(content);
    CmdReplyResult result = GetInstance()->m_pProcDbgger->RunCommand(request);
    return MakeCmdReply(result);
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
mstring ProcDbgProxy::ExecProc(const std::mstring &cmd, const std::mstring &content, void *param) {
    Value root;
    Reader().parse(content, root);

    mstring path = root["path"].asString();
    mstring exeParam = root["param"].asString();

    DbgProcUserContext context;
    context.m_strCmd = exeParam;
    context.m_strPePath = path;
    BOOL ret = GetInstance()->m_pProcDbgger->Connect(path.c_str(), &context);

    mstring res;
    if (ret)
    {
        res = MakeDbgRelpy(DbgReplyResult(0, "exec success", ""));
    } else {
        res = MakeDbgRelpy(DbgReplyResult(GetLastError(), "exec error", ""));
    }
    return res;
}

mstring ProcDbgProxy::AttachProc(const std::mstring &cmd, const std::mstring &content, void *param) {
    return MakeDbgRelpy(DbgReplyResult(0, "success", ""));
}

mstring ProcDbgProxy::DetachProc(const std::mstring &cmd, const std::mstring &content, void *param) {
    if (GetInstance()->m_pProcDbgger->GetDbggerStatus() != em_dbg_status_init)
    {
        GetInstance()->m_pProcDbgger->DisConnect();
    }

    return MakeDbgRelpy(DbgReplyResult(0, "success", ""));
}

mstring ProcDbgProxy::GetProcInfo(const mstring &cmd, const mstring &content, void *param) {
    Value root;
    Reader().parse(content, root);

    int start = GetIntFromJson(root, "start");
    if (1 == start)
    {
        if (GetInstance()->m_hProcListener != NULL)
        {
            ProcMonitor::GetInstance()->UnRegisterListener(GetInstance()->m_hProcListener);
            GetInstance()->m_hProcListener = NULL;
        }

        GetInstance()->m_hProcListener = ProcMonitor::GetInstance()->RegisterListener(GetInstance());
    } else {
        ProcMonitor::GetInstance()->UnRegisterListener(GetInstance()->m_hProcListener);
        GetInstance()->m_hProcListener = NULL;
    }
    return MakeDbgRelpy(DbgReplyResult(0, "success", ""));
}

void ProcDbgProxy::OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed) {
    ProcInfoSet procSet;
    for (list<const ProcMonInfo *>::const_iterator it = added.begin() ; it != added.end() ; it++)
    {
        procSet.mAddSet.push_back(**it);
    }
    procSet.mKillSet = killed;

    mstring packet = MakeDbgEvent(DBG_EVENT_PROC_CHANGEDA, EncodeProcMon(procSet));
    m_pDbgClient->ReportDbgEvent(packet);
}