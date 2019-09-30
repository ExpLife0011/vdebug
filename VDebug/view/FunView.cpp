#include <Windows.h>
#include <ComLib/winsize.h>
#include "../SyntaxHlpr/SyntaxDef.h"
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
            mStatView.SetText(LABEL_DBG_DEFAULT, "需要填写模块名称");
            return 0;
        }

        str.trim();
        if (str.empty())
        {
            mStatView.SetText(LABEL_DBG_DEFAULT, "没有需要分析的内容");
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
    mStatView.SetText(LABEL_DBG_DEFAULT, text);
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