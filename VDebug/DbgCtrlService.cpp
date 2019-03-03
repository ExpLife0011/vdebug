#include <Windows.h>
#include <Shlwapi.h>
#include <ComLib/ComLib.h>
#include <runner/runner.h>
#include <DbgCtrl/DbgProtocol.h>
#include "DbgCtrlService.h"
#include "MainView.h"

DbgCtrlService::DbgCtrlService() {
    m_procMon = false;
    m_stat = em_dbg_status_init;
}

DbgCtrlService::~DbgCtrlService() {
}

DbgCtrlService *DbgCtrlService::GetInstance() {
    static DbgCtrlService *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new DbgCtrlService();
    }
    return s_ptr;
}

bool DbgCtrlService::SetDebugger(DbggerType type) {
    m_pCtrlService->SetActivity(type);
    m_type = type;;
    return true;
}

DbggerStatus DbgCtrlService::GetDebuggerStat() {
    return m_stat;
}

bool DbgCtrlService::BreakDbgProcInCtrlService() const {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_BREAK;
    m_pCtrlService->DispatchCurDbgger(ctrl);
    return true;
}

void DbgCtrlService::DbgStatusNotifyProc(const DbgStat &status, void *param) {
    int ddd = 123;
    GetInstance()->m_stat = status.mDbggerStatus;
    if (status.mDbggerStatus == em_dbg_status_init) {
        SetCmdNotify(em_dbg_status_init, "初始状态");
    } else if (status.mDbggerStatus == em_dbg_status_free) {
        SetCmdNotify(em_dbg_status_free, FormatA("线程 %d >>", status.mCurTid));
    } else if (status.mDbggerStatus == em_dbg_status_busy) {
        SetCmdNotify(em_dbg_status_busy, "运行中");
    }
}

bool DbgCtrlService::InitCtrlService() {
#ifdef _DEBUG
    m_unique = UNIQUE_DEBUG;
#else
    srand(GetTickCount());
    m_unique = FormatA(
        "%04x-%04x%04x-%04x",
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff
        );
#endif

    m_pCtrlService = DbgServiceBase::GetInstance();
    m_pCtrlService->InitDbgService(m_unique.c_str());
    CDbgStatMgr::GetInst()->InitStatMgr(m_unique);
    CDbgStatMgr::GetInst()->RegisterStatusNotify(DbgStatusNotifyProc, NULL);
    CreateEventA(NULL, FALSE, FALSE, FormatA(SERVICE_EVENT, m_unique.c_str()).c_str());

#ifdef _DEBUG
#else
    char image[256];
    GetModuleFileNameA(NULL, image, 256);
    PathAppendA(image, "..\\x32\\dbg32.exe");

    mstring cmd = FormatA(RUNNER_EVENT32, m_unique.c_str());
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    RunProcInUser(image, cmd.c_str(), session);
#endif
    //Debug Event Register
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MSG, OnDbgMessage, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_CREATE, OnProcCreate, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_SYSTEM_BREAKPOINT, OnSystemBreakpoint, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_USER_BREAKPOINT, OnUserBreakpoint, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_END, OnProcExit, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_LOAD, OnModuleLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_UNLOAD, OnModuleUnLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_PROC_CHANGED, OnProcChanged, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DETACH, OnDetachDbgger, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_EXCEPTION, OnProgramException, this);
    return true;
}

bool DbgCtrlService::StartProcMon() {
    m_procMon = true;
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_GET_PROC;
    ctrl.mContent["start"] = 1;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, ctrl);
    return true;
}

void DbgCtrlService::StopProcMon() {
    m_procMon = false;
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_GET_PROC;
    ctrl.mContent["start"] = 0;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, ctrl);
    return;
}

