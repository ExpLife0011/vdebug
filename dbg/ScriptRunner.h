#pragma once
#include <Windows.h>
#include <ComLib/mstring.h>
#include "ScriptDef.h"

using namespace std;

//调试脚本引擎对外接口
class CScriptRunner {
public:
    static CScriptRunner *GetInst();
    bool ExecScriptStr(const mstring &str);
    bool ExecScriptFile(const mstring &filePath);

private:
    CScriptRunner();
    virtual ~CScriptRunner();
    bool RunLogic(LogicNode *root);

private:
};