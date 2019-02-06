#include <Windows.h>
#include <CommCtrl.h>
#include <ComLib/ComLib.h>
#include <SyntaxHlpr/SyntaxCfg.h>
#include <SyntaxHlpr/SyntaxView.h>
#include <SyntaxHlpr/SyntaxParser.h>

#include "ProcView.h"
#include "resource.h"
#include "MainView.h"
#include "CmdQueue.h"
#include "OpenView.h"
#include "DbgCtrlService.h"

#pragma comment(lib, "comctl32.lib")

#if defined _M_IX86
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define IDT_OPEN_APP    IDC_CMD_OPEN
#define IDT_ATTACH_PROC IDC_CMD_ATTACH
#define IDT_OPEN_DUMP   IDC_CMD_OPEN_DUMP
#define IDT_FIND_WND    (WM_USER + 601)

#define IDT_FIND_DATA   (WM_USER + 602)
#define IDT_SAVE_DATA   (WM_USER + 603)
#define IDT_EXIT_DEBUG  IDC_CMD_DETACH
#define IDT_CFG_DEBUG   (WM_USER + 605)
#define IDT_REFUSH      (WM_USER + 606)
#define IDT_7           (WM_USER + 607)
#define IDT_8           (WM_USER + 608)
#define IDT_9           (WM_USER + 609)
#define IDT_FONT        (WM_USER + 611)
#define IDT_ABOUT       IDC_CMD_ABOUT

#define  MSG_EXEC_COMMAND   (WM_USER + 610)
#define  MSG_PAGE_CHANGE    (WM_USER + 620)
#define  IDC_STATUS_BAR 100077
#define  TIMER_CFG_CHECK 5001

extern HINSTANCE g_hInstance;
static SyntaxView *gs_pSyntaxView = NULL;
static HWND gs_hMainView = NULL;
static HWND gs_hStatEdit = NULL;
static HWND gs_hCommand = NULL;
static HWND gs_hStatBar = NULL;
static HWND gs_hToolbar = NULL;
static HFONT gs_hFont = NULL;
static mstring gs_strCfgFile;
static CProcSelectView *gs_pProcSelect = NULL;
static PeFileOpenDlg *gs_pPeOpenView = NULL;
static CCmdQueue *gs_pCmdQueue = NULL;

struct SyntaxShowData {
    mstring m_label;
    mstring m_data;
};

#define MSG_APPEND_MSG (WM_USER + 651)
static CCriticalSectionLockable gs_dataLock;
static list<SyntaxShowData *> gs_showData;

void AppendToSyntaxView(const std::mstring &label, const std::mstring &data) {
    {
        CScopedLocker lock(&gs_dataLock);
        SyntaxShowData *p = new SyntaxShowData();
        p->m_label = label;
        p->m_data = data;
        gs_showData.push_back(p);
    }
    PostMessageW(gs_hMainView, MSG_APPEND_MSG, 0, 0);
}

SyntaxView *GetSyntaxView()
{
    return gs_pSyntaxView;
}

CProcSelectView *GetProcView() {
    return gs_pProcSelect;
}

static VOID _CreateStatusBar(HWND hdlg)
{
    gs_hStatBar = CreateStatusWindowW(WS_CHILD | WS_VISIBLE, NULL, hdlg, IDC_STATUS_BAR);
    int wide[5] = {0};
    int length = 0;
    //声明
    wide[0] = 280;
    //封包统计
    wide[1] = wide[0] + 360;
    //选择范围
    wide[2]= wide[1] + 160;
    //选择的数值
    wide[3] = wide[2] + 360;
    //无用的
    wide[4] = wide[3] + 256;
    SendMessage(gs_hStatBar, SB_SETPARTS, sizeof(wide) / sizeof(int), (LPARAM)(LPINT)wide); 
}

static VOID _LoadDebugFile()
{
    ustring wstrTest;
    GetModuleFileNameW(NULL, wstrTest.alloc(MAX_PATH), MAX_PATH);
    wstrTest.setbuffer();
    wstrTest.path_append(L"..\\test.txt");

    HANDLE hFile = CreateFileW(
        wstrTest.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return;
    }

    DWORD dwSize = GetFileSize(hFile, NULL);
    char *szBuff = new char[dwSize];
    DWORD dwRead = 0;
    ReadFile(hFile, szBuff, dwSize, &dwRead, NULL);
    ustring wstr = AtoW(mstring(szBuff, dwRead));
    delete []szBuff;
    CloseHandle(hFile);
}

