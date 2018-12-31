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

bool DbgCtrlService::SetDebuggerStat(DbggerType type) {
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
    m_unique = FormatW(
        L"%04x-%04x%04x-%04x",
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff
        );
#endif

    m_pCtrlService = DbgServiceBase::GetInstance();
    m_pCtrlService->InitDbgService(m_unique.c_str());
    CreateEventW(NULL, FALSE, FALSE, FormatW(SERVICE_EVENT, m_unique.c_str()).c_str());

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
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_DBG_PROC_END, OnProcExit, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_LOAD, OnModuleLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_MODULE_UNLOAD, OnModuleUnLoad, this);
    m_pCtrlService->RegisterDbgEvent(DBG_EVENT_PROC_CHANGED, OnProcChanged, this);
    StopProcMon();
    return true;
}

bool DbgCtrlService::StartProcMon() {
    m_procMon = true;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_TASK_GET_PROC, L"{\"start\":1}");
    return true;
}

void DbgCtrlService::StopProcMon() {
    m_procMon = false;
    m_pCtrlService->DispatchSpecDbgger(em_dbg_proc86, DBG_TASK_GET_PROC, L"{\"start\":0}");
    return;
}

void DbgCtrlService::RunProcInUser(LPCWSTR image, LPCWSTR cmd, DWORD session) {
    if (!image || !*image)
    {
        return;
    }

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(image))
    {
        return;
    }

    static DWORD s_dwSerial = 0xffff;
    srand(GetTickCount());
    WCHAR wszSub[1024] = {0};
    WCHAR wszMagic[32] = {0};
    wnsprintfW(wszMagic, 32, L"%04X%04X%08X", rand(), rand(), s_dwSerial++);
    wnsprintfW(wszSub, 1024, L"%ls\\%ls", PATH_SERVICE_CACHE, wszMagic);
    SHSetValueW(
        HKEY_LOCAL_MACHINE,
        wszSub,
        L"image",
        REG_SZ,
        image,
        (lstrlenW(image) + 1) * sizeof(WCHAR)
        );
    if (cmd)
    {
        SHSetValueW(
            HKEY_LOCAL_MACHINE,
            wszSub,
            L"cmd",
            REG_SZ,
            cmd,
            (lstrlenW(cmd) + 1) * sizeof(WCHAR)
            );
    }

    SHSetValueW(
        HKEY_LOCAL_MACHINE,
        wszSub,
        L"sessionId",
        REG_DWORD,
        &session,
        sizeof(session)
        );

    DWORD dwShell = 0;
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    GetWindowThreadProcessId(hShell, &dwShell);

    if (dwShell)
    {
        SHSetValueW(HKEY_LOCAL_MACHINE, wszSub, L"shell", REG_DWORD, &dwShell, sizeof(dwShell));
    }

    HANDLE hNotify = OpenEventW(EVENT_MODIFY_STATE, FALSE, SFV_NOTIFY_NAME);
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
bool DbgCtrlService::ExecProc(const std::ustring &path, const std::ustring &param) {
    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "path", WtoU(path).c_str());
    cJSON_AddStringToObject(content, "param", WtoU(param).c_str());
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_EXEC, UtoW(cJSON_PrintUnformatted(content)));
    cJSON_Delete(content);
    return true;
}

bool DbgCtrlService::RunCmdInCtrlService(const std::ustring &command) {
    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "command", WtoU(command).c_str());
    m_pCtrlService->DispatchCurDbgger(DBG_CTRL_RUNCMD, UtoW(cJSON_PrintUnformatted(content)).c_str());
    cJSON_Delete(content);
    return true;
}

