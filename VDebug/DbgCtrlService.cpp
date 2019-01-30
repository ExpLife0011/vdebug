#include <Windows.h>
#include <Shlwapi.h>
#include <ComLib/ComLib.h>
#include <runner/runner.h>
#include <DbgCtrl/DbgProtocol.h>
#include "DbgCtrlService.h"
#include "MainView.h"

DbgCtrlService::DbgCtrlService() {
    m_procMon = false;
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
    m_stat = type;;
    return true;
}

DbggerType DbgCtrlService::GetDebuggerStat() {
    return m_stat;
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
    WCHAR image[256];
    GetModuleFileNameW(NULL, image, 256);
    PathAppendW(image, L"..\\x32\\dbg32.exe");

    wstring cmd = FormatW(RUNNER_EVENT32, m_unique.c_str());
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    RunProcInUser(image, cmd.c_str(), session);
#endif
    //Debug Event Register
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MSG, OnDbgMessage, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_CREATE, OnProcCreate, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_SYSTEM_BREAKPOINT, OnSystemBreakpoint, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_END, OnProcExit, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_LOAD, OnModuleLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_UNLOAD, OnModuleUnLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_PROC_CHANGED, OnProcChanged, this);
    return true;
}

bool DbgCtrlService::StartProcMon() {
    m_procMon = true;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_TASK_GET_PROC, "{\"start\":1}");
    return true;
}

void DbgCtrlService::StopProcMon() {
    m_procMon = false;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_TASK_GET_PROC, "{\"start\":0}");
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
    wnsprintfA(szSub, 1024, "%ls\\%ls", PATH_SERVICE_CACHE, szMagic);
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

bool DbgCtrlService::DetachProc() {
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_DETACH, "{}");
    SetCmdNotify(em_dbg_status_init, "初始状态");
    return true;
}

CmdReplyResult DbgCtrlService::RunCmdInCtrlService(const std::mstring &command) {
    CmdRequest request;
    request.mCmdMode = CMD_MASK_SHOW;
    request.mCmd = command;
    mstring reply = m_pCtrlService->DispatchCurDbgger(DBG_CTRL_RUNCMD, MakeCmdRequest(request));
    CmdReplyResult result;
    ParserCmdReply(reply, result);
    return result;
}

void DbgCtrlService::OnProcCreate(const mstring &eventName, const mstring &content, void *param) {
    ProcCreateInfo info = DecodeProcCreate(content);

    PrintFormater pf;
    pf << "进程启动" << space                    << line_end;
    pf << "进程Pid"  << FormatA("%d", info.mPid) << line_end;
    pf << "映像路径" << info.mImage              << line_end;
    pf << "进程基址" << info.mBaseAddr           << line_end;
    pf << "入口地址" << info.mEntryAddr          << line_end;
    AppendToSyntaxView(SCI_LABEL_DEFAULT, pf.GetResult());
}

void DbgCtrlService::OnSystemBreakpoint(const mstring &eventName, const mstring &content, void *param) {
    PrintFormater pf;
    pf << "系统断点触发调试器中断" << line_end;
    AppendToSyntaxView(SCI_LABEL_DEFAULT, pf.GetResult());

    Value json;
    Reader().parse(content, json);
    int tid = json["tid"].asInt();

    SetCmdNotify(em_dbg_status_free, FormatA("线程 %d >>", tid));
}

void DbgCtrlService::OnDbgMessage(const mstring &event, const mstring &content, void *param) {
}

void DbgCtrlService::OnProcExit(const mstring &event, const mstring &content, void *param) {
}

void DbgCtrlService::OnModuleLoad(const mstring &eventName, const mstring &content, void *param) {
    DllLoadInfo dllInfo = DecodeDllLoadInfo(content);

    PrintFormater pf;;
    pf << "模块加载" << dllInfo.mBaseAddr << dllInfo.mEndAddr << dllInfo.mDllName << line_end;
    AppendToSyntaxView(SCI_LABEL_DEFAULT, pf.GetResult());
}

void DbgCtrlService::OnModuleUnLoad(const mstring &event, const mstring &content, void *param) {
}

void DbgCtrlService::OnProcChanged(const mstring &event, const mstring &content, void *param) {
    CProcSelectView *pProcView = GetProcView();
    if (!pProcView)
    {
        return;
    }

    if (!IsWindow(pProcView->GetWndHandle()))
    {
        return;
    }

    ProcInfoSet info = DecodeProcMon(content);
    pProcView->OnProcChanged(info);
}