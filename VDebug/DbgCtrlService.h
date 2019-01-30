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
    DbggerType GetDebuggerStat();
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

    //Debug Event
    static void WINAPI OnProcCreate(const std::mstring &eventName, const std::mstring &content, void *param);
    static void WINAPI OnSystemBreakpoint(const mstring &eventName, const mstring &content, void *param);
    static void WINAPI OnDbgMessage(const std::mstring &eventName, const std::mstring &content, void *param);
    static void WINAPI OnProcExit(const std::mstring &eventName, const std::mstring &content, void *param);
    static void WINAPI OnModuleLoad(const std::mstring &eventName, const std::mstring &content, void *param);
    static void WINAPI OnModuleUnLoad(const std::mstring &eventName, const std::mstring &content, void *param);

    //Proc changed
    static void WINAPI OnProcChanged(const std::mstring &eventName, const std::mstring &content, void *param);

private:
    void RunProcInUser(LPCSTR image, LPCSTR cmd, DWORD session);
private:
    DbggerType m_stat;
    std::mstring m_DbgChannel;
    std::mstring m_unique;
    DbgServiceBase *m_pCtrlService;
    bool m_procMon;
};
#endif //DBGSERVICE_VDEBUG_H_H_