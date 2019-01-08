#include <Windows.h>
#include <CommCtrl.h>
#include <ComLib/ComLib.h>
#include <ComLib/DbProxy.h>
#include "OpenView.h"
#include "resource.h"

static const int TIMER_ACTIVITY = 1511;

PeFileOpenDlg *PeFileOpenDlg::GetInstance() {
    static PeFileOpenDlg *s_ptr = NULL;
    if (s_ptr == NULL)
    {
        s_ptr = new PeFileOpenDlg();
    }
    return s_ptr;
}

PeFileOpenDlg::PeFileOpenDlg() {
}

PeFileOpenDlg::~PeFileOpenDlg() {
}

int PeFileOpenDlg::OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    PeFileOpenDlg *ptr = GetInstance();
    ptr->m_hParent = GetParent(hdlg);
    ptr->m_hTextPath = GetDlgItem(ptr->m_hParent, 0x442);
    ptr->m_hComPath = GetDlgItem(ptr->m_hParent, 0x47c);
    ptr->m_hTextType = GetDlgItem(ptr->m_hParent, 0x441);
    ptr->m_hComType = GetDlgItem(ptr->m_hParent, 0x470);

    ptr->m_hTextParam = GetDlgItem(hdlg, IDC_OPEN_TEXT1);
    ptr->m_hEditParam = GetDlgItem(hdlg, IDC_EDT_OPEN_CMD);

    ptr->m_hTextDir = GetDlgItem(hdlg, IDC_OPEN_TEXT2);
    ptr->m_hEditDir = GetDlgItem(hdlg, IDC_EDT_OPEN_WORKDIR);

    ptr->m_hTextHistory = GetDlgItem(hdlg, IDC_OPEN_TEXT3);
    ptr->m_hComHistory = GetDlgItem(hdlg, IDC_OPEN_COM_HISTORY);

    ptr->m_hEditStatus = GetDlgItem(hdlg, IDC_EDT_OPEN_STATUS);

    ptr->m_hBtnOk = GetDlgItem(ptr->m_hParent, 0x1);
    ptr->m_hBtnCancel = GetDlgItem(ptr->m_hParent, 0x2);

    RECT rtParent = {0};
    GetWindowRect(ptr->m_hParent, &rtParent);

    SetWindowTextW(ptr->m_hTextPath, L"程序路径");
    SetWindowTextW(ptr->m_hTextType, L"文件类型");

    SetTimer(hdlg, TIMER_ACTIVITY, 1, NULL);
    CentreWindow(ptr->m_hParent, GetInstance()->m_hParent);
    return 0;
}

