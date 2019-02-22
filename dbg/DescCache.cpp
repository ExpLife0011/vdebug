#include <Windows.h>
#include <Shlwapi.h>
#include <gdlib/gdcrc32.h>
#include <gdlib/json/json.h>
#include "DescCache.h"
#include "StrUtil.h"

using namespace std;
using namespace Json;

CDescCache *CDescCache::GetInst() {
    static CDescCache *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CDescCache();
    }
    return s_ptr;
}

void CDescCache::InsertBaseType(int type, const mstring &nameSet, int length, const mstring &fmt) {
    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        StructDesc *newPtr = new StructDesc();
        newPtr->mType = type;
        newPtr->mLength = length;
        newPtr->mFormat = fmt;
        newPtr->mTypeName = (*it);
        mStructCache[*it] = newPtr;
    }
}

bool CDescCache::InsertVoidPtr(const mstring &nameSet) {
    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        StructDesc *newPtr = new StructDesc();
        newPtr->mType = STRUCT_TYPE_PTR;
        newPtr->mUnknownType = true;
        newPtr->mLength = sizeof(void *);
        newPtr->mFormat = "0x%08x";

        newPtr->mTypeName = (*it);
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
    StructDesc *ptrLinked = it->second;

    list<mstring> nameArry = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator ij = nameArry.begin() ; ij != nameArry.end() ; ij++)
    {
        StructDesc *newPtr = new StructDesc();
        newPtr->mType = STRUCT_TYPE_PTR;
        newPtr->mLength = sizeof(void *);
        newPtr->mFormat = "0x%08x";

        newPtr->mTypeName = (*ij);
        newPtr->mPtr = ptrLinked;
        newPtr->mLinkCount = 1;
        newPtr->mPtrEnd = ptrLinked;
        newPtr->mEndType = ptrLinked->mTypeName;
        mStructCache[*ij] = newPtr;
    }
    return true;
}

bool CDescCache::LoadStructFromDb() {
    SqliteOperator opt(mDbPath);
    SqliteResult result = opt.Select("select content from tStructDesc");

    map<mstring, StructDesc *> cache;
    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        StructDesc *desc = StringToStruct(it.GetValue("content"));
        cache[desc->mTypeName] = desc;
    }

    for (map<mstring, StructDesc *>::const_iterator it = cache.begin() ; it != cache.end() ; it++)
    {
        StructDesc *ptr = it->second;

        if (ptr->mType == STRUCT_TYPE_PTR)
        {
            map<mstring, StructDesc *>::const_iterator ij = cache.find(ptr->mEndType);
            StructDesc *lastPtr = ptr;
            for (int i = 0 ; i < ptr->mLinkCount - 1 ; i++)
            {
                StructDesc *tmp = CreatePtrStruct();
                lastPtr->mPtr = tmp;
                lastPtr = tmp;
            }
            lastPtr->mPtr = ij->second;
            ptr->mPtrEnd = ij->second;
        } else if (ptr->mType == STRUCT_TYPE_STRUCT)
        {
            for (vector<mstring>::iterator it2 = ptr->mMemberType.begin() ; it2 != ptr->mMemberType.end() ; it2++)
            {
                map<mstring, StructDesc *>::const_iterator ij2 = mStructCache.find(*it2);
                if (ij2 == mStructCache.end())
                {
                    ij2 = cache.find(*it2);
                }

                ptr->mMemberSet.push_back(ij2->second);
            }
        }
    }
    mStructCache.insert(cache.begin(), cache.end());
    return true;
}

bool CDescCache::LoadFunctionFromDb() {
    SqliteOperator opt(mDbPath);
    SqliteResult result = opt.Select("select content from tStructDesc");

    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        FunDesc *desc = StringToFunction(it.GetValue("content"));
        mstring d = FormatA("%hs!%hs", desc->mDllName.c_str(), desc->mProcName.c_str());
        mFunSetByFunction[desc->mProcName].push_back(desc);
    }
    return true;
}

