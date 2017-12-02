#ifndef  MAINVIEW_VDEBUG_H_H_
#define MAINVIEW_VDEBUG_H_H_
#include "SyntaxView.h"
#include "../DbgProxy.h"

CSynbaxView *GetSyntaxView();

VOID SetCmdNotify(DebuggerStatus uStatus, const ustring &wstrShowMsg);

CDbggerProxy *GetCurrentDbgger();
#endif