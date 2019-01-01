#include <Windows.h>
#include <commctrl.h>
#include <ComLib/ComLib.h>
#include "ProcView.h"
#include "DbgCtrlService.h"
#include "../resource.h"

enum ProcSortType
{
    em_sortby_init,
    em_sortby_pid,
    em_sortby_name,
    em_sortby_starttime,
    em_sortby_path
};

static ProcSortType gs_eSortBy = em_sortby_init;
#define MSG_REFUSH      (WM_USER + 5001)

map<ustring, HICON> CProcSelectView::ms_peIcon;

BOOL CProcSelectView::SetFont(HFONT hFont)
{
    return TRUE;
}

BOOL CProcSelectView::Refush()
{
    return TRUE;
}

BOOL CProcSelectView::Create()
{
    return TRUE;
}

VOID CProcSelectView::CalWidthByColumns() const
{
    int iWidthTotal = 0;
    int iColumnWidth = 0;
    int iItm = 0;
    while((iColumnWidth = (int)SendMessageW(m_hProcList, LVM_GETCOLUMNWIDTH, iItm++, 0)) > 0)
    {
        iWidthTotal += iColumnWidth;
    }

    RECT rtDlg;
    GetWindowRect(m_hwnd, &rtDlg);
    RECT rtList;
    GetWindowRect(m_hProcList, &rtList);

    int iW = (rtList.left - rtDlg.left);
    if ((rtDlg.right - rtDlg.left) < (iWidthTotal + iW * 2 + 30))
    {
        SetWindowPos(m_hProcList, 0, 0, 0, iWidthTotal + 30, rtList.bottom - rtList.top, SWP_NOZORDER | SWP_NOMOVE);
        SetWindowPos(m_hwnd, 0, 0, 0, iWidthTotal + 30 + iW * 2, rtDlg.bottom - rtDlg.top, SWP_NOZORDER | SWP_NOMOVE);
    }
}