bool CDescCache::UpdateStructToDb(DWORD checkSum, const mstring &str) {
    SqliteOperator opt(mDbPath);
    mstring sql = FormatA(
        "replace into tStructDesc(id, content, time)values(%u, '%hs', '%hs')",
        checkSum,
        str.c_str(),
        "2018-11-11 11:22:33 456"
        );
    opt.Exec(sql);
    return true;
}

bool CDescCache::UpdateFunctionToDb(DWORD checkSum, const mstring &str) {
    SqliteOperator opt(mDbPath);
    mstring sql = FormatA(
        "replace into tFunctionDesc(id, content, time)values(%u, '%hs', '%hs')",
        checkSum,
        str.c_str(),
        "2018-11-11 11:22:33 456"
        );
    opt.Exec(sql);
    return true;
}

bool CDescCache::InitDescCache() {
    char dbPath[256] = {0};
    GetModuleFileNameA(NULL, dbPath, 256);
    PathAppendA(dbPath, "..\\db\\DescDb.db");
    mDbPath = dbPath;

    SqliteOperator opt(mDbPath);
    opt.Exec("create table if not exists tStructDesc (id BIGINT PRIMARY KEY, content TEXT, time CHAR(32))");
    opt.Exec("create table if not exists tFunctionDesc (id BIGINT PRIMARY KEY, content TEXT, time CHAR(32))");

    //1 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "bool;boolean", 1, "0x%x(%d)");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "byte;INT8;BYTE;__int8;unsigned char;UCHAR", 1, "0x%x(%u)");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "char;CHAR", 1, "'%c'(%d)");

    //2 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WORD;ATOM;unsigned short;USHORT;UINT16;uint16_t;short;int16_t;INT16;__int16", 2, "0x%x(%d)");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "BOOLEN;BOOL", 2, "0x%x(%d)");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WCHAR;wchar_t", 2, "'%c'(%d)");

    //4 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "int;INT32;__int32", 4, "0x%x(%d)");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "unsigned int;UINT;uint32_t;UINT32;DWORD", 4, "0x%x(%u)");

    //8 bytes
    InsertBaseType(STRUCT_TYPE_BASETYPE, "__int64;INT64;ULONGLONG;LONGLONG;UINT64;int64_t", 8, "0x%x(%I64d)");

    //void ptr
    InsertVoidPtr("void*;LPVOID;PVOID;HANDLE;HKEY;HWINSTA;HWND;HMENU;HINSTANCE;WNDPROC;HICON;HCURSOR;HBRUSH;HOOKPROC;LPTHREAD_START_ROUTINE");

    //ptr
    LinkPtr("LPBYTE;PBYTE;LPCBYTE;PINT8;LPINT8", "byte");
    LinkPtr("LPDWORD;PDWORD;PINT;PINT32;SIZE_T;size_t;ULONG_PTR;LONG_PTR;LSTATUS", "DWORD");
    LinkPtr("PWORD;LPWORD;PINT16;LPINT16", "WORD");
    LinkPtr("LPCSTR;PSTR;LPSTR", "char");
    LinkPtr("LPCWSTR;PWSTR;LPWSTR", "WCHAR");

    LoadStructFromDb();
    LoadFunctionFromDb();
    return true;
}

bool CDescCache::InsertStruct(StructDesc *desc) {
    mstring str = StructToString(desc);
    desc->mCheckSum = std_crc32(str.c_str(), str.size());
    mStructCache[desc->mTypeName] = desc;

    UpdateStructToDb(desc->mCheckSum, str);
    return true;
}

bool CDescCache::InsertFun(FunDesc *funDesc) {
    mstring d = FormatA("%hs!%hs", funDesc->mDllName.c_str(), funDesc->mProcName.c_str());
    mstring str = FunctionToString(funDesc);
    funDesc->mCheckSum = std_crc32(str.c_str(), str.size());
    mFunSetByFunction[funDesc->mProcName].push_back(funDesc);

    UpdateFunctionToDb(funDesc->mCheckSum, str);
    return true;
}

