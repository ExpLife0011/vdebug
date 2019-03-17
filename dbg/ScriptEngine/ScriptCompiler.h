#ifndef SCRIPT_PARSER_H_H_
#define SCRIPT_PARSER_H_H_
#include <Windows.h>
#include "ScriptDef.h"

class CScriptCompiler {
public:
    static CScriptCompiler *GetInst();

    LogicNode *Compile(const string &script);
private:
    CScriptCompiler();
    virtual ~CScriptCompiler();

private:
};
#endif //SCRIPT_PARSER_H_H_
