#ifndef OPENVIEW_VDEBUG_H_H_
#define OPENVIEW_VDEBUG_H_H_
#include <Windows.h>
#include "ViewBase.h"
#include <ComLib/global.h>
#include <ComStatic/ComStatic.h>

using namespace std;

class CPeFileOpenView : public CWindowBase, public CCriticalSectionLockable
{
public:
    CPeFileOpenView();
    virtual ~CPeFileOpenView();

protected:
    void OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnCommand(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnClose(HWND hwnd, WPARAM wp, LPARAM lp);
    void OnDropFiles(HWND hwnd, WPARAM wp, LPARAM lp);
    virtual LRESULT CPeFileOpenView::OnWindowMsg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    void FillFileInfo(const ustring &wstr);
    void ClearCtrl();
    ustring GetLastSelectDir();
    void RecordLastSelect(LPCWSTR wszDir);
    BOOL ChangeWndMessageFilter(UINT uMessage, BOOL bAllow);

protected:
    HWND m_hPePath;
    HWND m_hPeCmd;
    HWND m_hWorkDir;
    HWND m_hStatus;
};
#endif