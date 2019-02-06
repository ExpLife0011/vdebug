#ifndef DBGSERVICE_VDEBUG_H_H_
#define DBGSERVICE_VDEBUG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
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
    CmdReplyResult RunCmdInCtrlService(const std::mstring &cmd);

    //proc monitor
    bool StartProcMon();
    void StopProcMon();

private:
    void SetCtrlStatus(DbggerStatus stat);
    void OnCmdReply(const std::mstring &content);

    //Proc Debug Event
    static void WINAPI OnProcCreate(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnSystemBreakpoint(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnUserBreakpoint(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnDbgMessage(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnProcExit(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnModuleLoad(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnModuleUnLoad(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnDbgProcRunning(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnDetachDbgger(const EventDbgInfo &eventInfo, void *param);
    static void WINAPI OnProgramException(const EventDbgInfo &eventInfo, void *param);

    //Proc changed
    static void WINAPI OnProcChanged(const EventDbgInfo &eventInfo, void *param);

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