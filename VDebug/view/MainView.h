#ifndef MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include <ComStatic/ComStatic.h>
#include <SyntaxHlpr/SyntaxView.h>
#include <ComStatic/GlobalDef.h>
#include "DbgCtrlService.h"
#include "ProcView.h"

void AppendToSyntaxView(const std::mstring &label, const std::mstring &data);
CProcSelectView *GetProcView();
SyntaxView *GetSyntaxView();
VOID SetCmdNotify(DbggerStatus status, const ustring &wstrShowMsg);
#endif