int PeFileOpenDlg::OnSize(HWND hdlg, WPARAM wp, LPARAM lp) {
    PeFileOpenDlg *ptr = GetInstance();
    RECT rtDlg = {0};
    GetWindowRect(hdlg, &rtDlg);

    RECT rtParent = {0};
    GetWindowRect(ptr->m_hParent, &rtParent);

    int parentWidth = rtParent.right - rtParent.left;
    int parentHigh = rtParent.bottom - rtParent.top;

    RECT rt1, rt2, rt3, rt4;
    GetWindowRect(ptr->m_hTextPath, &rt1);
    GetWindowRect(ptr->m_hComPath, &rt2);
    GetWindowRect(ptr->m_hTextType, &rt3);
    GetWindowRect(ptr->m_hComType, &rt4);

    int spaceY = rt4.top - rt1.top + 3;
    int leftEdit = rt2.left - rtDlg.left;
    int topEdit = rt4.top + spaceY - rtDlg.top;
    int widthEdit = rt2.right - rt2.left;
    int highEdit = rt2.bottom - rt2.top;

    widthEdit = (parentWidth - leftEdit - 60);

    int leftText = rt1.left - rtDlg.left;
    int topText = rt3.top + spaceY;

    RECT rtText = {0};
    GetWindowRect(ptr->m_hTextParam, &rtText);
    int widthText = rtText.right - rtText.left;
    int highText = rtText.bottom - rtText.top;

    //param, dir, history位置调整
    SetWindowPos(ptr->m_hTextParam, 0, leftText, topEdit, widthText, highText, SWP_NOZORDER);
    SetWindowPos(ptr->m_hEditParam, 0, leftEdit, topEdit, widthEdit, highEdit, SWP_NOZORDER);
    SetWindowPos(ptr->m_hTextDir, 0, leftText, topEdit + spaceY, widthText, highText, SWP_NOZORDER);
    SetWindowPos(ptr->m_hEditDir, 0, leftEdit, topEdit + spaceY, widthEdit, highEdit, SWP_NOZORDER);
    SetWindowPos(ptr->m_hTextHistory, 0, leftText, topEdit + spaceY * 2, widthText, highText, SWP_NOZORDER);
    SetWindowPos(ptr->m_hComHistory, 0, leftEdit, topEdit + spaceY * 2, widthEdit, highEdit, SWP_NOZORDER);

    //path, type编辑框位置调整
    SetWindowPos(ptr->m_hComPath, 0, 0, 0, widthEdit, highEdit, SWP_NOZORDER | SWP_NOMOVE);
    SetWindowPos(ptr->m_hComType, 0, 0, 0, widthEdit, highEdit, SWP_NOZORDER | SWP_NOMOVE);

    RECT btnRect1 = {0};
    RECT btnRect2 = {0};
    GetWindowRect(ptr->m_hBtnOk, &btnRect1);
    GetWindowRect(ptr->m_hBtnCancel, &btnRect2);
    int btnWidth = (btnRect1.right - btnRect1.left);
    int btnHigh = (btnRect1.bottom - btnRect1.top);
    int btnY = topEdit + spaceY * 3 + 5;
    int btnXCannel = (leftEdit + widthEdit - btnWidth);
    int btnXOk = btnXCannel - btnWidth - 10;
    SetWindowPos(ptr->m_hBtnOk, 0, btnXOk, btnY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetWindowPos(ptr->m_hBtnCancel, 0, btnXCannel, btnY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetWindowTextW(ptr->m_hBtnOk, L"开始调试");
    SetWindowTextW(ptr->m_hBtnCancel, L"取消操作");

    //状态栏位置调整
    int editStatusY = topEdit + spaceY * 3 + 5;
    int leftStatusX = leftEdit;
    int widthStatus = btnXOk - leftStatusX - 20;
    SetWindowPos(ptr->m_hEditStatus, 0, leftStatusX, editStatusY, widthStatus, highEdit, SWP_NOZORDER);

    InvalidateRect(ptr->m_hTextParam, NULL, TRUE);
    InvalidateRect(ptr->m_hTextDir, NULL, TRUE);
    return 0;
}

int PeFileOpenDlg::OnTimer(HWND hdlg, WPARAM wp, LPARAM lp) {
    PeFileOpenDlg *ptr = GetInstance();
    if (TIMER_ACTIVITY == wp)
    {
        CentreWindow(ptr->m_hParent, GetInstance()->m_hParent);
        KillTimer(hdlg, TIMER_ACTIVITY);
    }
    return 0;
}

int PeFileOpenDlg::OnNotify(HWND hdlg, WPARAM wp, LPARAM lp) {
    PeFileOpenDlg *ptr = GetInstance();
    LPOFNOTIFY data = (LPOFNOTIFY)lp;

    if(data->hdr.code == CDN_FILEOK) {
        ptr->m_param.path = data->lpOFN->lpstrFile;

        WCHAR buf[256];
        buf[0] = 0;
        GetWindowTextW(ptr->m_hEditParam, buf, 256);
        ptr->m_param.command = buf;
        buf[0] = 0;
        GetWindowTextW(ptr->m_hEditDir, buf, 256);
        ptr->m_param.dir = buf;

        BOOL bSucc = FALSE;
        BOOL x64 = FALSE;
        if (!IsPeFileW(ptr->m_param.path.c_str(), &x64))
        {
            SetWindowTextW(ptr->m_hEditStatus, L"不是合法的可执行程序");
        } else if (x64)
        {
            SetWindowTextW(ptr->m_hEditStatus, L"尚不支持64位程序");
        } else {
            bSucc = TRUE;
        }
        ptr->m_param.succ = bSucc;

        if (!bSucc)
        {
            SetWindowLong(hdlg, DWL_MSGRESULT, 1);
            return 1;
        }
    }
    return 0;
}

UINT_PTR PeFileOpenDlg::OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
    int ret = 0;
    PeFileOpenDlg *ptr = GetInstance();
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            ptr->OnInitDialog(hdlg, wp, lp);
        }
        break;
    case WM_TIMER:
        {
            ptr->OnTimer(hdlg, wp, lp);
        }
        break;
    case WM_SIZE:
        {
            ptr->OnSize(hdlg, wp, lp);
        }
        break;
    case WM_NOTIFY:
        {
            ret = ptr->OnNotify(hdlg, wp, lp);
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            if (ptr->m_hEditStatus == (HWND)lp)
            {
                HDC dc = (HDC)wp;
                SetTextColor(dc, RGB(255, 0, 0));
                SetBkColor(dc, GetSysColor(COLOR_BTNFACE));

                static HBRUSH s_hbrBkgnd = GetSysColorBrush(COLOR_BTNFACE);
                return (INT_PTR)s_hbrBkgnd;
            }
        }
        break;
    }
    return ret;
}

