#ifndef  MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include "../DbgProxy.h"
#include <SyntaxHlpr/SyntaxView.h>

SyntaxView *GetSyntaxView();

VOID SetCmdNotify(DebuggerStatus uStatus, const ustring &wstrShowMsg);

CDbggerProxy *GetCurrentDbgger();
#endif