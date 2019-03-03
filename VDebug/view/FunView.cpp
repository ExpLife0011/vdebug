#include <Windows.h>
#include <ComLib/winsize.h>
#include <ComLib/SyntaxDef.h>
#include "DbgCtrlService.h"
#include "FunView.h"
#include "resource.h"

CFunctionView::CFunctionView() {
}

CFunctionView::~CFunctionView() {
}

void CFunctionView::ShowFunView(HWND parent) {
    if (!IsWindow(m_hwnd))
    {
        mParent = parent;
        CreateDlg(IDD_IMPORT_FUNDEF, parent, FALSE);
    }
}

void CFunctionView::CloseFunView() {
    EndDialog(m_hwnd, 0);
    m_hwnd = NULL;
}

void CFunctionView::UsingCppStyle() {
       const char* szKeywords1=
        "asm auto break case catch class const "
        "const_cast continue default delete do double "
        "dynamic_cast else enum explicit extern false "
        "for friend goto if inline mutable "
        "namespace new operator private protected public "
        "register reinterpret_cast return signed "
        "sizeof static static_cast struct switch template "
        "this throw true try typedef typeid typename "
        "union unsigned using virtual volatile while";
    const char* szKeywords2=
        "bool char float int long short void wchar_t";
    mEditView.SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT,10);
    mEditView.SendMsg(SCI_STYLECLEARALL, 0, 0);
    mEditView.SendMsg(SCI_SETLEXER, SCLEX_CPP, 0);
    mEditView.SendMsg(SCI_SETKEYWORDS, 0, (sptr_t)szKeywords1);
    mEditView.SendMsg(SCI_SETKEYWORDS, 1, (sptr_t)szKeywords2);

    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_WORD, 0x00FF0000);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_WORD2, 0x00800080);
    mEditView.SendMsg(SCI_STYLESETBOLD, SCE_C_WORD2, TRUE);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_STRING, 0x001515A3);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_CHARACTER, 0x001515A3);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x00808080);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_COMMENT, 0x00008000);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x00008000);
    mEditView.SendMsg(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x00008000);
   
    mEditView.SendMsg(SCI_SETCARETLINEVISIBLE, TRUE, 0);
    mEditView.SendMsg(SCI_SETCARETLINEBACK, 0xb0ffff, 0);
    mEditView.SendMsg(SCI_SETTABWIDTH, 4, 0);
    mEditView.SendMsg(SCI_SETMARGINTYPEN, SC_MARGIN_NUMBER, 1);
    mEditView.SetFont("Lucida Console");
    mEditView.SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 5);
    mEditView.ShowMargin(false);

    mStatView.SetFont("Lucida Console");
    mStatView.ShowMargin(false);
    mStatView.SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
}

int CFunctionView::OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp) {
    extern HINSTANCE g_hInstance;
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    mComModule = GetDlgItem(m_hwnd, IDC_IMPORT_COM_MODULE);
    mBtnOk = GetDlgItem(m_hwnd, IDC_IMPORT_BTN_OK);
    mBtnCancel = GetDlgItem(m_hwnd, IDC_IMPORT_BTN_CANCEL);
    mBtnCheck = GetDlgItem(m_hwnd, IDC_IMPORT_BTN_CHECK);

    RECT clientRect = {0};
    GetClientRect(hwnd, &clientRect);
    RECT comRect = {0};
    GetWindowRect(mComModule, &comRect);
    MapWindowPoints(NULL, hwnd, (LPPOINT)&comRect, 2);
    RECT btnOkRect = {0};
    GetWindowRect(mBtnOk, &btnOkRect);
    MapWindowPoints(NULL, hwnd, (LPPOINT)&btnOkRect, 2);

    int left1 = 5;
    int top1 = comRect.bottom + 5;
    int right1 = clientRect.right - 5;
    int bottom1 = btnOkRect.top - 5;

    int w = right1 - left1;
    int hight = (bottom1 - top1);
    int h1 = (hight / 10 * 7);
    int h2 = (hight / 10 * 3 - 5);

    mEditView.CreateView(hwnd, left1, top1, w, h1);
    int d = mEditView.SendMsg(SCI_SETLEXER, SCLEX_CPP, 0);
    int top2 = top1 + h1 + 5;
    int left2 = left1;
    mStatView.CreateView(hwnd, left1, top2, w, h2);

    CTL_PARAMS arry[] = {
        {0, mComModule, 0, 0, 1, 0},
        {0, mEditView.GetWindow(), 0, 0, 1, (float)h1 / (float)(h1 + h2)},
        {0, mStatView.GetWindow(), 0, (float)h1 / (float)(h1 + h2), 1, (float)h2 / (float)(h1 + h2)},
        {0, mBtnCheck, 1, 1, 0, 0},
        {0, mBtnOk, 1, 1, 0, 0},
        {0, mBtnCancel, 1, 1, 0, 0}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));

    //1005 * 573
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 1000, 573, SWP_NOMOVE);

    RECT rect = {0};
    GetWindowRect(hwnd, &rect);
    SetWindowRange(hwnd, rect.right - rect.left, rect.bottom - rect.top, 0, 0);

    UsingCppStyle();
    return 0;
}

int CFunctionView::OnCommand(HWND hwnd, WPARAM wp, LPARAM lp) {
    int id = LOWORD(wp);

    if (id == IDC_IMPORT_BTN_CHECK)
    {
        COMBOBOXINFO info = { sizeof(info) };
        SendMessageA(mComModule, CB_GETCOMBOBOXINFO, 0, (LPARAM)&info);

        mstring dll = GetWindowStrA(info.hwndItem);
        mstring str = mEditView.GetText();

        dll.trim();
        if (dll.empty())
        {
            mStatView.SetText(SCI_LABEL_DEFAULT, "需要填写模块名称");
            return 0;
        }

        str.trim();
        if (str.empty())
        {
            mStatView.SetText(SCI_LABEL_DEFAULT, "没有需要分析的内容");
            return 0;
        }
        DbgCtrlService::GetInstance()->TestDescStr(dll, str);
    }
    return 0;
}

LRESULT CFunctionView::OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) {
    switch (uMsg) {
        case WM_INITDIALOG:
            {
                OnInitDlg(hwnd, wp, lp);
            }
            break;
        case WM_COMMAND:
            {
                OnCommand(hwnd, wp, lp);
            }
            break;
        case WM_CLOSE:
            CloseFunView();
            break;
        default:
            break;
    }
    return 0;
}

void CFunctionView::SetStatText(const mstring &text) {
    mStatView.SetText(SCI_LABEL_DEFAULT, text);
}