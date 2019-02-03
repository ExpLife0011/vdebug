#ifndef PROCDBGPROXY_DBG_H_H_
#define PROCDBGPROXY_DBG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <DbgCtrl/DbgCtrl.h>
#include "ProcDbg.h"
#include "ProcCmd.h"
#include "procmon.h"

class ProcDbgProxy : public ProcListener {
private:
    ProcDbgProxy();
public:
    static ProcDbgProxy *GetInstance();
    bool InitProcDbgProxy(const char *unique);
    virtual ~ProcDbgProxy();

private:
    //Dbg Ctrl
    static std::mstring __stdcall GetProcInfo(const std::mstring &cmd, const std::mstring &content, void *param);
    static std::mstring __stdcall ExecProc(const std::mstring &cmd, const std::mstring &content, void *param);
    static std::mstring __stdcall AttachProc(const std::mstring &cmd, const std::mstring &content, void *param);
    static std::mstring __stdcall RunCmd(const std::mstring &cmd, const std::mstring &content, void *param);
    static std::mstring __stdcall DetachProc(const std::mstring &cmd, const std::mstring &content, void *param);
    static std::mstring __stdcall BreakDebugger(const std::mstring &cmd, const std::mstring &content, void *param);

private:
    //proc event
    virtual void OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed);

private:
    bool m_init;
    bool m_x64;
    CProcDbgger *m_pProcDbgger;
    CProcCmd *m_pCmdRunner;
    DbgClientBase *m_pDbgClient;
    HProcListener m_hProcListener;

};
#endif //PROCDBGPROXY_DBG_H_H_