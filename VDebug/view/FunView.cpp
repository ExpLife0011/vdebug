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

    //mEditView.SendMsg(SCI_STYLECLEARALL, 0, 0);
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
    mEditView.SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
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
    int w1 = (int)(w * 0.5);
    int w2 = w - w1 - 5;

    mEditView.CreateView(hwnd, left1, top1, w1, hight);
    int d = mEditView.SendMsg(SCI_SETLEXER, SCLEX_CPP, 0);
    int left2 = left1 + w1 + 5;
    mStatView.CreateView(hwnd, left2, top1, w2, hight);

    CTL_PARAMS arry[] = {
        {0, mComModule, 0, 0, 1, 0},
        {0, mEditView.GetWindow(), 0, 0, (float)w1 / float(w1 + w2), 1},
        {0, mStatView.GetWindow(), (float)w1 / float(w1 + w2), 0, (float)w2 / float(w1 + w2), 1},
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

    if (id == IDC_IMPORT_BTN_CHECK || id == IDC_IMPORT_BTN_OK)
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

        if (id == IDC_IMPORT_BTN_CHECK)
        {
            DbgCtrlService::GetInstance()->TestDescStr(dll, str);
        } else if (id == IDC_IMPORT_BTN_OK)
        {
            DbgCtrlService::GetInstance()->InputDescStr(dll, str, false);
        }
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

void CFunctionView::NotifyFunCover() {
    if (IDYES == MessageBoxA(
        m_hwnd,
        "需要保存的信息和本地缓存中的数据有冲突，是否覆盖保存？",
        "数据冲突",
        MB_YESNO | MB_SETFOREGROUND | MB_ICONWARNING
        ))
    {
        COMBOBOXINFO info = { sizeof(info) };
        SendMessageA(mComModule, CB_GETCOMBOBOXINFO, 0, (LPARAM)&info);

        mstring dll = GetWindowStrA(info.hwndItem);
        mstring str = mEditView.GetText();
        DbgCtrlService::GetInstance()->InputDescStr(dll, str, true);
    }
}