#pragma once
#include <Windows.h>
#include <map>

class CDialogBase {
public:
    CDialogBase();
    virtual ~CDialogBase();

    int DoModule(HWND parent, DWORD id);
    BOOL CreateDlg(HWND parent, DWORD id);
protected:
    HWND GetHandle() const;
    HWND GetParent() const;
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    static INT_PTR CALLBACK DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);

protected:
    static std::map<HWND, CDialogBase *> msPtrSet;
    HWND mHwnd;
    HWND mParent;
};