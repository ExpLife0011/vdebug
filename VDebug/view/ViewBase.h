#ifndef VIEWBASE_VDEBUG_H_H_
#define VIEWBASE_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <ComStatic/ComStatic.h>

using namespace std;

class CWindowBase
{
public:
    CWindowBase()
    {
        m_hwnd = NULL;
        ZeroMemory(&m_wndClass, sizeof(m_wndClass));
        m_bRegisterSucc = FALSE;
    }

    virtual ~CWindowBase()
    {}

    virtual LRESULT OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
    virtual BOOL SetFont(HFONT hFont)
    {
        return FALSE;
    }

    virtual BOOL Refush()
    {
        return FALSE;
    }

    virtual BOOL RegisterWnd(const WNDCLASSW &WndClass)
    {
        WNDCLASSW vCurClass = WndClass;
        vCurClass.lpfnWndProc = WinBaseProc;
        WNDCLASSW vWndClass = {0};
        if (!GetClassInfoW(NULL, vCurClass.lpszClassName, &vWndClass))
        {
            if(!RegisterClassW(&vCurClass))
            {
                return FALSE;
            }
        }
        m_wndClass = vCurClass;
        m_bRegisterSucc = TRUE;
        return TRUE;
    }

    virtual BOOL CreateWnd(HWND hParent, DWORD dwX, DWORD dwY, DWORD dwCX, DWORD dwCY, DWORD dwFlag)
    {
        if (!m_bRegisterSucc)
        {
            return FALSE;
        }

        extern HINSTANCE g_hInstance;
        m_hwnd = CreateWindowW(
            m_wndClass.lpszClassName,
            L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL,
            dwX, dwY,
            dwCX,
            dwCY,
            hParent,
            NULL,
            g_hInstance,
            this
            );
        return IsWindow(m_hwnd);
    }

    virtual BOOL CreateDlg(DWORD dwId, HWND hParent = NULL, BOOL bDoModule = TRUE);

    VOID Attach(HWND hwnd)
    {
        m_hwnd = hwnd;
    }

    HWND GetWndHandle()
    {
        return m_hwnd;
    }

    BOOL SendMsg(UINT uMsg, WPARAM wp, LPARAM lp)
    {
        if (IsWindow(m_hwnd))
        {
            return (BOOL)SendMessageW(m_hwnd, uMsg, wp, lp);
        }
        return FALSE;
    }
protected:
    HWND m_hwnd;
    WNDCLASSW m_wndClass;
    BOOL m_bRegisterSucc;

protected:
    static map<HWND, CWindowBase *> *m_pWndRegister;

    static VOID RegisterWnd(CWindowBase *ptr)
    {
        if (ptr && IsWindow(ptr->GetWndHandle()))
        {
            (*m_pWndRegister)[ptr->GetWndHandle()] = ptr;
        }
    }

    static VOID UnRegisterWnd(HWND hwnd)
    {
        if (m_pWndRegister->end() != m_pWndRegister->find(hwnd))
        {
            m_pWndRegister->erase(hwnd);
        }
    }

    static CWindowBase *GetWndFromHandle(HWND hwnd)
    {
        if (m_pWndRegister->end() != m_pWndRegister->find(hwnd))
        {
            return (*m_pWndRegister)[hwnd];
        }
        return NULL;
    }

    static LRESULT CALLBACK WinBaseProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
    static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
};
#endif