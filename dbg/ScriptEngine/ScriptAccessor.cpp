#include "ScriptAccessor.h"

CScriptAccessor *CScriptAccessor::GetInst() {
    static CScriptAccessor *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CScriptAccessor();
    }
    return s_ptr;
}

CScriptAccessor::CScriptAccessor() {
    mMemoryReader = NULL;
    mDbgger = NULL;
}

CScriptAccessor::~CScriptAccessor() {}

void CScriptAccessor::SetContext(CMemoryBase *reader, CDbgBase *dbgger) {
    mMemoryReader = reader;
    mDbgger = dbgger;
    UpdateContext();
}

void CScriptAccessor::UpdateContext() {
    mContext = mDbgger->GetCurrentContext();
}

bool CScriptAccessor::GetInternalVarData(const mstring &var, char *data) {
    if (var == "@csp")
    {
        memcpy(data, &mContext.csp, sizeof(ULONG_PTR));
    } else if (var == "@cbp")
    {
        memcpy(data, &mContext.cbp, sizeof(ULONG_PTR));
    } else if (var == "@cip")
    {
        memcpy(data, &mContext.cip, sizeof(ULONG_PTR));
    } else if (var == "@cax")
    {
        memcpy(data, &mContext.cax, sizeof(ULONG_PTR));
    } else if (var == "@ccx")
    {
    }
    else if (var == "@param0")
    {
    } else if (var == "@param1")
    {
    } else if (var == "@param2")
    {
    } else if (var == "@param3")
    {
    } else {
        return false;
    }
    return true;
}

bool CScriptAccessor::GetPtrData(DWORD64 ptr, int iSize, char *data) {
    DWORD readSize = 0;
    return mMemoryReader->MemoryReadSafe(ptr, data, iSize, &readSize);
}