#pragma once
#include <Windows.h>
#include <ComLib/mstring.h>
#include "ScriptDef.h"

using namespace std;

//���Խű��������ӿ�
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