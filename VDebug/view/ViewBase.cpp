#include "ViewBase.h"

map<HWND, CWindowBase *> *CWindowBase::m_pWndRegister = new map<HWND, CWindowBase *>();

LRESULT CWindowBase::OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    return 0;
}

BOOL CWindowBase::CreateDlg(DWORD dwId, HWND hParent, BOOL bDoModule)
{
    if (bDoModule)
    {
        DialogBoxParamW(NULL, MAKEINTRESOURCEW(dwId), hParent, DlgProc, (LPARAM)this);
    }
    else
    {
        m_hwnd = ::CreateDialogParamW(NULL, MAKEINTRESOURCEW(dwId), hParent, DlgProc, (LPARAM)this);
    }
    return IsWindow(m_hwnd);
}

INT_PTR CWindowBase::DlgProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    if (WM_INITDIALOG == uMsg)
    {
        CWindowBase *ptr = (CWindowBase *)lp;
        ptr->m_hwnd = hwnd;
        RegisterWnd(ptr);
    }

    CWindowBase *pWnd = GetWndFromHandle(hwnd);

    LRESULT res = 0;
    if (pWnd)
    {
        res = pWnd->OnWindowMsg(hwnd, uMsg, wp, lp);
    }

    if (WM_DESTROY == uMsg)
    {
        UnRegisterWnd(hwnd);
    }
    return res;
}

LRESULT CWindowBase::WinBaseProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    if (WM_NCCREATE == uMsg)
    {
        return DefWindowProc(hwnd, uMsg, wp, lp);
    }
    else if (WM_CREATE == uMsg)
    {
        RECT rt = {0};
        GetWindowRect(hwnd, &rt);
        CWindowBase *ptr = (CWindowBase *)((CREATESTRUCT *)lp)->lpCreateParams;
        ptr->m_hwnd = hwnd;
        RegisterWnd(ptr);
    }

    CWindowBase *pWnd = GetWndFromHandle(hwnd);

    LRESULT res = 0;
    if (pWnd)
    {
        res = pWnd->OnWindowMsg(hwnd, uMsg, wp, lp);
    }

    if (WM_DESTROY == uMsg)
    {
        UnRegisterWnd(hwnd);
    }
    return res;
}