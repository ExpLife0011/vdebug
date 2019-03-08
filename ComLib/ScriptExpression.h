#pragma once
#include <Windows.h>
#include <set>
#include <map>
#include "mstring.h"
#include "ScriptDef.h"

using namespace std;

class CScriptExpReader {
public:
    static CScriptExpReader *GetInst();
    void SetCache(ScriptCache *cache);
    VariateDesc *ParserExpression(const mstring &expression);

private:
    CScriptExpReader();
    virtual ~CScriptExpReader();

private:
    void InitReader();
    void AddVar(VariateDesc *desc);
    VariateDesc *ParserStr(const mstring &str);
    mstring GetTempVarName();

private:
    set<mstring> mOperatorSet;
    set<mstring> mCmdInternal;
    ScriptCache *mCache;

    DWORD mVarSerial;
    map<mstring, VariateDesc *> mTempVarSet;    //临时变量缓存，解析用
};