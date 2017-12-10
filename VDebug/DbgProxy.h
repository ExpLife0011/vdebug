#ifndef DBGPROXY_VDEBUG_H_H_
#define DBGPROXY_VDEBUG_H_H_
#include "CmdBase.h"
#include "DbgBase.h"

struct CCmdHelpDesc
{
    ustring m_wstrName;
    SyntaxDesc m_vCmdDesc;
    SyntaxDesc m_vCmdExample;
};

class CDbggerProxy : public CCmdBase, public CDbgBase
{
protected:
    static void OnCmdHlprRegister();
    static void RegisterCmdHlpr(const CCmdHelpDesc &vDesc);
    static DbgCmdResult GetAllCmdDesc();

protected:
    static list<CCmdHelpDesc> ms_vHelpDesc;
    static SyntaxDesc ms_vCmdSummary;

public:
    DbgCmdResult OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
    static void InitHelpEngine(); //初始化调试引擎
};
#endif