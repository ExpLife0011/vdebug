#pragma once
#include <Windows.h>
#include <ComLib/ComLib.h>
#include "ScriptDef.h"
#include "../DbgBase.h"
#include "../MemoryBase.h"

using namespace std;

class CScriptAccessor {
public:
    static CScriptAccessor *GetInst();
    void SetContext(CMemoryBase *reader, CDbgBase *dbgger);
    void UpdateContext();
    bool GetInternalVarData(const mstring &var, char *data);
    bool GetPtrData(DWORD64 ptr, int iSize, char *data);

private:
    CScriptAccessor();
    virtual ~CScriptAccessor();

private:
    CMemoryBase *mMemoryReader;
    CDbgBase *mDbgger;
    TITAN_ENGINE_CONTEXT_t mContext;
};