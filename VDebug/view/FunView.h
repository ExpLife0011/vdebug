#ifndef FUNVIEW_VDEBUG_H_H_
#define FUNVIEW_VDEBUG_H_H_
#include <Windows.h>
#include "ViewBase.h"
#include "FunSyntaxView.h"

class CFunctionView : public CWindowBase {
public:
    CFunctionView();
    virtual ~CFunctionView();

    void ShowFunView(HWND parent);
    void SetStatText(const std::mstring &text);
    void AppendStatText(const std::mstring &text);
    void NotifyFunCover();
    void CloseFunView();
private:
    void UsingCppStyle();
    int OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp);
    int OnCommand(HWND hwnd, WPARAM wp, LPARAM lp);

private:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);

private:
    HWND mParent;
    CFunSyntaxView mEditView;
    CFunSyntaxView mStatView;
    HWND mComModule;
    HWND mBtnCheck;
    HWND mBtnOk;
    HWND mBtnCancel;
};
#endif  //FUNVIEW_VDEBUG_H_H_