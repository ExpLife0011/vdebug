#ifndef DBGPROXY_VDEBUG_H_H_
#define DBGPROXY_VDEBUG_H_H_
#include "CmdBase.h"
#include "DbgBase.h"

struct CCmdHelpDesc
{
    SyntaxDesc m_vCmdName;
    SyntaxDesc m_vCmdDesc;
    SyntaxDesc m_vCmdExample;
};

class CDbggerProxy : public CCmdBase, public CDbgBase
{
public:
    void OnCmdHlprRegister();
protected:
    void RegisterCmdHlpr(const CCmdHelpDesc &vDesc);
    DbgCmdResult GetAllCmdDesc();

protected:
    static list<CCmdHelpDesc> *ms_pHelpDesc;

public:
    DbgCmdResult OnCmdHelp(const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
};
#endif