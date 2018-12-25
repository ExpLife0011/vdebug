#include <Windows.h>
#include "DbgCtrlService.h"
#include "OpenView.h"
#include "resource.h"
#include "MainView.h"

#define REG_KEY_PROCCACHE       L"software\\vdebug\\proccache"
#define REG_VALUE_LAST_SELECT   L"lastselect"

CPeFileOpenView::CPeFileOpenView()
{}

CPeFileOpenView::~CPeFileOpenView()
{}

LRESULT CPeFileOpenView::OnWindowMsg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case  WM_INITDIALOG:
        OnInitDlg(hwndDlg, wParam, lParam);
        break;
    case  WM_COMMAND:
        OnCommand(hwndDlg, wParam, lParam);
        break;
    case WM_DROPFILES:
        OnDropFiles(hwndDlg, wParam, lParam);
        break;
    case WM_CLOSE:
        {
            OnClose(hwndDlg, wParam, lParam);
            EndDialog(hwndDlg, 0);
        }
        break;
    }
    return 0;
}

BOOL CPeFileOpenView::ChangeWndMessageFilter(UINT uMessage, BOOL bAllow)
{
    HMODULE hUserMod = NULL;
    BOOL bResult = FALSE;
    typedef BOOL (WINAPI* ChangeWindowMessageFilterFn)(UINT, DWORD);

    hUserMod = GetModuleHandleW(L"user32.dll");
    if (hUserMod == NULL)
    {
        return FALSE;
    }

    ChangeWindowMessageFilterFn pfnChangeWindowMessageFilter = 
        (ChangeWindowMessageFilterFn)GetProcAddress(hUserMod, "ChangeWindowMessageFilter");
    if (pfnChangeWindowMessageFilter == NULL)
    {
        FreeLibrary(hUserMod);
        return FALSE;
    }

    bResult = pfnChangeWindowMessageFilter(uMessage, bAllow ? MSGFLT_ADD : MSGFLT_REMOVE);
    return bResult;
}

void CPeFileOpenView::OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp)
{
    CentreWindow(hwnd, GetParent(hwnd));
    m_hPePath = GetDlgItem(hwnd, IDC_EDT_PEOPEN_PATH);
    m_hPeCmd = GetDlgItem(hwnd, IDC_EDT_PEOPEN_CMD);
    m_hWorkDir = GetDlgItem(hwnd, IDC_EDT_PEOPEN_WORKDIR);
    m_hStatus = GetDlgItem(hwnd, IDC_EDT_PEOPEN_STATUS);

    ChangeWndMessageFilter(WM_DROPFILES, TRUE);
    ChangeWndMessageFilter(0x0049, TRUE);
    DragAcceptFiles(hwnd, TRUE);
}

void CPeFileOpenView::OnCommand(HWND hwnd, WPARAM wp, LPARAM lp)
{
    DWORD id = LOWORD(wp);
    if (IDC_BTN_PEOPEN_SELECT == id)
    {
        OPENFILENAMEW ofn = {0};
        ZeroMemory(&ofn, sizeof(ofn));  
        WCHAR wszFileName[MAX_PATH] = {0};
        ofn.lpstrFile = wszFileName;
        ofn.nMaxFile = MAX_PATH;  
        ofn.lpstrFilter =L"可执行文件(*.exe)\0*.exe\0快捷方式(*.lnk)\0*.lnk\0All Files\0*.*\0\0";
        ofn.lpstrDefExt = L"exe";
        ofn.lpstrTitle = L"选择要调试的程序";
        ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
        ustring wstrDir = GetLastSelectDir();
        ofn.lpstrInitialDir = wstrDir.c_str();
        ofn.FlagsEx = OFN_EX_NOPLACESBAR;
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = hwnd;
        if (GetOpenFileNameW(&ofn))  
        {
            ClearCtrl();
            FillFileInfo(wszFileName);
            PathAppendW(wszFileName, L"..");
            RecordLastSelect(wszFileName);
        }
    }
    else if (IDC_BTN_PEOPEN_DEBUG == id)
    {
        WCHAR wszBuffer[MAX_PATH] = {0};
        GetWindowTextW(m_hPePath, wszBuffer, MAX_PATH);
        WCHAR param[MAX_PATH] = {0};
        GetWindowTextW(m_hPeCmd, param, MAX_PATH);

        BOOL x64 = FALSE;
        if (!IsPeFileW(wszBuffer, &x64))
        {
            SetWindowTextW(m_hStatus, L"不是有效的pe文件");
            return;
        }

        if (x64)
        {
            DbgCtrlService::GetInstance()->SetDebuggerStat(em_dbg_proc64);
        } else {
            DbgCtrlService::GetInstance()->SetDebuggerStat(em_dbg_proc86);
        }
        DbgCtrlService::GetInstance()->ExecProc(wszBuffer, param);
    }
}

void CPeFileOpenView::ClearCtrl()
{
    SetWindowTextW(m_hPePath, L"");
    SetWindowTextW(m_hPeCmd, L"");
    SetWindowTextW(m_hWorkDir, L"");
}

void CPeFileOpenView::FillFileInfo(const ustring &wstr)
{
    GDS_LINKINFO info;
    if (ShlParseShortcutsW(wstr.c_str(), &info))
    {
        SetWindowTextW(m_hPePath, info.wszPath);
        SetWindowTextW(m_hPeCmd, info.wszArgs);
        SetWindowTextW(m_hWorkDir, info.wszWorkDir);
    }
    else
    {
        SetWindowTextW(m_hPePath, wstr.c_str());
    }
}

void CPeFileOpenView::OnDropFiles(HWND hwnd, WPARAM wp, LPARAM lp)
{
    ClearCtrl();
    WCHAR wszBuffer[MAX_PATH] = {0};
    DragQueryFileW((HDROP)wp, 0, wszBuffer, MAX_PATH);
    if (wszBuffer[0])
    {
        FillFileInfo(wszBuffer);
    }
}

void CPeFileOpenView::OnClose(HWND hwnd, WPARAM wp, LPARAM lp)
{}

ustring CPeFileOpenView::GetLastSelectDir()
{
    WCHAR wszValue[MAX_PATH] = {0};
    DWORD dwSize = sizeof(wszValue);
    SHGetValueW(HKEY_LOCAL_MACHINE, REG_KEY_PROCCACHE, REG_VALUE_LAST_SELECT, NULL, wszValue, &dwSize);

    if (!wszValue[0])
    {
        GetModuleFileNameW(NULL, wszValue, MAX_PATH);
        PathAppendW(wszValue, L"..");
    }
    return wszValue;
}

void CPeFileOpenView::RecordLastSelect(LPCWSTR wszDir)
{
    SHSetValueW(HKEY_LOCAL_MACHINE, REG_KEY_PROCCACHE, REG_VALUE_LAST_SELECT, REG_SZ, wszDir, lstrlenW(wszDir) * sizeof(WCHAR));
}
