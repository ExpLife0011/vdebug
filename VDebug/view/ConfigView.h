#pragma once
#include "DialogBase.h"
#include "../SyntaxHlpr/SyntaxTextView.h"
#include "CmdShowView.h"
#include "StyleConfigMgr.h"
#include <map>

class CConfigDlg : public CDialogBase {
public:
    static CConfigDlg *GetInst();
    CStyleConfig GetStyleCfg() const;

private:
    CConfigDlg();
    virtual ~CConfigDlg();

private:
    void LoadFonts() const;
    void LoadStyleConfig();
    void SetStaticBackColour(HWND hEdit, DWORD rgb);
    void ShowColourDlg();
    BOOL GetStyleCfg(StyleConfigInfo &cfg) const;
    //从RGB(11,22,33)串中获取rgb值
    BOOL GetRgbFromStr(const std::mstring &str, DWORD &rgb) const;
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnTimer(WPARAM wp, LPARAM lp);
    INT_PTR OnCtrlColour(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    struct EditColourInfo {
        DWORD mColour;
        HBRUSH mBrush;
    };

    std::map<HWND, EditColourInfo> mEditStyleMap;
    CStyleConfig mStyleCfg;
    CCmdShowView mStyleView;
    HWND mEditText1, mEditBack1;
    HWND mEditText2, mEditBack2;
    HWND mEditText3, mEditBack3;
    HWND mEditText4, mEditBack4;
    HWND mComFont, mComSize;
    HWND mBtnReset, mBtnOk;
    HWND mCbLineNum;
    HWND mEditSelColour, mEditSelAlpha;
};