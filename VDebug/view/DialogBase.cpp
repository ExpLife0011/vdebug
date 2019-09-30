#include "DialogBase.h"

using namespace std;

map<HWND, CDialogBase *> CDialogBase::msPtrSet;

CDialogBase::CDialogBase() {
}

CDialogBase::~CDialogBase() {
}

int CDialogBase::DoModule(HWND parent, DWORD id) {
    mParent = parent;
    return DialogBoxParamA(NULL, MAKEINTRESOURCEA(id), parent, DialogProc, (LPARAM)this);
}

BOOL CDialogBase::CreateDlg(HWND parent, DWORD id) {
    mParent = parent;
    mHwnd = CreateDialogParamA(NULL, MAKEINTRESOURCEA(id), parent, DialogProc, (LPARAM)this);
    return (TRUE == IsWindow(mHwnd));
}

HWND CDialogBase::GetHandle() const {
    return mHwnd;
}

INT_PTR CDialogBase::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CDialogBase::DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_INITDIALOG)
    {
        CDialogBase *ptr = (CDialogBase *)lp;
        msPtrSet[hdlg] = ptr;
        ptr->mHwnd = hdlg;
    }

    map<HWND, CDialogBase*>::iterator it = msPtrSet.find(hdlg);
    if (it == msPtrSet.end())
    {
        return 0;
    }

    INT_PTR result = it->second->MessageProc(msg, wp, lp);
    if (msg == WM_DESTROY)
    {
        msPtrSet.erase(hdlg);
    }
    return result;
}