bool PeFileOpenDlg::SaveHistory(HistoryInfo &history) const {
    history.mTime = GetCurTimeStr1("%4d-%2d-%2d %02d:%02d:%02d %03d");
    mstring str = WtoA(history.mPath);
    history.mId = crc32(str.c_str(), str.size(), 0xffabcdef);

    mstring sql = FormatA(
        "replace into tOpenHistory(id, path, param, dir, time)values(%u, '%ls', '%ls', '%ls', '%ls')",
        history.mId,
        history.mPath.c_str(),
        history.mParam.c_str(),
        history.mDir.c_str(),
        history.mTime.c_str()
        );
    return DbProxy::GetInstance()->ExecCfg(sql);
}

list<PeFileOpenDlg::HistoryInfo> PeFileOpenDlg::GetHistory(int maxSize) const {
    mstring sql = FormatA("select * from tOpenHistory order by time limit %d", maxSize);
    SqliteResult result = DbProxy::GetInstance()->SelectCfg(sql.c_str());

    list<HistoryInfo> ret;
    HistoryInfo tmp;
    for (SqliteIterator *it = result.begin() ; it != result.end() ; it++)
    {
        tmp.mId = (unsigned long)atoi(it->GetValue("id").c_str());
        tmp.mPath = it->GetValue("path");
        tmp.mParam = it->GetValue("param");
        tmp.mDir = it->GetValue("dir");
        tmp.mTime = it->GetValue("time");
        ret.push_back(tmp);
    }
    return ret;
}

BOOL PeFileOpenDlg::ShowFileOpenDlg(HWND parent, ProcParam &param) {
    m_hParent = parent;

    WCHAR buffer[512];
    buffer[0] = 0;
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parent;
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
    ofn.lpstrFilter = L"可执行程序\0*.exe;*.dll\0所有文件(*.*)\0*.*\0\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"选择要执行的程序";
    ofn.lpfnHook = OFNHookProc;
    ofn.lpTemplateName = MAKEINTRESOURCEW(IDD_PROC_OPEN);
    GetOpenFileNameW(&ofn);
    param = m_param;

    if (param.succ)
    {
        HistoryInfo history;
        history.mPath = param.path;
        history.mParam = param.command;
        history.mDir = param.dir;
        SaveHistory(history);
    }
    return param.succ;
}