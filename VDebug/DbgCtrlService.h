#ifndef DBGSERVICE_VDEBUG_H_H_
#define DBGSERVICE_VDEBUG_H_H_
#include <Windows.h>
#include <ComLib/ComLib.h>
#include <DbgCtrl/DbgCtrl.h>

class DbgCtrlService {
private:
    DbgCtrlService();

public:
    static DbgCtrlService *GetInstance();
    virtual ~DbgCtrlService();

    bool InitCtrlService();
    bool SetDebugger(DbggerType type);
    DbggerStatus GetDebuggerStat();
    bool BreakDbgProcInCtrlService() const;
    bool ExecProc(const std::mstring &path, const std::mstring &param);
    bool AttachProc(DWORD pid);
    bool DetachProc();

    bool TestDescStr(const std::mstring &dll, const std::mstring &str);
    bool InputDescStr(const std::mstring &dll, const std::mstring &str, bool cover = false);

    bool OpenDump(const std::mstring &path) const;
    CtrlReply RunCmdInCtrlService(const std::mstring &cmd);

    //proc monitor
    bool StartProcMon();
    void StopProcMon();

private:
    void SetCtrlStatus(DbggerStatus stat);
    void OnCmdReply(const std::mstring &content);

    //Proc Debug Event
    static void WINAPI OnProcCreate(const EventInfo &eventInfo, void *param);
    static void WINAPI OnSystemBreakpoint(const EventInfo &eventInfo, void *param);
    static void WINAPI OnUserBreakpoint(const EventInfo &eventInfo, void *param);
    static void WINAPI OnDbgMessage(const EventInfo &eventInfo, void *param);
    static void WINAPI OnProcExit(const EventInfo &eventInfo, void *param);
    static void WINAPI OnModuleLoad(const EventInfo &eventInfo, void *param);
    static void WINAPI OnModuleUnLoad(const EventInfo &eventInfo, void *param);
    static void WINAPI OnDetachDbgger(const EventInfo &eventInfo, void *param);
    static void WINAPI OnProgramException(const EventInfo &eventInfo, void *param);

    //Proc changed
    static void WINAPI OnProcChanged(const EventInfo &eventInfo, void *param);

    //Dbg Status
    static void __stdcall DbgStatusNotifyProc(const DbgStat &status, void *param);
private:
    void RunProcInUser(LPCSTR image, LPCSTR cmd, DWORD session);
private:
    DbggerType m_type;
    DbggerStatus m_stat;
    std::mstring m_DbgChannel;
    std::mstring m_unique;
    DbgServiceBase *m_pCtrlService;
    bool m_procMon;
};
#endif //DBGSERVICE_VDEBUG_H_H_