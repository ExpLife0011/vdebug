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

struct ProcInfo
{
    ProcInfo() : m_dwPid(0), m_dwIcoIdex(0), m_bIs64(0)
    {}

    ustring m_wstrProcPath;
    ustring m_wstrShowName;
    ustring m_wstrStartTime;
    ustring m_wstrCmd;
    DWORD m_dwPid;
    DWORD m_dwIcoIdex;
    BOOL m_bIs64;
};

class CProcSelectView : public CWindowBase, public CCriticalSectionLockable
{
public:
    CProcSelectView() : m_hParent(NULL), m_hFont(NULL), m_hProcList(NULL), m_hImageList(NULL), m_hNotify(NULL), m_hTread(NULL), m_hExit(NULL), m_dwSelectPid(0)
    {
        m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
        m_hExit = CreateEventW(NULL, FALSE, FALSE, NULL);
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

    bool IsProcInCache(const ustring &wstrUnique);
    void InsertUnique(const ustring &wstrUnique);
    ustring GetProcUnique(const ProcInfo &info);

protected:
    virtual LRESULT OnWindowMsg(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp);
    static BOOL WINAPI ProcHandlerW(PPROCESSENTRY32W pe, void *pParam);
    static DWORD WINAPI RefushThread(LPVOID pParam);

protected:
    HWND m_hParent;
    HWND m_hEdit;
    HWND m_hProcList;
    HFONT m_hFont;
    HANDLE m_hNotify;
    HANDLE m_hExit;
    HANDLE m_hTread;
    DWORD m_dwSelectPid;

    HIMAGELIST m_hImageList;
    vector<ProcInfo *> m_vTempInfo;
    vector<ProcInfo *> m_vProcInfo;
    set<ustring> m_vProcUnique;
    map<ustring, int> m_PeIco;
    set<HICON> m_vIcons;
};
#endif