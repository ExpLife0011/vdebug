#ifndef DESCCACHE_H_H_H_
#define DESCCACHE_H_H_H_
#include <Windows.h>
#include <map>
#include "DescParser.h"
#include "mstring.h"

using namespace std;

class CDescCache {
public:
    static CDescCache *GetInst();
    bool InitDesc();
    bool InsertStruct(const StructDesc *structDesc);
    bool InsertFun(const FunDesc *funDesc);
    StructDesc *GetStructByName(const mstring &name) const;
    FunDesc *GetFunByName(const mstring &dll, const mstring &fun) const;

private:
    CDescCache();
    virtual ~CDescCache();

private:
    void InsertBaseType(int type, const mstring &nameSet, int length, pfnFormatProc pfn);
    bool InsertVoidPtr(const mstring &nameSet);
    bool LinkPtr(const mstring &nameSet, const mstring &linked);

    DWORD GetStructUnique(StructDesc *desc) const;
    DWORD GetFunctionUnique(FunDesc *desc) const;

    static mstring __stdcall BoolenFormater(LPVOID ptr, int length);
    static mstring __stdcall ByteFormater(LPVOID ptr, int length);
    static mstring __stdcall CharFormater(LPVOID ptr, int length);
    static mstring __stdcall WordFormater(LPVOID ptr, int length);
    static mstring __stdcall BOOLFormater(LPVOID ptr, int length);
    static mstring __stdcall WcharFormater(LPVOID ptr, int length);
    static mstring __stdcall In32Formater(LPVOID ptr, int length);
    static mstring __stdcall Uint32Formater(LPVOID ptr, int length);
    static mstring __stdcall Int64Foramter(LPVOID ptr, int length);
    static mstring __stdcall PtrFormater(LPVOID ptr, int length);
private:
    map<mstring, StructDesc *> mStructCache;
    map<mstring, FunDesc *>mFunCache;
};
#endif //DESCCACHE_H_H_H_