#ifndef PROCDBGPROXY_DBG_H_H_
#define PROCDBGPROXY_DBG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <DbgCtrl/DbgCtrl.h>
#include "ProcDbg.h"
#include "procmon.h"

class ProcDbgProxy : public ProcListener {
private:
    ProcDbgProxy();
public:
    static ProcDbgProxy *GetInstance();
    bool InitProcDbgProxy(const wchar_t *unique);
    virtual ~ProcDbgProxy();

private:
    //Dbg Ctrl
    static std::ustring __stdcall GetProcInfo(const std::ustring &cmd, const std::ustring &content, void *param);
    static std::ustring __stdcall ExecProc(const std::ustring &cmd, const std::ustring &content, void *param);
    static std::ustring __stdcall AttachProc(const std::ustring &cmd, const std::ustring &content, void *param);
    static std::ustring _stdcall RunCmd(const std::ustring &cmd, const std::ustring &content, void *param);

private:
    //proc event
    virtual void OnProcChanged(HProcListener listener, const list<const ProcMonInfo *> &added, const list<DWORD> &killed);

private:
    bool m_init;
    bool m_x64;
    CProcDbgger *m_pProcDbgger;
    DbgClientBase *m_pDbgClient;
    HProcListener m_hProcListener;

};
#endif //PROCDBGPROXY_DBG_H_H_