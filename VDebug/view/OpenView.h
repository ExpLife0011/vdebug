#ifndef OPENVIEW_VDEBUG_H_H_
#define OPENVIEW_VDEBUG_H_H_
#include <Windows.h>
#include "ViewBase.h"
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

using namespace std;

struct ProcParam {
    BOOL succ;
    std::ustring path;
    std::ustring command;
    std::ustring dir;

    ProcParam() {
        succ = FALSE;
    }
};

class PeFileOpenDlg : public CCriticalSectionLockable
{
    struct HistoryInfo {
        unsigned long mId;
        std::ustring mPath;
        std::ustring mParam;
        std::ustring mDir;
        std::ustring mTime;
    };

private:
    PeFileOpenDlg();
public:
    static PeFileOpenDlg *GetInstance();
    virtual ~PeFileOpenDlg();
    BOOL ShowFileOpenDlg(HWND parent, ProcParam &param);

private:
    void OnHistorySelect(int index) const;
    bool SaveHistory(HistoryInfo &history) const;
    std::vector<HistoryInfo> GetHistory(int maxSize) const;
    int OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnSize(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnTimer(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnNotify(HWND hdlg, WPARAM wp, LPARAM lp);
    int OnCommand(HWND hdlg, WPARAM wp, LPARAM lp);
    static UINT_PTR CALLBACK OFNHookProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
    static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);

private:
    HWND m_hdlg;
    ProcParam m_param;

    HWND m_hParent;
    HWND m_hTextPath;
    HWND m_hComExPath;
    HWND m_hEditPath;

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
    HWND m_hEditStatus;

    vector<HistoryInfo> mHistory;
};
#endif