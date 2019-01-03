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
    static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND m_hdlg;
    HWND m_hParent;
};
#endif