void DbgCtrlService::RunProcInUser(LPCSTR image, LPCSTR cmd, DWORD session) {
    if (!image || !*image)
    {
        return;
    }

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(image))
    {
        return;
    }

    static DWORD s_dwSerial = 0xffff;
    srand(GetTickCount());
    char szSub[1024] = {0};
    char szMagic[32] = {0};
    wnsprintfA(szMagic, 32, "%04X%04X%08X", rand(), rand(), s_dwSerial++);
    wnsprintfA(szSub, 1024, "%hs\\%hs", PATH_SERVICE_CACHE, szMagic);
    SHSetValueA(
        HKEY_LOCAL_MACHINE,
        szSub,
        "image",
        REG_SZ,
        image,
        (lstrlenA(image) + 1) * sizeof(CHAR)
        );
    if (cmd)
    {
        SHSetValueA(
            HKEY_LOCAL_MACHINE,
            szSub,
            "cmd",
            REG_SZ,
            cmd,
            (lstrlenA(cmd) + 1) * sizeof(CHAR)
            );
    }

    SHSetValueA(
        HKEY_LOCAL_MACHINE,
        szSub,
        "sessionId",
        REG_DWORD,
        &session,
        sizeof(session)
        );

    DWORD dwShell = 0;
    HWND hShell = FindWindowA("Shell_TrayWnd", NULL);
    GetWindowThreadProcessId(hShell, &dwShell);

    if (dwShell)
    {
        SHSetValueA(HKEY_LOCAL_MACHINE, szSub, "shell", REG_DWORD, &dwShell, sizeof(dwShell));
    }

    HANDLE hNotify = OpenEventA(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
    if (hNotify)
    {
        SetEvent(hNotify);
        CloseHandle(hNotify);
    }
    return;
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
bool DbgCtrlService::ExecProc(const std::mstring &path, const std::mstring &param) {
    Value content;
    content["path"] = path;
    content["param"] = param;

    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_EXEC;
    ctrl.mContent = content;
    m_pCtrlService->DispatchCurDbgger(ctrl);
    return true;
}

bool DbgCtrlService::AttachProc(DWORD pid) {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_ATTACH;
    ctrl.mContent["pid"] = (int)pid;
    m_pCtrlService->DispatchCurDbgger(ctrl);
    return true;
}

bool DbgCtrlService::DetachProc() {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_DETACH;
    m_pCtrlService->DispatchCurDbgger(ctrl);
    return true;
}

bool DbgCtrlService::TestDescStr(const mstring &dll, const mstring &str) {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_TEST_DESC;
    ctrl.mContent["module"] = dll;
    ctrl.mContent["descStr"] = str;

    CtrlReply d = m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, ctrl);
    SetFunViewStat(d.mShow);
    return true;
}

bool DbgCtrlService::InputDescStr(const mstring &dll, const mstring &str) {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_INPUT_DESC;
    ctrl.mContent["module"] = dll;
    ctrl.mContent["descStr"] = str;

    CtrlReply d = m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, ctrl);
    SetFunViewStat(d.mShow);
    return true;
}

bool DbgCtrlService::OpenDump(const std::mstring &path) const {
    CtrlRequest ctrl;
    ctrl.mCmd = DBG_CTRL_OPEN_DUMP;
    ctrl.mContent["dumpPath"] = path;
    CtrlReply d  = m_pCtrlService->DispatchCurDbgger(ctrl);

    AppendToSyntaxView(d.mLabel, d.mShow);

    int tid = d.mResult["tid"].asInt();
    return (0 == d.mStatus);
}

CtrlReply DbgCtrlService::RunCmdInCtrlService(const std::mstring &command) {
    mstring tmp = command;
    tmp.trim();
    tmp.makelower();

    if (tmp == "cls")
    {
        GetSyntaxView()->ClearView();
        return CtrlReply();
    } else if (tmp == "pi")
    {
    }

    CtrlRequest request;
    request.mCmd = DBG_CTRL_RUNCMD;
    request.mContent["command"] = command;
    return m_pCtrlService->DispatchCurDbgger(request);
}

void DbgCtrlService::OnProcCreate(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(SCI_LABEL_DEFAULT, eventInfo.mShow);
}

void DbgCtrlService::OnSystemBreakpoint(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mLabel, eventInfo.mShow);
}

void DbgCtrlService::OnUserBreakpoint(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mLabel, eventInfo.mShow);

    CtrlReply result = GetInstance()->RunCmdInCtrlService("r");
    AppendToSyntaxView(result.mLabel, result.mShow);
}

void DbgCtrlService::OnDbgMessage(const EventInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnProcExit(const EventInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnModuleLoad(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mLabel, eventInfo.mShow);
}

void DbgCtrlService::OnModuleUnLoad(const EventInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnDetachDbgger(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mLabel, eventInfo.mShow);
}

void DbgCtrlService::OnProgramException(const EventInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mLabel, eventInfo.mShow);
}

void DbgCtrlService::OnProcChanged(const EventInfo &eventInfo, void *param) {
    CProcSelectView *pProcView = GetProcView();
    if (!pProcView)
    {
        return;
    }

    if (!IsWindow(pProcView->GetWndHandle()))
    {
        return;
    }

    ProcInfoSet info = DecodeProcMon(FastWriter().write(eventInfo.mContent));
    pProcView->OnProcChanged(info);
}