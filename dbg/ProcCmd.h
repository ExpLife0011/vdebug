#ifndef PROCCMD_DBG_H_H_
#define PROCCMD_DBG_H_H_
#include <Windows.h>
#include <ComLib/ComLib.h>
#include "CmdBase.h"
#include "ProcDbg.h"

class CProcCmd : public CCmdBase {
private:
    CProcCmd();
public:
    static CProcCmd *GetInst();
    virtual ~CProcCmd();
    void InitProcCmd(CProcDbgger *pDbgger);

private:
    static CtrlReply OnCmdBp(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdBl(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdBc(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdBu(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdBe(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnBreak(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdDisass(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdUb(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdUf(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdGo(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdGu(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdKv(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdDb(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdDd(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdDu(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdDa(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdReg(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdScript(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdTs(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdTc(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdLm(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdPf(const mstring &cmd, HUserCtx ctx);
    static CtrlReply OnCmdHelp(const mstring &param, HUserCtx ctx);

    static void __cdecl GuCmdCallback();
private:
    static CProcDbgger *mProcDbgger;
};
#endif //PROCCMD_DBG_H_H_