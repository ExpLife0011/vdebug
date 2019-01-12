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
    Value root;
    Reader().parse(WtoU(content), root);

    wstring path = UtoW(root["path"].asString());
    wstring exeParam = UtoW(root["param"].asString());

    DbgProcUserContext context;
    context.m_wstrCmd = exeParam;
    context.m_wstrPePath = path;
    BOOL ret = GetInstance()->m_pProcDbgger->Connect(path.c_str(), &context);

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
    Value root;
    Reader().parse(WtoU(content), root);

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
    return MakeDbgRelpy(0, "success", "");
}

void ProcDbgProxy::OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed) {
    ProcInfoSet procSet;
    for (list<const ProcMonInfo *>::const_iterator it = added.begin() ; it != added.end() ; it++)
    {
        procSet.mAddSet.push_back(**it);
    }
    procSet.mKillSet = killed;

    utf8_mstring packet = MakeDbgEvent(DBG_EVENT_PROC_CHANGED, EncodeProcMon(procSet));
    m_pDbgClient->ReportDbgEvent(packet);
}