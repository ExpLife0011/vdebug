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
    virtual CtrlReply OnCommand(const mstring &cmd, const mstring &param, const CmdUserParam *pParam);

private:
    CtrlReply OnCmdBp(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdBl(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdBc(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdBu(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdBe(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDisass(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdUb(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdUf(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdGo(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdGu(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdKv(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDb(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDd(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDu(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDa(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdReg(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdScript(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdTs(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdTc(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdLm(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdPf(const mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdHelp(const mstring &param, const CmdUserParam *pParam);

    static void __cdecl GuCmdCallback();
private:
    CProcDbgger *mProcDbgger;
};
#endif //PROCCMD_DBG_H_H_