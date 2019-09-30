#include "ConfigView.h"
#include "../../ComLib/ComUtil.h"
#include "../../ComLib/StrUtil.h"
#include "../../ComLib/deelx.h"
#include "../resource.h"
#include "MainView.h"

using namespace std;
#define TIMER_UPDATE_STYLE      (WM_USER + 1133)

CConfigDlg *CConfigDlg::GetInst() {
    static CConfigDlg *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CConfigDlg();
    }
    return sPtr;
}

CConfigDlg::CConfigDlg() {
}

CConfigDlg::~CConfigDlg() {
}

CStyleConfig CConfigDlg::GetStyleCfg() const {
    return mStyleCfg;
}

void CConfigDlg::SetStaticBackColour(HWND hEdit, DWORD rgb) {
    map<HWND, EditColourInfo>::const_iterator it = mEditStyleMap.find(hEdit);
    if (it != mEditStyleMap.end())
    {
        DeleteObject((HGDIOBJ)it->second.mBrush);
    }

    EditColourInfo info;
    info.mColour = rgb, info.mBrush = CreateSolidBrush(rgb);
    mEditStyleMap[hEdit] = info;
    InvalidateRect(hEdit, NULL, TRUE);
}

void CConfigDlg::LoadFonts() const {
    struct LocalFontProc {
        static int CALLBACK FontEnumProc(CONST LOGFONTA *p1, CONST TEXTMETRICA *p2, DWORD type, LPARAM param)
        {
            //等宽字体列表
            LPCSTR fontSet[] = {
                "BatangChe", "Courier", "Courier New", "DotumChe",
                "Fixedsys", "GulimChe", "GungsuhChe", "Lucida Console",
                "Lucida Sans Typewriter", "MingLiU", "MS Gothic", "MS Mincho",
                "Terminal", "仿宋_GB2312", "楷体_GB2312", "隶书",
                "宋体-方正超大字符集", "新宋体", "幼圆", "宋体", "Source Code Pro"
            };

            //dp("test font, name:%hs, p1:%d, p2:%d", p1->lfFaceName, p1->lfCharSet, p1->lfWidth);
            set<mstring> *ptr = (set<mstring> *)param;
            for (int i = 0 ; i < RTL_NUMBER_OF(fontSet) ; i++)
            {
                if (0 == strcmp(p1->lfFaceName, fontSet[i]))
                {
                    ptr->insert(p1->lfFaceName);
                    return 1;
                }
            }
            return 1;
        }
    };

    LOGFONTA ft = {0};
    ft.lfCharSet = DEFAULT_CHARSET;

    set<mstring> fontSet;
    EnumFontFamiliesExA(GetWindowDC(mHwnd), &ft, LocalFontProc::FontEnumProc, (LPARAM)&fontSet, 0);

    for (set<mstring>::const_iterator it = fontSet.begin() ; it != fontSet.end() ; it++)
    {
        SendMessageA(mComFont, CB_INSERTSTRING, -1, (LPARAM)it->c_str());
    }

    for (int i = 5 ; i < 23 ; i++)
    {
        SendMessageA(mComSize, CB_INSERTSTRING, -1, (LPARAM)FormatA("%d", i).c_str());
    }
}