/*
{
    "cmd":"event",
    "content":{
        "type":"proccreate",
        "data":{
            "pid":"0x1234",
            "image":"d:\\desktop\\1234.exe",
            "baseAddr":"0x4344353",
            "entryAddr":"0x4344389"
        }
    }
}
*/
void DbgCtrlService::OnProcCreate(const ustring &event, const ustring &content, void *param) {
    cJSON *data = cJSON_Parse(WtoU(content).c_str());
    JsonAutoDelete tmp(data);

    if (!data || data->type != cJSON_Object)
    {
        return;
    }
    mstring pid = UtoA(GetStrFormJson(data, "pid"));
    mstring image = UtoA(GetStrFormJson(data, "image"));
    mstring baseAddr = UtoA(GetStrFormJson(data, "baseAddr"));
    mstring entryAddr = UtoA(GetStrFormJson(data, "entryAddr"));

    int processId = 0;
    sscanf(pid.c_str(), "0x%x", &processId);

    PrintFormater pf;
    pf << "进程启动" << space                    << line_end;
    pf << "进程Pid"  << FormatA("%d", processId) << line_end;
    pf << "映像路径" << image                    << line_end;
    pf << "进程基址" << baseAddr                 << line_end;
    pf << "入口地址" << entryAddr                << line_end;
    AppendToSyntaxView(SCI_LABEL_DEFAULT, pf.GetResult());
}

void DbgCtrlService::OnDbgMessage(const ustring &event, const ustring &content, void *param) {
}

void DbgCtrlService::OnProcExit(const ustring &event, const ustring &content, void *param) {
}

/*
{
    "cmd":"event",
    "content":{
        "type":"moduleload",
        "data":{
            "name":"kernel32.dll",
            "baseAddr":"0x4344353",
            "endAddr":"0x43443ff"
        }
    }
}
*/
void DbgCtrlService::OnModuleLoad(const ustring &event, const ustring &content, void *param) {
    cJSON *data = cJSON_Parse(WtoU(content).c_str());
    JsonAutoDelete abc(data);
    mstring name = UtoA(GetStrFormJson(data, "name"));
    mstring baseAddr = UtoA(GetStrFormJson(data, "baseAddr"));
    mstring endAddr = UtoA(GetStrFormJson(data, "endAddr"));

    PrintFormater pf;
    pf.SetRule("0;16");
    pf << "模块加载" << name << baseAddr << endAddr << line_end;
    AppendToSyntaxView(SCI_LABEL_DEFAULT, pf.GetResult());
}

void DbgCtrlService::OnModuleUnLoad(const ustring &event, const ustring &content, void *param) {
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
void DbgCtrlService::OnProcChanged(const ustring &event, const ustring &content, void *param) {
    CProcSelectView *pProcView = GetProcView();
    if (!IsWindow(pProcView->GetWndHandle()))
    {
        return;
    }

    cJSON *data = cJSON_Parse(WtoU(content).c_str());
    JsonAutoDelete abc(data);
    cJSON *add = cJSON_GetObjectItem(data, "add");
    cJSON *kill = cJSON_GetObjectItem(data, "kill");

    list<ProcMonInfo *> addSet;
    list<DWORD> killSet;
    if (add && add->type == cJSON_Array)
    {
        for (cJSON *it = add->child; it != NULL ; it = it->next)
        {
            /*
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
            }
            */
            ProcMonInfo *newProc = new ProcMonInfo();
            newProc->procUnique = GetIntFromJson(it, "unique");
            newProc->procPid = GetIntFromJson(it, "pid");
            newProc->procPath = UtoW(GetStrFormJson(it, "procPath"));
            newProc->procDesc = UtoW(GetStrFormJson(it, "procDesc"));
            newProc->procCmd = UtoW(GetStrFormJson(it, "cmd"));
            newProc->startTime = UtoW(GetStrFormJson(it, "startTime"));
            newProc->x64 = GetIntFromJson(it, "x64");
            newProc->sessionId = GetIntFromJson(it, "session");
            newProc->procUser = UtoW(GetStrFormJson(it, "user"));
            newProc->procUserSid = UtoW(GetStrFormJson(it, "sid"));
            addSet.push_back(newProc);
        }
    }

    if (kill && kill->type == cJSON_Array)
    {
        for (cJSON *ij = kill->child ; ij != NULL ; ij = ij->next)
        {
            killSet.push_back(ij->valueint);
        }
    }
    pProcView->OnProcChanged(addSet, killSet);
}