static VOID _LoadDefaultFont()
{
    LOGFONTW ft = {0};
    //字体大小需要是偶数，否则中文和两个英文宽度会差一个像素
    ft.lfHeight = 14;
    ft.lfWidth = 0;
    ft.lfEscapement = 0;
    ft.lfWeight = 0;
    ft.lfItalic = 0;
    ft.lfUnderline = 0;
    ft.lfStrikeOut = 0;

    static LPCWSTR s_vFontArry[] =
    {
        L"宋体",
        L"Lucida Console",
        L"Courier New"
    };

    for (DWORD dwIdex = 0 ; dwIdex < RTL_NUMBER_OF(s_vFontArry) ; dwIdex++)
    {
        lstrcpynW(ft.lfFaceName, s_vFontArry[dwIdex], RTL_NUMBER_OF(ft.lfFaceName));
        if (gs_hFont = CreateFontIndirectW(&ft))
        {
            SendMessage(gs_hMainView,WM_SETFONT, (WPARAM)gs_hFont, 1);
            SendMessage(gs_hStatBar,WM_SETFONT, (WPARAM)gs_hFont, 1);
            SendMessage(gs_hStatEdit,WM_SETFONT, (WPARAM)gs_hFont, 1);
            SendMessage(gs_hCommand,WM_SETFONT, (WPARAM)gs_hFont, 1);
            break;
        }
    }
}

static VOID _CreateToolBar()
{
    TBBUTTON tbb[] =
    {
        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},

        {0, IDT_OPEN_APP, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"调试程序")},
        {2, IDT_FIND_WND, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"查找窗口")},
        {3, IDT_FIND_DATA, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"查找数据")},
        {4, IDT_SAVE_DATA, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"保存数据")},
        {5, IDT_EXIT_DEBUG, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"脱离调试器")},
        {6, IDT_REFUSH, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"刷新窗体")},

        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},

        {7, IDT_7, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"test7")},
        {8, IDT_8, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"test8")},
        {9, IDT_9, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"test9")},

        {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},

        {11, IDT_FONT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"设置字体")},
        {12, IDT_ABOUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, {0, 0}, 0, (INT_PTR)(L"关于VDebug")},
    };

    HIMAGELIST image = NULL;
    image = ImageList_LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDB_TOOLBAR), 16, 15, 0xFF000000, 0, 0);
    gs_hToolbar = CreateWindowExW(
        0,
        TOOLBARCLASSNAMEW,
        NULL,
        WS_CHILD | TBSTYLE_FLAT | WS_BORDER | CCS_NOMOVEY | CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS | TBSTYLE_ALTDRAG,
        0,
        0,
        0,
        0,
        gs_hMainView,
        NULL,
        g_hInstance,
        NULL
        ); 

    SendMessageW(gs_hToolbar, TB_SETIMAGELIST, 0, (LPARAM)image);
    SendMessageW(gs_hToolbar, TB_ADDBUTTONSW, (WPARAM)(sizeof(tbb) / sizeof(TBBUTTON)), (LPARAM)(LPTBBUTTON)&tbb);
    SendMessageW(gs_hToolbar, TB_SETMAXTEXTROWS, (WPARAM) 0, 0);;
    ShowWindow(gs_hToolbar, SW_SHOW);
}

//调整控件位置
static VOID _MoveMainWndCtrl()
{
    RECT rtClient = {0};
    GetClientRect(gs_hMainView, &rtClient);

    RECT rtToolBar = {0};
    RECT rtStatBar = {0};
    RECT rtEdtStat = {0};
    RECT rtCommand = {0};
    GetWindowRect(gs_hToolbar, &rtToolBar);
    MapWindowPoints(NULL, gs_hMainView, (LPPOINT)&rtToolBar, 2);
    GetWindowRect(gs_hStatBar, &rtStatBar);
    MapWindowPoints(NULL, gs_hMainView, (LPPOINT)&rtStatBar, 2);
    GetWindowRect(gs_hStatEdit, &rtEdtStat);
    MapWindowPoints(NULL, gs_hMainView, (LPPOINT)&rtEdtStat, 2);
    GetWindowRect(gs_hCommand, &rtCommand);
    MapWindowPoints(NULL, gs_hMainView, (LPPOINT)&rtCommand, 2);

    int iY = (rtToolBar.bottom - rtToolBar.top);
    int iCY = rtClient.bottom - (rtStatBar.bottom - rtStatBar.top) - (rtEdtStat.bottom - rtEdtStat.top) - (rtToolBar.bottom - rtToolBar.top) - 5 -5;
    MoveWindow(gs_hStatEdit, 5, iY + iCY + 5, rtEdtStat.right - rtEdtStat.left, rtEdtStat.bottom - rtEdtStat.top, TRUE);
    MoveWindow(gs_hCommand, rtEdtStat.right + 5 + 5, iY + iCY + 5, rtCommand.right - rtCommand.left - 5, rtCommand.bottom - rtCommand.top, TRUE);
    MoveWindow(gs_pSyntaxView->GetWindow(), 0, iY, rtClient.right - rtClient.left, iCY, TRUE);
}

