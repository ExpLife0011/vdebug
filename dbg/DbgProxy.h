#ifndef DBGPROXY_VDEBUG_H_H_
#define DBGPROXY_VDEBUG_H_H_
#include <DbgCtrl/DbgCtrlCom.h>
#include "CmdBase.h"
#include "DbgBase.h"

class CDbggerProxy : public CCmdBase, public CDbgBase
{
protected:
    static void OnCmdHlprRegister();
    static std::mstring GetAllCmdDesc();

protected:

public:
    CmdReplyResult OnCmdHelp(const mstring &param, DWORD mode, const CmdUserParam *pParam);
    static void InitHelpEngine(); //初始化调试引擎
};
#endif