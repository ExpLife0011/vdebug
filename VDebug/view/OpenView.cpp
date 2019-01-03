#include <Windows.h>
#include <ComLib/ComLib.h>
#include "OpenView.h"
#include "resource.h"

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

UINT_PTR PeFileOpenDlg::OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
    static const int TIMER_ACTIVITY = 1511;
    static HWND s_hParent = NULL;
    static HWND s_hTextPath = NULL;
    static HWND s_hComPath = NULL;
    static HWND s_hTextType = NULL;
    static HWND s_hComType = NULL;

    static HWND s_hBtnOk = NULL;
    static HWND s_hBtnCancel = NULL;

    static HWND s_hTextParam = NULL;
    static HWND s_hEditParam = NULL;
    static HWND s_hTextDir = NULL;
    static HWND s_hEditDir = NULL;
    static HWND s_hTextHistory = NULL;
    static HWND s_hComHistory = NULL;

    PeFileOpenDlg *ptr = GetInstance();
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            s_hParent = GetParent(hdlg);
            s_hTextPath = GetDlgItem(s_hParent, 0x442);
            s_hComPath = GetDlgItem(s_hParent, 0x47c);
            s_hTextType = GetDlgItem(s_hParent, 0x441);
            s_hComType = GetDlgItem(s_hParent, 0x470);

            s_hTextParam = GetDlgItem(hdlg, IDC_OPEN_TEXT1);
            s_hEditParam = GetDlgItem(hdlg, IDC_EDT_OPEN_CMD);

            s_hTextDir = GetDlgItem(hdlg, IDC_OPEN_TEXT2);
            s_hEditDir = GetDlgItem(hdlg, IDC_EDT_OPEN_WORKDIR);

            s_hTextHistory = GetDlgItem(hdlg, IDC_OPEN_TEXT3);
            s_hComHistory = GetDlgItem(hdlg, IDC_OPEN_COM_HISTORY);

            s_hBtnOk = GetDlgItem(s_hParent, 0x1);
            s_hBtnCancel = GetDlgItem(s_hParent, 0x2);

            RECT rtParent = {0};
            GetWindowRect(s_hParent, &rtParent);

            SetWindowTextW(s_hTextPath, L"程序路径");
            SetWindowTextW(s_hTextType, L"文件类型");

            SetTimer(hdlg, TIMER_ACTIVITY, 1, NULL);
            CentreWindow(s_hParent, GetInstance()->m_hParent);
        }
        break;
    case WM_TIMER:
        {
            if (TIMER_ACTIVITY == wp)
            {
                CentreWindow(s_hParent, GetInstance()->m_hParent);
                KillTimer(hdlg, TIMER_ACTIVITY);
            }
        }
        break;
    case WM_SIZE:
        {
            RECT rtDlg = {0};
            GetWindowRect(hdlg, &rtDlg);

            RECT rtParent = {0};
            GetWindowRect(s_hParent, &rtParent);

            int parentWidth = rtParent.right - rtParent.left;
            int parentHigh = rtParent.bottom - rtParent.top;

            RECT rt1, rt2, rt3, rt4;
            GetWindowRect(s_hTextPath, &rt1);
            GetWindowRect(s_hComPath, &rt2);
            GetWindowRect(s_hTextType, &rt3);
            GetWindowRect(s_hComType, &rt4);

            int spaceY = rt4.top - rt1.top + 3;
            int leftEdit = rt2.left - rtDlg.left;
            int topEdit = rt4.top + spaceY - rtDlg.top;
            int widthEdit = rt2.right - rt2.left;
            int highEdit = rt2.bottom - rt2.top;

            widthEdit = (parentWidth - leftEdit - 60);

            int leftText = rt1.left - rtDlg.left;
            int topText = rt3.top + spaceY;

            RECT rtText = {0};
            GetWindowRect(s_hTextParam, &rtText);
            int widthText = rtText.right - rtText.left;
            int highText = rtText.bottom - rtText.top;

            //param, dir, history位置调整
            SetWindowPos(s_hTextParam, 0, leftText, topEdit, widthText, highText, SWP_NOZORDER);
            SetWindowPos(s_hEditParam, 0, leftEdit, topEdit, widthEdit, highEdit, SWP_NOZORDER);
            SetWindowPos(s_hTextDir, 0, leftText, topEdit + spaceY, widthText, highText, SWP_NOZORDER);
            SetWindowPos(s_hEditDir, 0, leftEdit, topEdit + spaceY, widthEdit, highEdit, SWP_NOZORDER);
            SetWindowPos(s_hTextHistory, 0, leftText, topEdit + spaceY * 2, widthText, highText, SWP_NOZORDER);
             SetWindowPos(s_hComHistory, 0, leftEdit, topEdit + spaceY * 2, widthEdit, highEdit, SWP_NOZORDER);

            //path, type编辑框位置调整
            SetWindowPos(s_hComPath, 0, 0, 0, widthEdit, highEdit, SWP_NOZORDER | SWP_NOMOVE);
            SetWindowPos(s_hComType, 0, 0, 0, widthEdit, highEdit, SWP_NOZORDER | SWP_NOMOVE);

            RECT btnRect1 = {0};
            RECT btnRect2 = {0};
            GetWindowRect(s_hBtnOk, &btnRect1);
            GetWindowRect(s_hBtnCancel, &btnRect2);
            int btnWidth = (btnRect1.right - btnRect1.left);
            int btnHigh = (btnRect1.bottom - btnRect1.top);
            int btnY = topEdit + spaceY * 3 + 5;
            int btnXCannel = (leftEdit + widthEdit - btnWidth);
            int btnXOk = btnXCannel - btnWidth - 10;
            SetWindowPos(s_hBtnOk, 0, btnXOk, btnY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            SetWindowPos(s_hBtnCancel, 0, btnXCannel, btnY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            SetWindowTextW(s_hBtnOk, L"开始调试");
            SetWindowTextW(s_hBtnCancel, L"取消操作");

            InvalidateRect(s_hTextParam, NULL, TRUE);
            InvalidateRect(s_hTextDir, NULL, TRUE);
        }
        break;
    case WM_NOTIFY:
        break;
    }
    return 0;
}

bool PeFileOpenDlg::ShowFileOpenDlg(HWND parent, ProcParam &param) {
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
    return true;
}