typedef LRESULT (CALLBACK *PWIN_PROC)(HWND, UINT, WPARAM, LPARAM);
static PWIN_PROC gs_pfnCommandProc = NULL;

static LRESULT CALLBACK _CommandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (WM_CHAR == msg)
    {
        if (0x0d == wp)
        {
            SendMessageW(gs_hMainView, MSG_EXEC_COMMAND, 0, 0);
        }
    }

    if (WM_KEYDOWN == msg)
    {
        if (VK_UP == wp || VK_DOWN == wp)
        {
            SendMessageW(gs_hMainView, MSG_PAGE_CHANGE, wp, lp);
        }
    }
    return CallWindowProc(gs_pfnCommandProc, hwnd, msg, wp, lp);
}

static void _InitSyntaxView() {
    gs_pSyntaxView = new SyntaxView();

    gs_pSyntaxView->CreateView(gs_hMainView, 0, 0, 100, 100);
    gs_pSyntaxView->ShowMargin(false);
    gs_pSyntaxView->SetCaretColour(RGB(255, 255, 255));

    gs_pSyntaxView->SetFont("Lucida Console");
    string ff = gs_pSyntaxView->GetFont();
    gs_pSyntaxView->SetCaretSize(1);

    gs_pSyntaxView->SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    gs_pSyntaxView->ShowVsScrollBar(true);
    gs_pSyntaxView->ShowHsScrollBar(true);
}

static HHOOK gs_pfnKeyboardHook = NULL;
static LRESULT CALLBACK _KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (GetAsyncKeyState(VK_CONTROL) & (1 << 16)){
        // ctrl+b break debugger
        if (wParam == 0x42 && DbgCtrlService::GetInstance()->GetDebuggerStat() == em_dbg_status_busy)
        {
            static DWORD s_lastCount = 0;

            DWORD curCount = GetTickCount();
            if (curCount - s_lastCount >= 1000)
            {
                s_lastCount = curCount;
                DbgCtrlService::GetInstance()->BreakDbgProcInCtrlService();
            }
        }
    }

    return CallNextHookEx(gs_pfnKeyboardHook, code, wParam, lParam);
}

static VOID _OnInitDialog(HWND hwnd, WPARAM wp, LPARAM lp)
{
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    CentreWindow(hwnd);
    gs_hMainView = hwnd;
    gs_hStatEdit = GetDlgItem(hwnd, IDC_EDT_STATUS);
    gs_hCommand = GetDlgItem(hwnd, IDC_EDT_COMMAND);
    _CreateStatusBar(hwnd);
    _CreateToolBar();

    RECT rtClient = {0};
    GetClientRect(gs_hMainView, &rtClient);
    gs_pProcSelect = new CProcSelectView;
    gs_pPeOpenView = PeFileOpenDlg::GetInstance();

    _InitSyntaxView();

    _LoadDefaultFont();
    _MoveMainWndCtrl();

    GetModuleFileNameA(NULL, gs_strCfgFile.alloc(MAX_PATH), MAX_PATH);
    gs_strCfgFile.setbuffer();
    gs_strCfgFile.path_append("..\\SyntaxCfg.json");
    LoadSyntaxCfg(gs_strCfgFile.c_str());
    UpdateSyntaxView(gs_pSyntaxView);

    CTL_PARAMS vCtrls[] =
    {
        {0, gs_hToolbar, 0, 0, 1, 0},
        {0, gs_pSyntaxView->GetWindow(), 0, 0, 1, 1},
        {0, gs_hStatEdit, 0, 1, 0, 0},
        {0, gs_hCommand, 0, 1, 1, 0},
        {0, gs_hStatBar, 0, 1, 1, 0}
    };

    SetCtlsCoord(hwnd, vCtrls, RTL_NUMBER_OF(vCtrls));
    SetTimer(hwnd, TIMER_CFG_CHECK, 500, NULL);

    SetCmdNotify(em_dbg_status_init, "初始状态");
    gs_pfnCommandProc = (PWIN_PROC)SetWindowLongPtr(gs_hCommand, GWLP_WNDPROC, (LONG_PTR)_CommandProc);
    gs_pCmdQueue = new CCmdQueue();

    //SetWindowPos(gs_hMainView, 0, 0, 0, 600, 160, SWP_NOMOVE | SWP_NOZORDER);

    ustring wstrVersion;
    WCHAR wszBuf[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, wszBuf, MAX_PATH);
    GetPeVersionW(wszBuf, wstrVersion.alloc(MAX_PATH), MAX_PATH);
    wstrVersion.setbuffer();

    gs_pfnKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, _KeyboardProc, g_hInstance, GetCurrentThreadId());

    AppendToSyntaxView(SCI_LABEL_DEFAULT, FormatA("VDebug调试器，版本：%ls\n", wstrVersion.c_str()));
}

