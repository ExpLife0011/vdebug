#ifndef DUMP_CMD_DBG_H_H_
#define DUMP_CMD_DBG_H_H_
#include <ComLib/ComLib.h>
#include <DbgCtrl/DbgCtrl.h>
#include "CmdBase.h"

class CDumpCmd : public CCmdBase {
private:
    CDumpCmd();
public:
    static CDumpCmd *GetInst();
    virtual ~CDumpCmd();
    void InitProcCmd();
    virtual CtrlReply OnCommand(const std::mstring &cmd, const std::mstring &param, HUserCtx ctx);

private:
    CtrlReply OnCmdDisass(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdUb(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdUf(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdKv(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdDb(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdDd(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdDu(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdDa(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdReg(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdTs(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdTc(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdLm(const std::mstring &cmd, HUserCtx ctx);
    CtrlReply OnCmdHelp(const std::mstring &param, HUserCtx ctx);
};
#endif //DUMP_CMD_DBG_H_H_