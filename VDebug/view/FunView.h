#ifndef FUNVIEW_VDEBUG_H_H_
#define FUNVIEW_VDEBUG_H_H_
#include <Windows.h>
#include "ViewBase.h"
#include "SyntaxView.h"

class CFunctionView : public CWindowBase {
public:
    CFunctionView();
    virtual ~CFunctionView();

    void ShowFunView(HWND parent);
    void CloseFunView();
private:
    void UsingCppStyle();
    int OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp);

private:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);

private:
    HWND mParent;
    SyntaxView mEditView;
    SyntaxView mStatView;
    HWND mComModule;
    HWND mBtnCheck;
    HWND mBtnOk;
    HWND mBtnCancel;
};
#endif  //FUNVIEW_VDEBUG_H_H_