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

    col.cx = 320;
    col.pszText = L"进程路径";
    SendMessageW(m_hProcList, LVM_INSERTCOLUMNW, 4, (LPARAM)&col);
}

/*
DWORD CProcSelectView::RefushThread(LPVOID pParam)
{
    CProcSelectView *ptr = (CProcSelectView *)pParam;
    HANDLE vArry[] = {ptr->m_hNotify, ptr->m_hExit};
    //初始化com，防止出现部分pe文件ico获取不到的问题
    CoInitialize(NULL);
    while (TRUE)
    {
        ptr->RefushProc();
        DWORD dwRet = WaitForMultipleObjects(RTL_NUMBER_OF(vArry), vArry, FALSE, 2000);
        if ((WAIT_OBJECT_0 + 1) == dwRet)
        {
            break;
        }
    }
    CoUninitialize();
    return 0;
}
*/

int CProcSelectView::GetFileIco(const ustring &wstrFile)
{
    if (m_PeIco.end() != m_PeIco.find(wstrFile))
    {
        return m_PeIco[wstrFile];
    }

    CoInitialize(NULL);
    SHFILEINFOW info = {0};
    HICON hIcon = NULL;
    if (!SHGetFileInfoW(wstrFile.c_str(), 0, &info, sizeof(info), SHGFI_ICON))
    {
        CoUninitialize();
        int e = GetLastError();
        return -1;
    }

    hIcon = info.hIcon;
    int iIdex = ImageList_AddIcon(m_hImageList, hIcon);
    m_PeIco[wstrFile] = iIdex;
    //m_vIcons.insert(info.hIcon);
    CoUninitialize();
    return iIdex;
}

void CProcSelectView::OnProcChanged(const list<ProcMonInfo *> &added, const list<DWORD> &killed) {
    CScopedLocker lock(this);
    list<ProcMonInfo *>::const_iterator it;
    if (!IsWindow(m_hwnd))
    {
        for (it = added.begin() ; it != added.end() ; it++)
        {
            delete *it;
        }
        return;
    }

    for (it = added.begin() ; it != added.end() ; it++)
    {
        m_procAll.push_back(*it);
    }

    if (killed.size() > 0)
    {
        for (vector<ProcMonInfo *>::const_iterator ij = m_procAll.begin() ; ij != m_procAll.end() ;)
        {
            ProcMonInfo *ptr = *ij;
            bool del = false;
            for (list<DWORD>::const_iterator ij2 = killed.begin() ; ij2 != killed.end() ;ij2++)
            {
                if (ptr->procUnique == *ij2)
                {
                    del = true;
                    break;
                }
            }

            if (del)
            {
                ij = m_procAll.erase(ij);
            } else {
                ij++;
            }
        }
    }
}

void CProcSelectView::DeleteProcCache() {
    CScopedLocker lock(this);
    for (vector<ProcMonInfo *>::const_iterator it = m_procAll.begin() ; it != m_procAll.end() ; it++) {
        delete *it;
    }
    m_procAll.clear();
}

/*
void CProcSelectView::RefushProc()
{
    PVOID ptr = DisableWow64Red();
    m_vTempInfo.clear();
    IterateProcW(ProcHandlerW, this);
    RevertWow64Red(ptr);
    {
        CScopedLocker lock(this);
        //m_vProcInfo = m_vTempInfo;
        //增加的
        for (vector<ProcInfo *>::const_iterator it = m_vTempInfo.begin() ; it != m_vTempInfo.end() ; it++)
        {
            ProcInfo *ptr = *it;
            ustring wstr = GetProcUnique(*ptr);
            if (IsProcInCache(wstr))
            {
                delete ptr;
                continue;
            }
            else
            {
                InsertUnique(wstr);
                vector<ProcInfo *>::iterator it2;
                int idex = 0;
                switch (gs_eSortBy)
                {
                case  em_sortby_init:
                    {
                        m_vProcInfo.push_back(ptr);
                    }
                    break;
                case  em_sortby_name:
                    {
                        for (it2 = m_vProcInfo.begin(), idex = 0 ; it2 != m_vProcInfo.end() ; it2++, idex++)
                        {
                            if (ptr->m_wstrShowName < (*it2)->m_wstrShowName)
                            {
                                m_vProcInfo.insert(m_vProcInfo.begin() + idex, ptr);
                                break;
                            }
                        }

                        if (idex == m_vProcInfo.size())
                        {
                            m_vProcInfo.push_back(ptr);
                        }
                    }
                    break;
                case  em_sortby_pid:
                    {
                        for (it2 = m_vProcInfo.begin(), idex = 0 ; it2 != m_vProcInfo.end() ; it2++, idex++)
                        {
                            if (ptr->m_dwPid < (*it2)->m_dwPid)
                            {
                                m_vProcInfo.insert(m_vProcInfo.begin() + idex, ptr);
                                break;
                            }
                        }

                        if (idex == m_vProcInfo.size())
                        {
                            m_vProcInfo.push_back(ptr);
                        }
                    }
                    break;
                case em_sortby_starttime:
                    {
                        for (it2 = m_vProcInfo.begin(), idex = 0 ; it2 != m_vProcInfo.end() ; it2++, idex++)
                        {
                            if (ptr->m_wstrStartTime < (*it2)->m_wstrStartTime)
                            {
                                m_vProcInfo.insert(m_vProcInfo.begin() + idex, ptr);
                                break;
                            }
                        }

                        if (idex == m_vProcInfo.size())
                        {
                            m_vProcInfo.push_back(ptr);
                        }
                    }
                    break;
                case em_sortby_path:
                    {
                        for (it2 = m_vProcInfo.begin(), idex = 0 ; it2 != m_vProcInfo.end() ; it2++, idex++)
                        {
                            if (ptr->m_wstrProcPath < (*it2)->m_wstrProcPath)
                            {
                                m_vProcInfo.insert(m_vProcInfo.begin() + idex, ptr);
                                break;
                            }
                        }

                        if (idex == m_vProcInfo.size())
                        {
                            m_vProcInfo.push_back(ptr);
                        }
                    }
                    break;
                }
            }
        }
        //减少的,暂不考虑
    }

    PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_vProcInfo.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_vProcInfo.size());
}
*/

