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
    bool SetDebuggerStat(DbggerType type);
    DbggerType GetDebuggerStat();
    bool ExecProc(const std::ustring &path, const std::ustring &param);
    bool AttachProc(DWORD pid);
    bool DetachProc();
    bool RunCmdInCtrlService(const std::ustring &command);

private:
    void OnCmdReply(const std::ustring &content);
    //Debug Event
    static void WINAPI OnProcCreate(const std::ustring &event, const std::ustring &content, void *param);
    static void WINAPI OnDbgMessage(const std::ustring &event, const std::ustring &content, void *param);
    static void WINAPI OnProcExit(const std::ustring &event, const std::ustring &content, void *param);
    static void WINAPI OnModuleLoad(const std::ustring &event, const std::ustring &content, void *param);
    static void WINAPI OnModuleUnLoad(const std::ustring &event, const std::ustring &content, void *param);
    static void WINAPI OnProcChanged(const std::ustring &event, const std::ustring &content, void *param);

private:
    void RunProcInUser(LPCWSTR image, LPCWSTR cmd, DWORD session);
private:
    DbggerType m_stat;
    std::ustring m_DbgChannel;
    std::ustring m_unique;
    DbgServiceBase *m_pCtrlService;
};
#endif //DBGSERVICE_VDEBUG_H_H_