#ifndef DBGPROXY_VDEBUG_H_H_
#define DBGPROXY_VDEBUG_H_H_
#include "CmdBase.h"
#include "DbgBase.h"

class CDbggerProxy : public CCmdBase, public CDbgBase
{
    DbgCmdResult OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
};
#endif