#ifndef  MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include <SyntaxHlpr/SyntaxView.h>
#include <ComStatic/GlobalDef.h>
#include "DbgCtrlService.h"

SyntaxView *GetSyntaxView();
VOID SetCmdNotify(DebuggerStatus status, const ustring &wstrShowMsg);
#endif