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
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_BREAK, "{}");
    return true;
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
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_RUNNING, OnDbgProcRunning, this);
    return true;
}

bool DbgCtrlService::StartProcMon() {
    m_procMon = true;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_CTRL_GET_PROC, "{\"start\":1}");
    return true;
}

void DbgCtrlService::StopProcMon() {
    m_procMon = false;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_CTRL_GET_PROC, "{\"start\":0}");
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

void DbgCtrlService::SetCtrlStatus(DbggerStatus stat) {
    SetCmdNotify(stat, "abcdef");
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

    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_EXEC, FastWriter().write(content));
    return true;
}

bool DbgCtrlService::AttachProc(DWORD pid) {
    Value content;
    content["pid"] = (int)pid;
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_ATTACH, FastWriter().write(content));
    return true;
}

bool DbgCtrlService::DetachProc() {
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_DETACH, "{}");
    SetCmdNotify(em_dbg_status_init, "初始状态");
    return true;
}

CmdReplyResult DbgCtrlService::RunCmdInCtrlService(const std::mstring &command) {
    mstring tmp = command;
    tmp.trim();
    tmp.makelower();

    if (tmp == "cls")
    {
        GetSyntaxView()->ClearView();
        return CmdReplyResult();
    }

    CmdRequest request;
    request.mCmdMode = CMD_MASK_SHOW;
    request.mCmd = command;
    mstring reply = m_pCtrlService->DispatchCurDbgger(DBG_CTRL_RUNCMD, MakeCmdRequest(request));
    CmdReplyResult result;
    ParserCmdReply(reply, result);
    return result;
}

void DbgCtrlService::OnProcCreate(const EventDbgInfo &eventInfo, void *param) {
    AppendToSyntaxView(SCI_LABEL_DEFAULT, eventInfo.mEventShow);
}

void DbgCtrlService::OnSystemBreakpoint(const EventDbgInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mEventLabel, eventInfo.mEventShow);

    int tid = eventInfo.mEventResult["tid"].asInt();
    GetInstance()->m_stat = em_dbg_status_free;
    SetCmdNotify(GetInstance()->m_stat, FormatA("线程 %d >>", tid));
}

void DbgCtrlService::OnUserBreakpoint(const EventDbgInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mEventLabel, eventInfo.mEventShow);

    int tid = eventInfo.mEventResult["tid"].asInt();
    GetInstance()->m_stat = em_dbg_status_free;
    SetCmdNotify(GetInstance()->m_stat, FormatA("线程 %d >>", tid));
    CmdReplyResult result = GetInstance()->RunCmdInCtrlService("r");
    AppendToSyntaxView(result.mCmdLabel, result.mCmdShow);
}

void DbgCtrlService::OnDbgMessage(const EventDbgInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnProcExit(const EventDbgInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnModuleLoad(const EventDbgInfo &eventInfo, void *param) {
    AppendToSyntaxView(eventInfo.mEventLabel, eventInfo.mEventShow);
}

void DbgCtrlService::OnModuleUnLoad(const EventDbgInfo &eventInfo, void *param) {
}

void DbgCtrlService::OnDbgProcRunning(const EventDbgInfo &eventInfo, void *param) {
    GetInstance()->m_stat = em_dbg_status_busy;
    SetCmdNotify(GetInstance()->m_stat, "运行中");
}

void DbgCtrlService::OnProcChanged(const EventDbgInfo &eventInfo, void *param) {
    CProcSelectView *pProcView = GetProcView();
    if (!pProcView)
    {
        return;
    }

    if (!IsWindow(pProcView->GetWndHandle()))
    {
        return;
    }

    ProcInfoSet info = DecodeProcMon(FastWriter().write(eventInfo.mEventResult));
    pProcView->OnProcChanged(info);
}