static mstring _OpenDumpFile() {
    char dumpPath[256] = {0};

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = gs_hMainView;
    ofn.hInstance = GetModuleHandleA(NULL);
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLETEMPLATE;
    ofn.lpstrFilter = "dump文件\0*.dmp;*.dump\0所有文件(*.*)\0*.*\0\0";
    ofn.lpstrFile = dumpPath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "选择要分析的Dump文件";
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = MAKEINTRESOURCEA(IDD_PROC_OPEN);
   
    if (GetOpenFileNameA(&ofn))
    {
        return dumpPath;
    }
    return "";
}

static VOID _OnCommand(HWND hwnd, WPARAM wp, LPARAM lp)
{
    DWORD dwId = LOWORD(wp);
    switch (dwId)
    {
    case IDT_EXIT_DEBUG:
        {
            DbgCtrlService::GetInstance()->DetachProc();
        }
        break;
    case  IDC_CMD_OPEN:
        {
            ProcParam param; 
            if (gs_pPeOpenView->ShowFileOpenDlg(hwnd, param))
            {
                if (param.x64)
                {
                    DbgCtrlService::GetInstance()->SetDebugger(em_dbg_proc64);
                } else {
                    DbgCtrlService::GetInstance()->SetDebugger(em_dbg_proc86);
                }
                DbgCtrlService::GetInstance()->ExecProc(param.path, param.command);
            }
        }
        break;
    case IDC_CMD_ATTACH:
        {
            gs_pProcSelect->CreateDlg(IDD_PROC_ATTACH, hwnd, TRUE);
        }
        break;
    case IDC_CMD_OPEN_DUMP: 
        {
        }
        break;
    default:
        break;
    }
}

static VOID _OnTimer(HWND hwnd, WPARAM wp, LPARAM lp)
{
    if (TIMER_CFG_CHECK == wp)
    {
        static FILETIME s_timeLastWrite = {0};
        FILETIME time = {0};
        HANDLE hFile = CreateFileA(
            gs_strCfgFile.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );
        if (INVALID_HANDLE_VALUE == hFile)
        {
            return;
        }

        GetFileTime(hFile, NULL, NULL, &time);
        if (memcmp(&s_timeLastWrite, &time, sizeof(time)))
        {
            s_timeLastWrite = time;
            //gs_pSyntaxView->ReloadSynbaxCfg(gs_wstrCfgFile.c_str());
        }
        CloseHandle(hFile);
    }
}

static VOID _OnClose(HWND hwnd)
{
    EndDialog(hwnd, 0);
}

static VOID _OnExecCommand(HWND hwnd, WPARAM wp, LPARAM lp)
{
    mstring cmd = GetWindowStrA(gs_hCommand);
    cmd.trim();
    if (cmd.empty())
    {
        return;
    }

    gs_pCmdQueue->EnterCmd(cmd);
    AppendToSyntaxView(SCI_LABEL_DEFAULT, GetWindowStrA(gs_hStatEdit) + " " + cmd + "\n");
    CmdReplyResult result = DbgCtrlService::GetInstance()->RunCmdInCtrlService(cmd);
    AppendToSyntaxView(result.mCmdLabel, result.mCmdShow);
    SetWindowTextA(gs_hCommand, "");
}

