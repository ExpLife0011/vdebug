#ifndef PROCDBGPROXY_DBG_H_H_
#define PROCDBGPROXY_DBG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <DbgCtrl/DbgCtrl.h>
#include "ProcDbg.h"

class ProcDbgProxy {
private:
    ProcDbgProxy();
public:
    static ProcDbgProxy *GetInstance();
    bool InitProcDbgProxy(const wchar_t *unique);
    virtual ~ProcDbgProxy();

private:
    //Dbg Ctrl
    static std::ustring __stdcall ExecProc(const std::ustring &cmd, const std::ustring &content, void *param);
    static std::ustring __stdcall AttachProc(const std::ustring &cmd, const std::ustring &content, void *param);
    static std::ustring _stdcall RunCmd(const std::ustring &cmd, const std::ustring &content, void *param);

private:
    bool m_init;
    bool m_x64;
    CProcDbgger *m_pProcDbgger;
    DbgClientBase *m_pDbgClient;

};
#endif //PROCDBGPROXY_DBG_H_H_