void CConfigDlg::LoadStyleConfig() {
    StyleConfigInfo cfg = mStyleCfg.GetStyleConfig();
    mstring selStr = FormatA("RGB(%d,%d,%d)", (int)GetRValue(cfg.mSelColour), (int)GetGValue(cfg.mSelColour), (int)GetBValue(cfg.mSelColour));
    SetStaticBackColour(mEditSelColour, cfg.mSelColour);
    SetWindowTextA(mEditSelColour, selStr.c_str());
    SetWindowTextA(mEditSelAlpha, FormatA("%d", cfg.mSelAlpha).c_str());

    for (map<mstring, StyleConfigNode>::const_iterator it = cfg.mCfgSet.begin() ; it != cfg.mCfgSet.end() ; it++)
    {
        const StyleConfigNode &info = it->second;
        mstring textStr1 = FormatA("RGB(%d,%d,%d)", (int)GetRValue(info.mRgbText), (int)GetGValue(info.mRgbText), (int)GetBValue(info.mRgbText));
        mstring textStr2 = FormatA("RGB(%d,%d,%d)", (int)GetRValue(info.mRgbBack), (int)GetGValue(info.mRgbBack), (int)GetBValue(info.mRgbBack));
        if (info.mStyleDesc == "default")
        {
            SetStaticBackColour(mEditText1, info.mRgbText);
            SetWindowTextA(mEditText1, textStr1.c_str());
            SetStaticBackColour(mEditBack1, info.mRgbBack);
            SetWindowTextA(mEditBack1, textStr2.c_str());
        } else if (info.mStyleDesc == "cmdSend")
        {
            SetStaticBackColour(mEditText2, info.mRgbText);
            SetWindowTextA(mEditText2, textStr1.c_str());
            SetStaticBackColour(mEditBack2, info.mRgbBack);
            SetWindowTextA(mEditBack2, textStr2.c_str());
        } else if (info.mStyleDesc == "cmdRecv")
        {
            SetStaticBackColour(mEditText3, info.mRgbText);
            SetWindowTextA(mEditText3, textStr1.c_str());
            SetStaticBackColour(mEditBack3, info.mRgbBack);
            SetWindowTextA(mEditBack3, textStr2.c_str());
        } else if (info.mStyleDesc == "cmdHight")
        {
            SetStaticBackColour(mEditText4, info.mRgbText);
            SetWindowTextA(mEditText4, textStr1.c_str());
            SetStaticBackColour(mEditBack4, info.mRgbBack);
            SetWindowTextA(mEditBack4, textStr2.c_str());
        }
    }

    int pos = SendMessageA(mComFont, CB_FINDSTRING, -1, (LPARAM)cfg.mFontName.c_str());
    SendMessageA(mComFont, CB_SETCURSEL, pos, 0);

    pos = SendMessageA(mComSize, CB_FINDSTRING, -1, (LPARAM)FormatA("%d", cfg.mFontSize).c_str());
    SendMessageA(mComSize, CB_SETCURSEL, pos, 0);
}

void CConfigDlg::ShowColourDlg() {
    CHOOSECOLORA cc;
    static COLORREF acrCustClr[16];
    static DWORD rgbCurrent;

    // Initialize CHOOSECOLOR 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = mHwnd;
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = rgbCurrent;
    cc.Flags = CC_ANYCOLOR | CC_PREVENTFULLOPEN;

    ChooseColorA(&cc);
}

INT_PTR CConfigDlg::OnInitDialog(WPARAM wp, LPARAM lp) {
    CentreWindow(mHwnd, NULL);

    mEditText1 = GetDlgItem(mHwnd, IDC_EDT_TEXT1);
    mEditBack1 = GetDlgItem(mHwnd, IDC_EDT_BACK1);
    mEditText2 = GetDlgItem(mHwnd, IDC_EDT_TEXT2);
    mEditBack2 = GetDlgItem(mHwnd, IDC_EDT_BACK2);
    mEditText3 = GetDlgItem(mHwnd, IDC_EDT_TEXT3);
    mEditBack3 = GetDlgItem(mHwnd, IDC_EDT_BACK3);
    mEditText4 = GetDlgItem(mHwnd, IDC_EDT_TEXT4);
    mEditBack4 = GetDlgItem(mHwnd, IDC_EDT_BACK4);
    mComFont = GetDlgItem(mHwnd, IDC_COM_FONT);
    mComSize = GetDlgItem(mHwnd, IDC_COM_SIZE);
    mBtnReset = GetDlgItem(mHwnd, IDC_BTN_CFG_RESET);
    mBtnOk = GetDlgItem(mHwnd, IDC_BTN_CFG_OK);
    mCbLineNum = GetDlgItem(mHwnd, IDC_CK_LINENUM);
    mEditSelColour = GetDlgItem(mHwnd, IDC_EDT_SEL_RGB);
    mEditSelAlpha = GetDlgItem(mHwnd, IDC_EDT_SEL_ALPHA);

    RECT rtFont = {0};
    GetWindowRect(GetDlgItem(mHwnd, IDC_ST_OTHER), &rtFont);
    MapWindowPoints(NULL, mHwnd, (LPPOINT)&rtFont, 2);

    int x = rtFont.left;
    int w = rtFont.right - rtFont.left;
    RECT rtButton = {0};
    GetWindowRect(GetDlgItem(mHwnd, IDC_BTN_CFG_OK), &rtButton);
    MapWindowPoints(NULL, mHwnd, (LPPOINT)&rtButton, 2);

    int y = rtFont.bottom + 5;
    int h = rtButton.top - 5 - y;
    mStyleView.CreateView(mHwnd, x, y, w, h);
    mStyleView.InitShowView();
    //mStyleCfg = CMainView::GetInst()->GetStyleCfg();
    mStyleView.LoadUserCfg(mStyleCfg);

    LoadFonts();
    LoadStyleConfig();
    mStyleView.AppendText(LABEL_CMD_SEND, "发送数据 >> ps\n");
    mStyleView.AppendText(LABEL_CMD_RECV, "收到数据 >> explorer.exe\n");
    SetFocus(mBtnOk);
    SetTimer(mHwnd, TIMER_UPDATE_STYLE, 50, NULL);
    return 0;
}