void CProcSelectView::InitListCtrl()
{
    m_hImageList = ImageList_Create(
        15,
        16,
        ILC_COLOR32 | ILC_MASK,
        5,
        1
        );

    SendMessageW(
        m_hProcList,
        LVM_SETIMAGELIST,
        LVSIL_SMALL,
        (LPARAM)m_hImageList
        );

    ListView_SetExtendedListViewStyle(m_hProcList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    LVCOLUMNW col;
    memset(&col, 0x00, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 150;
    col.pszText = L"进程";
    SendMessageW(m_hProcList, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);

    col.cx = 60;
    col.pszText = L"Pid";
    SendMessageW(m_hProcList, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);


    col.cx = 90;
    col.pszText = L"启动时间";
    SendMessageW(m_hProcList, LVM_INSERTCOLUMNW, 3, (LPARAM)&col);
}

ustring CProcSelectView::GetSearchStr(ProcShowInfo *ptr) {
    ptr->m_indexStr = FormatW(L"%ls_%d", ptr->info.procPath.c_str(), ptr->info.procPid);
    return ptr->m_indexStr;
}

int CProcSelectView::GetFileIco(const ustring &wstrFile)
{
    if (m_icoIndex.end() != m_icoIndex.find(wstrFile))
    {
        return m_icoIndex[wstrFile];
    }

    map<ustring, HICON> ::const_iterator it = ms_peIcon.find(wstrFile);
    HICON icon = NULL;
    if (it != ms_peIcon.end())
    {
        icon = it->second;
    } else {
        CoInitialize(NULL);
        PVOID p = DisableWow64Red();
        SHFILEINFOW info = {0};
        SHGetFileInfoW(wstrFile.c_str(), 0, &info, sizeof(info), SHGFI_ICON);
        RevertWow64Red(p);
        CoUninitialize();

        if (!info.hIcon)
        {
            return -1;
        }
        icon = info.hIcon;
        ms_peIcon[wstrFile] = icon;
    }

    int iIdex = ImageList_AddIcon(m_hImageList, icon);
    m_icoIndex[wstrFile] = iIdex;
    return iIdex;
}

void CProcSelectView::DeleteFromSet(vector<ProcShowInfo *> &procSet, const list<DWORD> &killed, bool freeMem) {
    if (killed.size() > 0)
    {
        for (vector<ProcShowInfo *>::const_iterator ij = procSet.begin() ; ij != procSet.end() ;)
        {
            ProcShowInfo *ptr = *ij;
            bool del = false;
            for (list<DWORD>::const_iterator ij2 = killed.begin() ; ij2 != killed.end() ;ij2++)
            {
                if (ptr->info.procUnique == *ij2)
                {
                    del = true;
                    break;
                }
            }

            if (del)
            {
                if (freeMem)
                {
                    delete *ij;
                }

                ij = procSet.erase(ij);
            } else {
                ij++;
            }
        }
    }
}

void CProcSelectView::OnProcChanged(const list<ProcMonInfo> &added, const list<DWORD> &killed) {
    CScopedLocker lock(this);
    list<ProcMonInfo>::const_iterator it;
    if (!IsWindow(m_hwnd))
    {
        return;
    }

    for (it = added.begin() ; it != added.end() ; it++)
    {
        ProcShowInfo *newProc = new ProcShowInfo();
        newProc->info = *it;
        newProc->m_dwIcoIdex = GetFileIco(it->procPath);
        newProc->m_procShow = PathFindFileNameW(newProc->info.procPath.c_str());
        GetSearchStr(newProc);

        m_procAll.push_back(newProc);

        if (m_searchStr.empty() || ustring::npos != newProc->m_indexStr.find_in_rangei(m_searchStr))
        {
            m_procShow.push_back(newProc);
        }
    }

    DeleteFromSet(m_procShow, killed, false);
    DeleteFromSet(m_procAll, killed, true);

    PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_procShow.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_procShow.size());
    SetWindowTextW(m_hEditStatus, FormatW(L"进程数量:%d 符合过滤条件:%d", m_procAll.size(), m_procShow.size()).c_str());
}

void CProcSelectView::DeleteProcCache() {
    CScopedLocker lock(this);
    for (vector<ProcShowInfo *>::const_iterator it = m_procAll.begin() ; it != m_procAll.end() ; it++) {
        delete *it;
    }
    m_procAll.clear();
}

INT_PTR CProcSelectView::OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp)
{
    extern HINSTANCE g_hInstance;
    m_hParent = GetParent(hwnd);
    CentreWindow(m_hwnd, m_hParent);
    m_hProcList = GetDlgItem(hwnd, IDC_PROC_LIST_PROC);
    m_hEditInfo = GetDlgItem(hwnd, IDC_PROC_EDT_INFO);
    m_hEditFlt = GetDlgItem(hwnd, IDC_PROC_EDT_FLT);
    m_hEditStatus = GetDlgItem(hwnd, IDC_PROC_EDT_STATUS);

    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    InitListCtrl();

    CTL_PARAMS vArry[] =
    {
        {IDC_PROC_LIST_PROC, NULL, 0, 0, 0, 1},
        {IDC_PROC_EDT_INFO, NULL, 0, 0, 1, 1},
        {IDC_PROC_EDT_FLT, NULL, 0, 0, 1, 0},
        {IDC_PROC_EDT_STATUS, NULL, 0, 1, 1, 0},

        {IDC_PROC_BTN_ATTACH, NULL, 1, 0, 0, 0},
        {IDC_PROC_BTN_OPEN, NULL, 1, 0, 0, 0}
    };
    SetCtlsCoord(hwnd, vArry, RTL_NUMBER_OF(vArry));

    RECT rt = {0};
    GetWindowRect(m_hwnd, &rt);
    SetWindowRange(hwnd, rt.right - rt.left, rt.bottom - rt.top, 0, 0);
    
    m_searchStr.clear();
    m_procShow.clear();
    DeleteProcCache();
    DbgCtrlService::GetInstance()->StartProcMon();
    return 0;
}

VOID CProcSelectView::OnGetListCtrlDisplsy(IN OUT NMLVDISPINFOW* ptr)
{
    int iItm = ptr->item.iItem;
    int iSub = ptr->item.iSubItem;

    do
    {
        CScopedLocker lock(this);
        ProcShowInfo *pInfo = NULL;
        {
            if (iItm >= (int)m_procShow.size())
            {
                break;
            }

            pInfo = m_procShow[iItm];
        }

        if (0 == iSub)
        {
            ptr->item.iImage = pInfo->m_dwIcoIdex;
        }

        static ustring s_wstrBuf;
        switch (iSub)
        {
        case  0:
            s_wstrBuf = pInfo->m_procShow;
            break;
        case 1:
            s_wstrBuf.format(L"%d", pInfo->info.procPid);
            break;
        case 2:
            s_wstrBuf = pInfo->info.startTime;
            break;
        }

        ptr->item.pszText = (LPWSTR)s_wstrBuf.c_str();
    } while (FALSE);
}

