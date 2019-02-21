#ifndef DESCCACHE_H_H_H_
#define DESCCACHE_H_H_H_
#include <Windows.h>
#include <map>
#include "DescParser.h"
#include "mstring.h"
#include "SqliteOperator.h"

using namespace std;

class CDescCache {
public:
    static CDescCache *GetInst();
    bool InitDescCache();
    bool InsertStruct(StructDesc *structDesc);
    bool InsertFun(FunDesc *funDesc);
    StructDesc *GetStructByName(const mstring &name) const;
    FunDesc *GetFunByName(const mstring &dll, const mstring &fun) const;
    StructDesc *GetLinkDescByType(int level, const mstring &linkName) const;
    StructDesc *GetLinkDescByDesc(int level, StructDesc *desc) const;
    StructDesc *CreatePtrStruct() const;
    mstring GetFormatStr(const mstring &fmt, const char *ptr, int length) const;

private:
    CDescCache();
    virtual ~CDescCache();

private:
    mstring StructToString(const StructDesc *desc) const;
    StructDesc *StringToStruct(const mstring &str) const;

    void InsertBaseType(int type, const mstring &nameSet, int length, const mstring &fmt);
    bool InsertVoidPtr(const mstring &nameSet);
    bool LinkPtr(const mstring &nameSet, const mstring &linked);

    bool LoadDescFromDb();
    bool UpdateDescToDb(DWORD checkSum, const mstring &str);

    DWORD GetStructUnique(StructDesc *desc) const;
    DWORD GetFunctionUnique(FunDesc *desc) const;
private:
    mstring mDbPath;
    map<mstring, StructDesc *> mStructCache;
    map<mstring, FunDesc *> mFunCache;
};
#endif //DESCCACHE_H_H_H_