#ifndef PROCMON_DBG_H_H_
#define PROCMON_DBG_H_H_
#include <Windows.h>
#include <TlHelp32.h>
#include <map>
#include <set>
#include <list>

#include <ComLib/ComLib.h>
#include <ComLib/ComLib.h>

typedef DWORD HProcListener;

class ProcListener {
public:
    virtual void OnProcChanged(HProcListener listener, const std::list<const ProcMonInfo *> &added, const std::list<DWORD> &killed) = 0;
};

struct ProcRegisterInfo {
    HProcListener m_index;
    ProcListener *m_listener;
    std::set<DWORD> m_ProcCache;
};

class ProcMonitor : public CCriticalSectionLockable {
private:
    ProcMonitor();

public:
    static ProcMonitor *GetInstance();
    virtual ~ProcMonitor();

    HProcListener RegisterListener(ProcListener *listener);
    void UnRegisterListener(HProcListener);

private:
    void RefushProc();
    static bool GetProcSidAndUser(HANDLE process, std::mstring &sid, std::mstring &user);
    static BOOL WINAPI ProcHandlerW(PPROCESSENTRY32W pe, void *pParam);
    static DWORD WINAPI MonitorThread(LPVOID pParam);
    void DispatchProcChanged();
    static DWORD GetProcUnique(DWORD pid);

private:
    HANDLE m_hMonitorThread;
    HANDLE m_hExitNotify;
    std::map<HProcListener, ProcRegisterInfo> m_register;
    std::map<DWORD, ProcMonInfo *> m_ProcInfo;
    static int ms_curIndex;
};
#endif //PROCMON_DBG_H_H_