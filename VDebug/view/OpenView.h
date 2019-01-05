#ifndef OPENVIEW_VDEBUG_H_H_
#define OPENVIEW_VDEBUG_H_H_
#include <Windows.h>
#include "ViewBase.h"
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

using namespace std;

struct ProcParam {
    std::ustring path;
    std::ustring command;
    std::ustring dir;
};

class PeFileOpenDlg : public CCriticalSectionLockable
{
private:
    PeFileOpenDlg();
public:
    static PeFileOpenDlg *GetInstance();
    virtual ~PeFileOpenDlg();
    bool ShowFileOpenDlg(HWND parent, ProcParam &param);

private:
    int OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnSize(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnTimer(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnNotify(HWND hdlg, WPARAM wp, LPARAM lp);
    static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND m_hdlg;
    ProcParam m_param;

    HWND m_hParent;
    HWND m_hTextPath;
    HWND m_hComPath;
    HWND m_hTextType;
    HWND m_hComType;

    HWND m_hBtnOk;
    HWND m_hBtnCancel;

    HWND m_hTextParam;
    HWND m_hEditParam;
    HWND m_hTextDir;
    HWND m_hEditDir;
    HWND m_hTextHistory;
    HWND m_hComHistory;
};
#endif