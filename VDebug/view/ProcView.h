#ifndef PROCVIEW_VDEBUG_H_H_
#define PROCVIEW_VDEBUG_H_H_
#include <Windows.h>
#include <CommCtrl.h>
#include <set>
#include <vector>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include "ViewBase.h"

using namespace std;

struct ProcShowInfo
{
    ProcMonInfo info;
    DWORD m_dwIcoIdex;

    ProcShowInfo() {
        m_dwIcoIdex = -1;
    }
};

class CProcSelectView : public CWindowBase, public CCriticalSectionLockable
{
public:
    CProcSelectView() : m_hParent(NULL), m_hFont(NULL), m_hProcList(NULL), m_hImageList(NULL), m_dwSelectPid(0)
    {
    }

    virtual ~CProcSelectView()
    {}

    BOOL SetFont(HFONT hFont);

    BOOL Refush();

    BOOL Create();

    BOOL DoModule(HWND hParent);

    DWORD GetSelectProc()
    {
        return m_dwSelectPid;
    }

    void OnProcChanged(const list<ProcMonInfo *> &added, const list<DWORD> &killed);
    void DeleteProcCache();
protected:
    INT_PTR OnInitDlg(HWND hwnd, WPARAM wp, LPARAM lp);

    VOID OnGetListCtrlDisplsy(IN OUT NMLVDISPINFOW* ptr);
    void OnListColumnClick(IN NMLISTVIEW *ptr);

    INT_PTR OnNotify(HWND hwnd, WPARAM wp, LPARAM lp);
    INT_PTR OnClose(HWND hwnd, WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(HWND hwnd, WPARAM wp, LPARAM lp);

    
protected:
    void InitListCtrl();
    VOID CalWidthByColumns() const;
    void RefushProc();
    int GetFileIco(const ustring &wstrFile);
    ustring GetLastSelectDir();
    void RecordLastSelect(LPCWSTR wszDir);

protected:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);

protected:
    HWND m_hParent;
    HWND m_hEdit;
    HWND m_hProcList;
    HFONT m_hFont;
    DWORD m_dwSelectPid;

    HIMAGELIST m_hImageList;
    map<ustring, int> m_PeIco;
    vector<ProcMonInfo *> m_procShow;
    vector<ProcMonInfo *> m_procAll;

    static map<ustring, HICON> ms_peIcon;
};
#endif