INT_PTR CConfigDlg::OnCtrlColour(WPARAM wp, LPARAM lp) {
    HWND hEdit = (HWND)lp;
    HDC hEditDC = (HDC)wp;

    map<HWND, EditColourInfo>::const_iterator it = mEditStyleMap.find(hEdit);
    if (it == mEditStyleMap.end())
    {
        return 0;
    }

    SetBkColor(hEditDC, it->second.mColour);

    byte r = it->second.mColour & 0xff;
    byte g = (it->second.mColour >> 8) & 0xff;
    byte b = (it->second.mColour >> 16) & 0xff;
    //编辑框字体颜色取最大色差
    r = 255 - r, g = 255 - g; b = 255 - b;
    SetTextColor(hEditDC, RGB(r, g, b));
    return (INT_PTR)(it->second.mBrush);
}

INT_PTR CConfigDlg::OnCommand(WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_BTN_CFG_RESET)
    {
        mStyleCfg.SetDefault();
        LoadStyleConfig();
        mStyleView.LoadUserCfg(mStyleCfg);
    } else if (id == IDC_BTN_CFG_OK)
    {
        if (IDOK == MessageBoxW(mHwnd, L"确认保存当前的配置?", L"配置生效", MB_OKCANCEL))
        {
            StyleConfigInfo cfg;
            if (GetStyleCfg(cfg))
            {
                mStyleCfg.UpdateStyleConfig(cfg);

                //mstring cfgPath = WtoA(CCmdInstall::GetInst()->GetStyleCfgPath());
                //mStyleCfg.SaveCache(cfgPath);
                EndDialog(mHwnd, 1);
            }
        }
    }
    return 0;
}

//从RGB(11,22,33)串中获取rgb值
BOOL CConfigDlg::GetRgbFromStr(const mstring &str, DWORD &rgb) const {
    mstring tmp = str;
    tmp.trim();
    tmp.delchar(' ');

    const char *reg1 = "^RGB\\([1-2]{0,1}[0-9]{1,2},[1-2]{0,1}[0-9]{1,2},[1-2]{0,1}[0-9]{1,2}\\)$";
    CRegexpT<char> regexParser(reg1);
    MatchResult regexResult = regexParser.Match(tmp.c_str());
    if (!regexResult.IsMatched())
    {
        return FALSE;
    }

    size_t pos0 = 0, pos1 = 0, pos2 = 0, pos3 = 0;
    pos0 = tmp.find("(") + 1;
    pos1 = tmp.find(",", pos0) + 1;
    pos2 = tmp.find(",", pos1) + 1;
    pos3 = tmp.find(")", pos2);
    byte r = atoi(tmp.substr(pos0, pos1 - pos0).c_str());
    byte g = atoi(tmp.substr(pos1, pos2 - pos1).c_str());
    byte b = atoi(tmp.substr(pos2, pos3 - pos2).c_str());
    rgb = RGB(r, g, b);
    return TRUE;
}

