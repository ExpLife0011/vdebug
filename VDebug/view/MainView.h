#ifndef MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include <ComLib/ComLib.h>
#include <ComLib/GlobalDef.h>
#include "DbgCtrlService.h"
#include "ProcView.h"
#include "CmdShowView.h"

//��ȡ����ʽ����
const CStyleConfig *GetMainStyleCfg();
void AppendToSyntaxView(const std::mstring &label, const std::mstring &data);
CProcSelectView *GetProcView();
CCmdShowView *GetSyntaxView();
VOID SetCmdNotify(DbggerStatus status, const mstring &strShowMsg);
VOID SetInputStat();
VOID SetFunViewStat(const std::mstring &text);
VOID NotifyFunCover();
#endif