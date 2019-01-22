#ifndef DBGPROXY_VDEBUG_H_H_
#define DBGPROXY_VDEBUG_H_H_
#include "CmdBase.h"
#include "DbgBase.h"

class CDbggerProxy : public CCmdBase, public CDbgBase
{
protected:
    static void OnCmdHlprRegister();
    static DbgCmdResult GetAllCmdDesc();

protected:

public:
    DbgCmdResult OnCmdHelp(const mstring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    static void InitHelpEngine(); //初始化调试引擎
};
#endif