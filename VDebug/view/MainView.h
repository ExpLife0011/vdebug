#ifndef MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include <ComLib/ComLib.h>
#include <SyntaxHlpr/SyntaxView.h>
#include <ComLib/GlobalDef.h>
#include "DbgCtrlService.h"
#include "ProcView.h"

void AppendToSyntaxView(const std::mstring &label, const std::mstring &data);
CProcSelectView *GetProcView();
SyntaxView *GetSyntaxView();
VOID SetCmdNotify(DbggerStatus status, const mstring &strShowMsg);
VOID SetInputStat();
VOID SetFunViewStat(const std::mstring &text);
VOID NotifyFunCover();
#endif