#include <Windows.h>
#include "DescCache.h"
#include "StrUtil.h"

using namespace std;

CDescCache *CDescCache::GetInst() {
    static CDescCache *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CDescCache();
    }
    return s_ptr;
}

void CDescCache::InsertBaseType(int type, const mstring &nameSet, int length, pfnFormatProc pfn) {
    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        StructDesc *newPtr = new StructDesc();
        newPtr->mType = type;
        newPtr->mLength = length;
        newPtr->mPfnFormat = pfn;
        newPtr->mNameSet.insert(*it);
        mStructCache[*it] = newPtr;
    }
}

bool CDescCache::InsertVoidPtr(const mstring &nameSet) {
    StructDesc *newPtr = new StructDesc();
    newPtr->mType = STRUCT_TYPE_PTR;
    newPtr->mUnknownType = true;
    newPtr->mLength = sizeof(void *);
    newPtr->mPfnFormat = PtrFormater;

    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        newPtr->mNameSet.insert(*it);
        mStructCache[*it] = newPtr;
    }
    return true;
}

bool CDescCache::LinkPtr(const mstring &nameSet, const mstring &linked) {
    map<mstring, StructDesc *>::const_iterator it = mStructCache.find(linked);

    if (it == mStructCache.end())
    {
        return false;
    }

    StructDesc *newPtr = new StructDesc();
    newPtr->mType = STRUCT_TYPE_PTR;
    newPtr->mLength = sizeof(void *);
    newPtr->mPfnFormat = PtrFormater;
    StructDesc *ptr = it->second;
    list<mstring> nameArry = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArry.begin() ; it != nameArry.end() ; it++)
    {
        newPtr->mNameSet.insert(*it);
        newPtr->mPtr = ptr;
        mStructCache[*it] = newPtr;
    }
    return true;
}

mstring CDescCache::BoolenFormater(LPVOID ptr, int length) {
    bool t = *(bool *)ptr;
    if (t)
    {
        return "true";
    }
    return "false";
}

mstring CDescCache::ByteFormater(LPVOID ptr, int length) {
    int t = *(byte *)ptr;
    return FormatA("0x%02x(%d)", t, t);
}

mstring CDescCache::CharFormater(LPVOID ptr, int length) {
    char c = *(char *)ptr;
    return FormatA("%c", c);
}

mstring CDescCache::WordFormater(LPVOID ptr, int length) {
    int t = *((WORD *)ptr);
    return FormatA("0x%04x(%d)", t, t);
}

mstring CDescCache::BOOLFormater(LPVOID ptr, int length) {
    BOOL b = *((BOOL *)ptr);
    if (b)
    {
        return "TRUE";
    }
    return "FALSE";
}

mstring CDescCache::WcharFormater(LPVOID ptr, int length) {
    WCHAR w = *((WCHAR *)ptr);
    WCHAR s[2] = {w, 0};
    return FormatA("%ls", s);
}

mstring CDescCache::In32Formater(LPVOID ptr, int length) {
    int d = *((int *)ptr);
    return FormatA("0x%08x(%d)", d, d);
}

mstring CDescCache::Uint32Formater(LPVOID ptr, int length) {
    UINT32 d = *((UINT32 *)ptr);
    return FormatA("0x%08x(%u)", d, d);
}

mstring CDescCache::Int64Foramter(LPVOID ptr, int length) {
    UINT64 d = *((UINT64 *)ptr);
    return FormatA("0x%016x(%I64d)", d, d);
}

mstring CDescCache::PtrFormater(LPVOID ptr, int length) {
    LPVOID p = *((int **)ptr);
    return FormatA("0x%p", p);
}

bool CDescCache::InitDesc() {
    //1 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "bool;boolean", 1, BoolenFormater);
    InsertBaseType(STRUCT_TYPE_BASETYPE, "byte;INT8;BYTE;__int8;unsigned char;UCHAR", 1, ByteFormater);
    InsertBaseType(STRUCT_TYPE_BASETYPE, "char;CHAR", 1, CharFormater);

    //2 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WORD;ATOM;unsigned short;USHORT;UINT16;uint16_t;short;int16_t;INT16;__int16", 2, WordFormater);
    InsertBaseType(STRUCT_TYPE_BASETYPE, "BOOLEN;BOOL", 2, BOOLFormater);
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WCHAR;wchar_t", 2, WcharFormater);

    //4 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "int;INT32;__int32", 4, In32Formater);
    InsertBaseType(STRUCT_TYPE_BASETYPE, "unsigned int;UINT;uint32_t;UINT32;DWORD", 4, Uint32Formater);

    //8 bytes
    InsertBaseType(STRUCT_TYPE_BASETYPE, "__int64;INT64;ULONGLONG;LONGLONG;UINT64;int64_t", 8, Int64Foramter);

    //void ptr
    InsertVoidPtr("void*;LPVOID;PVOID;HANDLE;HKEY;HWINSTA;HWND;HMENU;HINSTANCE;WNDPROC;HICON;HCURSOR;HBRUSH;HOOKPROC;LPTHREAD_START_ROUTINE");

    //ptr
    LinkPtr("LPBYTE;PBYTE;LPCBYTE;PINT8;LPINT8", "byte");
    LinkPtr("LPDWORD;PDWORD;PINT;PINT32;SIZE_T;size_t;ULONG_PTR;LONG_PTR;LSTATUS", "DWORD");
    LinkPtr("PWORD;LPWORD;PINT16;LPINT16", "WORD");
    LinkPtr("LPCSTR;PSTR;LPSTR", "char");
    LinkPtr("LPCWSTR;PWSTR;LPWSTR", "WCHAR");
    return true;
}

bool CDescCache::InsertStruct(const StructDesc *structDesc) {
    return true;
}

bool CDescCache::InsertFun(const FunDesc *funDesc) {
    return true;
}

StructDesc *CDescCache::GetStructByName(const mstring &name) const {
    return NULL;
}

FunDesc *CDescCache::GetFunByName(const mstring &dll, const mstring &fun) const {
    return NULL;
}

CDescCache::CDescCache() {
}

CDescCache::~CDescCache() {
}

DWORD CDescCache::GetStructUnique(StructDesc *desc) const {
    return 0;
}

DWORD CDescCache::GetFunctionUnique(FunDesc *desc) const {
    return 0;
}