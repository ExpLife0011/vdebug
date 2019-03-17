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
    virtual CtrlReply OnCommand(const std::mstring &cmd, const std::mstring &param, const CmdUserParam *pParam);

private:
    CtrlReply OnCmdDisass(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdUb(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdUf(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdKv(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDb(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDd(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDu(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdDa(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdReg(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdTs(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdTc(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdLm(const std::mstring &cmd, const CmdUserParam *pParam);
    CtrlReply OnCmdHelp(const std::mstring &param, const CmdUserParam *pParam);
};
#endif //DUMP_CMD_DBG_H_H_