BOOL CConfigDlg::GetStyleCfg(StyleConfigInfo &info) const {
    StyleConfigNode node;
    BOOL result = FALSE;

    do
    {
        node.mStyleDesc = "default";
        node.mSyntaxStyle = STYLE_CMD_DEFAULT;
        if (!GetRgbFromStr(GetWindowStr(mEditText1), node.mRgbText) || !GetRgbFromStr(GetWindowStr(mEditBack1), node.mRgbBack))
        {
            break;
        }
        info.mCfgSet[node.mStyleDesc] = node;

        node.mStyleDesc = "cmdSend";
        node.mSyntaxStyle = STYLE_CMD_SEND;
        if (!GetRgbFromStr(GetWindowStr(mEditText2), node.mRgbText) || !GetRgbFromStr(GetWindowStr(mEditBack2), node.mRgbBack))
        {
            break;
        }
        info.mCfgSet[node.mStyleDesc] = node;

        node.mStyleDesc = "cmdRecv";
        node.mSyntaxStyle = STYLE_CMD_RECV;
        if (!GetRgbFromStr(GetWindowStr(mEditText3), node.mRgbText) || !GetRgbFromStr(GetWindowStr(mEditBack3), node.mRgbBack))
        {
            break;
        }
        info.mCfgSet[node.mStyleDesc] = node;

        node.mStyleDesc = "cmdHight";
        node.mSyntaxStyle = STYLE_CMD_HIGHT;
        if (!GetRgbFromStr(GetWindowStr(mEditText4), node.mRgbText) || !GetRgbFromStr(GetWindowStr(mEditBack4), node.mRgbBack))
        {
            break;
        }
        info.mCfgSet[node.mStyleDesc] = node;

        if (TRUE == SendMessageA(mComFont, CB_GETDROPPEDSTATE, 0, 0))
        {
            break;
        }

        int sel1 = SendMessageA(mComFont, CB_GETCURSEL, 0, 0);
        char buff[128];
        SendMessageA(mComFont, CB_GETLBTEXT, sel1, (LPARAM)buff);
        info.mFontName = buff;

        if (TRUE == SendMessageA(mComSize, CB_GETDROPPEDSTATE, 0, 0))
        {
            break;
        }

        int sel2 = SendMessageA(mComSize, CB_GETCURSEL, 0, 0);
        SendMessageA(mComSize, CB_GETLBTEXT, sel2, (LPARAM)buff);
        info.mFontSize = atoi(buff);
        dp(L"sel1:%d, sel2:%d", sel1, sel2);

        if (BST_CHECKED == SendMessageA(mCbLineNum, BM_GETCHECK, 0, 0))
        {
            info.mLineNum = true;
        } else {
            info.mLineNum = false;
        }

        if (!GetRgbFromStr(GetWindowStr(mEditSelColour), info.mSelColour))
        {
            break;
        }

        info.mSelAlpha = atoi(GetWindowStr(mEditSelAlpha).c_str());
        result = TRUE;
    } while (FALSE);
    return result;
}

INT_PTR CConfigDlg::OnTimer(WPARAM wp, LPARAM lp) {
    if (TIMER_UPDATE_STYLE == wp)
    {
        StyleConfigInfo cfg;
        if (GetStyleCfg(cfg) && (cfg != mStyleCfg.GetStyleConfig()))
        {
            mStyleCfg.UpdateStyleConfig(cfg);
            mStyleView.LoadUserCfg(mStyleCfg);
        }
    }
    return 0;
}

INT_PTR CConfigDlg::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG:
            OnInitDialog(wp, lp);
            break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            break;
        case WM_CTLCOLOREDIT:
            return OnCtrlColour(wp, lp);
        case WM_TIMER:
            OnTimer(wp, lp);
            break;
        case WM_CLOSE:
            EndDialog(mHwnd, 0);
            break;
        default:
            break;
    }
    return 0;
}
