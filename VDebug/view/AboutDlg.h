#pragma once
#include <Windows.h>
#include "DialogBase.h"

class CAboutDlg : public CDialogBase {
public:
    CAboutDlg();
    virtual ~CAboutDlg();

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    HBITMAP mBmp1;
    HBITMAP mBmp2;
    HWND mLinkVdebug;
    HWND mLinkHomePage;
};