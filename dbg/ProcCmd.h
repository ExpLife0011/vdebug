#ifndef PROCCMD_DBG_H_H_
#define PROCCMD_DBG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include "CmdBase.h"
#include "ProcDbg.h"

class CProcCmd : public CCmdBase {
private:
    CProcCmd();
public:
    static CProcCmd *GetInst();
    virtual ~CProcCmd();
    void InitProcCmd(CProcDbgger *pDbgger);
    virtual CmdReplyResult OnCommand(const mstring &cmd, const mstring &param, DWORD mode, const CmdUserParam *pParam);

private:
    CmdReplyResult OnCmdBp(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBl(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBc(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdBu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdClear(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDisass(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdUb(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdUf(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdGo(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdGu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdKv(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDb(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDd(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdDu(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdReg(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdScript(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdTs(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdTc(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdLm(const mstring &cmd, DWORD mode, const CmdUserParam *pParam);
    CmdReplyResult OnCmdHelp(const mstring &param, DWORD mode, const CmdUserParam *pParam);

    static void __cdecl GuCmdCallback();
private:
    CProcDbgger *mProcDbgger;
};
#endif //PROCCMD_DBG_H_H_