VOID CProcSelectView::OnListColumnClick(NMLISTVIEW* ptr)
{
}

void CProcSelectView::OnListItemChanged(HWND hwnd, WPARAM wp, LPARAM lp) {
    NMLISTVIEW *ptr1 = (NMLISTVIEW *)lp;
    int it = ptr1->iItem;
    CScopedLocker lock(this);

    if (it >= (int)m_procShow.size() || it < 0)
    {
        return;
    }
    ProcShowInfo *ptr2 = m_procShow[it];

    mstring show;
    show += "进程路径\r\n";
    show += (WtoA(ptr2->info.procPath) + "\r\n\r\n");

    show += "进程描述\r\n";
    if (ptr2->info.procDesc.size())
    {
        show += (WtoA(ptr2->info.procDesc) + "\r\n\r\n");
    } else {
        show += "Nothing\r\n\r\n";
    }

    show += (FormatA("session %d", ptr2->info.sessionId) + "\r\n\r\n");

    show += "进程User\r\n";
    show += (WtoA(ptr2->info.procUser) + "\r\n\r\n");

    show += "进程Sid\r\n";
    show += (WtoA(ptr2->info.procUserSid) + "\r\n\r\n");

    show += "进程命令行\r\n";
    show += (WtoA(ptr2->info.procCmd) + "\r\n\r\n");
    SetWindowTextA(m_hEditInfo, show.c_str());
}

INT_PTR CProcSelectView::OnNotify(HWND hwnd, WPARAM wp, LPARAM lp)
{
    switch (((LPNMHDR) lp)->code)
    {
    case LVN_GETDISPINFO:
        {
            NMLVDISPINFO* ptr = NULL;
            ptr = (NMLVDISPINFOW *)lp;

            if (ptr->hdr.hwndFrom == m_hProcList)
            {
                OnGetListCtrlDisplsy(ptr);
            }
        }
        break;
    case  LVN_ITEMCHANGED:
        {
            OnListItemChanged(hwnd, wp, lp);
        }
        break;
    case LVN_COLUMNCLICK:
        {
            NMLISTVIEW *ptr = (NMLISTVIEW *)lp;
            if (ptr->hdr.hwndFrom == m_hProcList)
            {
                OnListColumnClick(ptr);
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

INT_PTR CProcSelectView::OnCommand(HWND hwnd, WPARAM wp, LPARAM lp)
{
    DWORD dwId = LOWORD(wp);
    if (IDC_BTN_REFUSH == dwId)
    {
    }

    else if (IDC_BTN_ATTACH == dwId)
    {
        int iSelect = (int)SendMessageW(m_hProcList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
        if (-1 == iSelect)
        {
            SetWindowTextW(m_hEditStatus, L"请先选择一个进程");
        }
        else
        {
            //CScopedLocker lock(this);
            //m_dwSelectPid = m_vProcInfo[iSelect]->m_dwPid;
            //PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
        }
    }
    return 0;
}

INT_PTR CProcSelectView::OnClose(HWND hwnd, WPARAM wp, LPARAM lp)
{
    ImageList_Destroy(m_hImageList);
    m_hImageList = NULL;

    DbgCtrlService::GetInstance()->StopProcMon();
    m_icoIndex.clear();
    CScopedLocker lock(this);
    m_procShow.clear();
    DeleteProcCache();
    return 0;
}

LRESULT CProcSelectView::OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        OnInitDlg(hwnd, wp, lp);
        break;
    case WM_NOTIFY:
        OnNotify(hwnd, wp, lp);
        break;
    case  WM_COMMAND:
        OnCommand(hwnd, wp, lp);
        break;
    case WM_CLOSE:
        {
            OnClose(hwnd, wp, lp);
            EndDialog(hwnd, 0);
        }
        break;
    }
    return 0;
}