static VOID _OnPageChange(HWND hwnd, WPARAM wp, LPARAM lp)
{
    if (DbgCtrlService::GetInstance()->GetDebuggerStat() != em_dbg_status_free)
    {
        return;
    }

    mstring str;
    if (VK_UP == wp)
    {
        str = gs_pCmdQueue->GetFrontCmd();
    }
    else if (VK_DOWN == wp)
    {
        str = gs_pCmdQueue->GetLastCmd();
    }
    SetWindowTextA(gs_hCommand, str.c_str());
    SendMessageA(gs_hCommand, EM_SETSEL, str.size(), str.size());
}

static void _OnAppendMsg() {
    list<SyntaxShowData *> tmp;
    {
        CScopedLocker lock(&gs_dataLock);
        tmp = gs_showData;
        gs_showData.clear();
    }

    for (list<SyntaxShowData *>::const_iterator it = tmp.begin() ; it != tmp.end() ; it++)
    {
        SyntaxShowData *p = *it;
        gs_pSyntaxView->AppendText(p->m_label, p->m_data);
        delete p;
    }
    gs_pSyntaxView->SetScrollEndLine();
}

static INT_PTR _OnKeyDown(HWND hdlg, WPARAM wp, LPARAM lp) {
    if (GetAsyncKeyState(VK_CONTROL) & (1 << 16))
    {
        int ddd = 123;
    }
    return 0;
}

static INT_PTR CALLBACK _MainViewProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int ret = 0;
    switch(msg)
    {
    case  WM_INITDIALOG:
        {
            _OnInitDialog(hdlg, wp, lp);
        }
        break;
    case  WM_COMMAND:
        {
            _OnCommand(hdlg, wp, lp);
        }
        break;
    case WM_TIMER:
        {
            _OnTimer(hdlg, wp, lp);
        }
        break;
    case WM_KEYDOWN:
        {
            _OnKeyDown(hdlg, wp, lp);
        }
        break;
    case MSG_EXEC_COMMAND:
        {
            _OnExecCommand(hdlg, wp, lp);
        }
        break;
    case  MSG_PAGE_CHANGE:
        {
            _OnPageChange(hdlg, wp, lp);
        }
        break;
    case MSG_APPEND_MSG:
        {
            _OnAppendMsg();
        }
        break;
    case WM_CLOSE:
        {
            _OnClose(hdlg);
        }
        break;
    }
    return 0;
}

static void _DisableCtrls()
{
    HMENU hMenu = GetMenu(gs_hMainView);
    EnableMenuItem(hMenu, IDC_CMD_OPEN, MF_GRAYED);
    EnableMenuItem(hMenu, IDC_CMD_ATTACH, MF_GRAYED);
    EnableMenuItem(hMenu, IDC_CMD_OPEN_DUMP, MF_GRAYED);

    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_OPEN_APP, FALSE);
    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_ATTACH_PROC, FALSE);
    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_OPEN_DUMP, FALSE);//
}

static void _EnableCtrls()
{
    HMENU hMenu = GetMenu(gs_hMainView);
    EnableMenuItem(hMenu, IDC_CMD_OPEN, MF_ENABLED);
    EnableMenuItem(hMenu, IDC_CMD_ATTACH, MF_ENABLED);
    EnableMenuItem(hMenu, IDC_CMD_OPEN_DUMP, MF_ENABLED);

    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_OPEN_APP, TRUE);
    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_ATTACH_PROC, TRUE);
    SendMessageW(gs_hToolbar, TB_ENABLEBUTTON, IDT_OPEN_DUMP, TRUE);
}

VOID SetCmdNotify(DbggerStatus status, const mstring &strShowMsg)
{
    SetWindowTextA(gs_hStatEdit, strShowMsg.c_str());
    if (em_dbg_status_init == status)
    {
        SetWindowTextA(gs_hCommand, "");
        SendMessageA(gs_hCommand, EM_SETREADONLY, 1, 0);
        _EnableCtrls();
    }
    else if (em_dbg_status_free == status)
    {
        SendMessageA(gs_hCommand, EM_SETREADONLY, 0, 0);
        SetFocus(gs_hCommand);
        _DisableCtrls();
    }
    else if (em_dbg_status_busy == status)
    {
        SetWindowTextA(gs_hCommand, "");
        SendMessageA(gs_hCommand, EM_SETREADONLY, 1, 0);
        _DisableCtrls();
    }
}

VOID SetMainviewTitle(const ustring &wstrTitle)
{
}

VOID ShowMainView()
{
    DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_MAINVIEW), NULL, _MainViewProc);
}

VOID SetInputStat() {
    SetFocus(gs_hCommand);
}