INT_PTR CProcSelectView::OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp)
{
    extern HINSTANCE g_hInstance;
    m_hParent = GetParent(hwnd);
    CentreWindow(m_hwnd, m_hParent);
    m_hProcList = GetDlgItem(hwnd, IDC_LIST_PROC);
    m_hEdit = GetDlgItem(hwnd, IDC_EDT_MSG);

    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hwnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    InitListCtrl();

    CTL_PARAMS vArry[] =
    {
        {IDC_LIST_PROC, NULL, 0, 0, 1, 1},
        {IDC_EDT_FILTER, NULL, 0, 1, 1, 0},
        {IDC_BTN_REFUSH, NULL, 1, 1, 0, 0},
        {IDC_BTN_ATTACH, NULL, 1, 1, 0, 0}
    };
    SetCtlsCoord(hwnd, vArry, RTL_NUMBER_OF(vArry));
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
        /*
        ProcInfo *pInfo;
        {
            if (iItm >= (int)m_vProcInfo.size())
            {
                break;
            }

            pInfo = m_vProcInfo[iItm];
        }

        if (0 == iSub)
        {
            ptr->item.iImage = pInfo->m_dwIcoIdex;
        }

        static ustring s_wstrBuf;
        switch (iSub)
        {
        case  0:
            s_wstrBuf = pInfo->m_wstrShowName;
            break;
        case 1:
            s_wstrBuf.format(L"%d", pInfo->m_dwPid);
            break;
        case 2:
            s_wstrBuf = pInfo->m_wstrStartTime;
            break;
        case 3:
            s_wstrBuf = pInfo->m_wstrProcPath;
        }

        ptr->item.pszText = (LPWSTR)s_wstrBuf.c_str();
        */
    } while (FALSE);
}

VOID CProcSelectView::OnListColumnClick(NMLISTVIEW* ptr)
{
    /*
    int id = ptr->iSubItem;
    CScopedLocker lock(this);
    vector<ProcInfo *>::iterator it;
    ProcInfo *ptr1 = NULL;
    ProcInfo *ptr2 = NULL;
    int idex = 0;
    switch (id)
    {
    case 0:
        {
            gs_eSortBy = em_sortby_name;
            for (idex = 0, it = m_vProcInfo.begin() ; it != m_vProcInfo.end() ; it++, idex++)
            {
                for (int i = idex + 1 ; i < (int)m_vProcInfo.size() ; i++)
                {
                    ptr1 = m_vProcInfo[idex];
                    ptr2 = m_vProcInfo[i];
                    if (ptr1->m_wstrShowName > ptr2->m_wstrShowName)
                    {
                        m_vProcInfo[idex] = ptr2;
                        m_vProcInfo[i] = ptr1;
                    }
                }
            }
            PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_vProcInfo.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
            PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_vProcInfo.size());
        }
        break;
    case 1:
        {
            gs_eSortBy = em_sortby_pid;
            for (idex = 0, it = m_vProcInfo.begin() ; it != m_vProcInfo.end() ; it++, idex++)
            {
                for (int i = idex + 1 ; i < (int)m_vProcInfo.size() ; i++)
                {
                    ptr1 = m_vProcInfo[idex];
                    ptr2 = m_vProcInfo[i];
                    if (ptr1->m_dwPid > ptr2->m_dwPid)
                    {
                        m_vProcInfo[idex] = ptr2;
                        m_vProcInfo[i] = ptr1;
                    }
                }
            }
            PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_vProcInfo.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
            PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_vProcInfo.size());
        }
        break;
    case 2:
        {
            gs_eSortBy = em_sortby_starttime;
            for (idex = 0, it = m_vProcInfo.begin() ; it != m_vProcInfo.end() ; it++, idex++)
            {
                for (int i = idex + 1 ; i < (int)m_vProcInfo.size() ; i++)
                {
                    ptr1 = m_vProcInfo[idex];
                    ptr2 = m_vProcInfo[i];
                    if (ptr1->m_wstrStartTime > ptr2->m_wstrStartTime)
                    {
                        m_vProcInfo[idex] = ptr2;
                        m_vProcInfo[i] = ptr1;
                    }
                }
            }
            PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_vProcInfo.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
            PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_vProcInfo.size());
        }
        break;
    case 3:
        {
            gs_eSortBy = em_sortby_path;
            for (idex = 0, it = m_vProcInfo.begin() ; it != m_vProcInfo.end() ; it++, idex++)
            {
                for (int i = idex + 1 ; i < (int)m_vProcInfo.size() ; i++)
                {
                    ptr1 = m_vProcInfo[idex];
                    ptr2 = m_vProcInfo[i];
                    if (ptr1->m_wstrProcPath > ptr2->m_wstrProcPath)
                    {
                        m_vProcInfo[idex] = ptr2;
                        m_vProcInfo[i] = ptr1;
                    }
                }
            }
            PostMessageW(m_hProcList, LVM_SETITEMCOUNT, m_vProcInfo.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
            PostMessageW(m_hProcList, LVM_REDRAWITEMS, 0, m_vProcInfo.size());
        }
        break;
    default:
        break;
    }
    */
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
            SetWindowTextW(m_hEdit, L"请先选择一个进程");
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

    m_PeIco.clear();
    CScopedLocker lock(this);
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