StructDesc *CDescCache::GetStructByName(const mstring &name) const {
    map<mstring, StructDesc *>::const_iterator it = mStructCache.find(name);

    if (it == mStructCache.end())
    {
        return NULL;
    }
    return it->second;
}

list<FunDesc *> CDescCache::GetFunByName(const mstring &dll, const mstring &fun) const {
    map<mstring, list<FunDesc *>>::const_iterator it = mFunSetByFunction.find(fun);
    if (mFunSetByFunction.end() == it)
    {
        return list<FunDesc *>();
    }

    if (dll.empty() && dll == "*")
    {
        return it->second;
    }

    list<FunDesc *> result;
    for (list<FunDesc *>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
    {
        if ((*ij)->mDllName == dll)
        {
            result.push_back(*ij);
        }
    }
    return result;
}

StructDesc *CDescCache::CreatePtrStruct() const {
    StructDesc *ptr = new StructDesc();
    ptr->mType = STRUCT_TYPE_PTR;
    ptr->mLength = sizeof(void *);
    ptr->mFormat = "0x%08x";
    return ptr;
}

mstring CDescCache::GetFormatStr(const mstring &fmt, const char *ptr, int length) const {
    int paramCount = 0;

    vector<mstring> fmtSet;
    size_t lastPos = 0;
    size_t curPos = 0;
    curPos = fmt.find('%');
    if (curPos != mstring::npos)
    {
        curPos = fmt.find('%', curPos + 1);
    }

    if (curPos != mstring::npos)
    {
        do
        {
            fmtSet.push_back(fmt.substr(lastPos, curPos - lastPos));
            lastPos = curPos;
            curPos = curPos + 1;

            curPos = fmt.find('%', curPos);
        } while (curPos != mstring::npos);

        if (fmt.size() > lastPos)
        {
            fmtSet.push_back(fmt.substr(lastPos, fmt.size() - lastPos));
        }
    } else {
        fmtSet.push_back(fmt);
    }

    mstring result;
    size_t i = 0;
    if (length == 1)
    {
        byte d = *(byte *)ptr;
        for (i = 0 ; i < fmtSet.size() ; i++)
        {
            result += FormatA(fmtSet[i].c_str(), d);
        }
    } else if (length == 2)
    {
        unsigned short d = *(unsigned short *)ptr;
        for (i = 0 ; i < fmtSet.size() ; i++)
        {
            result += FormatA(fmtSet[i].c_str(), d);
        }
    } else if (length == 4)
    {
        unsigned int d = *(unsigned int *)ptr;
        for (i = 0 ; i < fmtSet.size() ; i++)
        {
            result += FormatA(fmtSet[i].c_str(), d);
        }
    } else if (length == 8)
    {
        ULONGLONG d = *(ULONGLONG *)ptr;
        for (i = 0 ; i < fmtSet.size() ; i++)
        {
            result += FormatA(fmtSet[i].c_str(), d);
        }
    }
    return result;
}

StructDesc *CDescCache::GetLinkDescByType(int level, const mstring &linkName) const {
    StructDesc *pStructCache = GetStructByName(linkName);
    if (pStructCache == NULL)
    {
        throw(new CParserException(FormatA("未识别的参数类型 %hs", linkName.c_str())));
    }

    return GetLinkDescByDesc(level, pStructCache);
}

StructDesc *CDescCache::GetLinkDescByDesc(int level, StructDesc *desc) const {
    StructDesc *root = CreatePtrStruct();
    StructDesc *lastPtr = root;
    for (int i = 0 ; i < level - 1 ; i++) {
        StructDesc *tmp = CreatePtrStruct();
        lastPtr->mPtr = tmp;
        lastPtr = tmp;
    }

    lastPtr->mPtr = desc;
    root->mLinkCount = level;
    root->mEndType = desc->mTypeName;
    root->mPtrEnd = desc;
    return root;
}

CDescCache::CDescCache() {
}

CDescCache::~CDescCache() {
}

mstring CDescCache::StructToString(const StructDesc *desc) const {
    Value content;
    content["type"] = (int)desc->mType;
    content["typeName"] = desc->mTypeName;

    if (STRUCT_TYPE_BASETYPE == desc->mType)
    {
        content["fmtStr"] = desc->mFormat;
        content["length"] = desc->mLength;
    } else if (STRUCT_TYPE_PTR == desc->mType)
    {
        content["unknownType"] = desc->mUnknownType;
        content["linkCount"] = desc->mLinkCount;
        content["arrySize"] = desc->mSize;
        content["endType"] = desc->mEndType;
    } else if (STRUCT_TYPE_STRUCT == desc->mType)
    {
        Value members;
        for (size_t i = 0 ; i < desc->mMemberSet.size() ; i++)
        {
            Value single;
            single["memberType"] = desc->mMemberSet[i]->mTypeName;
            single["memberName"] = desc->mMemberName[i];
            single["memberOffset"] = desc->mMemberOffset[i];
            members.append(single);
        }
        content["memberSet"] = members;
    }
    return FastWriter().write(content);
}

StructDesc *CDescCache::StringToStruct(const mstring &str) const {
    Value content;

    Reader().parse(str, content);
    if (content.type() != objectValue)
    {
        return NULL;
    }

    StructDesc *desc = new StructDesc();
    desc->mType = content["type"].asInt();
    desc->mTypeName = content["typeName"].asString();

    if (desc->mType == STRUCT_TYPE_BASETYPE)
    {
        desc->mFormat = content["fmtStr"].asString();
        desc->mLength = content["length"].asInt();
    } else if (desc->mType == STRUCT_TYPE_PTR)
    {
        desc->mUnknownType = content["unknownType"].asBool();
        desc->mLinkCount = content["linkCount"].asInt();
        desc->mSize = content["arrySize"].asInt();
        desc->mEndType = content["endType"].asString();
    } else if (desc->mType == STRUCT_TYPE_STRUCT)
    {
        Value memberSet = content["memberSet"];
        for (size_t i = 0 ; i < memberSet.size() ; i++)
        {
            Value single = memberSet[i];
            desc->mMemberType.push_back(single["memberType"].asString());
            desc->mMemberName.push_back(single["memberName"].asString());
            desc->mMemberOffset.push_back(single["memberOffset"].asInt());
        }
    }
    return desc;
}

mstring CDescCache::FunctionToString(const FunDesc *desc) const {
    Value content;
    content["module"] = desc->mDllName;
    content["procName"] = desc->mProcName;
    content["callType"] = (int)desc->mCallType;

    Value paramSet(arrayValue);
    for (vector<ParamDesc>::const_iterator it = desc->mParam.begin() ; it != desc->mParam.end() ; it++)
    {
        Value param;
        param["paramType"] = it->mParamType;
        param["paramName"] = it->mParamName;
        paramSet.append(param);
    }
    content["paramSet"] = paramSet;
    content["returnType"] = desc->mReturn.mReturnType;
    return FastWriter().write(content);
}

FunDesc *CDescCache::StringToFunction(const mstring &str) const {
    Value content;
    Reader().parse(str, content);

    FunDesc *ptr = new FunDesc();
    ptr->mDllName = content["module"].asString();
    ptr->mProcName = content["procName"].asString();
    ptr->mCallType = (ProcCallType)content["callType"].asInt();

    Value paramSet = content["paramSet"];
    for (size_t i = 0 ; i != paramSet.size() ; i++)
    {
        Value p = paramSet[i];
        ParamDesc param;
        param.mParamType = p["paramType"].asString();
        param.mParamName = p["paramName"].asString();
        param.mStruct = GetStructByName(param.mParamType);
    }
    ptr->mReturn.mReturnType = content["returnType"].asString();
    ptr->mReturn.mStruct = GetStructByName(ptr->mReturn.mReturnType);
    return ptr;
}