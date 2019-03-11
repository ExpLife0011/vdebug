#include <Windows.h>
#include <Shlwapi.h>
#include <ComLib/ComLib.h>
#include <ComStatic/StrUtil.h>
#include "DescCache.h"

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

//结构描述是否在db存储中,因为部分仅在db缓存中
bool CDescCache::IsStructInDb(const StructDesc *structDesc) {
    SqliteOperator opt(mDbPath);

    mstring tmp = structDesc->mTypeName;
    tmp.makelower();
    mstring sql = FormatA("select count(1) from tStructDesc where name='%hs'", tmp.c_str());
    SqliteResult result = opt.Select(sql);
    mstring err = opt.GetError();
    mstring dd = result.begin().GetValue("count(1)");
    return (result.begin().GetValue("count(1)") != "0");
}

//函数描述是否在db存储中
bool CDescCache::IsFunctionInDb(const FunDesc *funDesc) {
    SqliteOperator opt(mDbPath);

    mstring tmp1 = funDesc->mDllName;
    tmp1.makelower();
    mstring tmp2 = funDesc->mProcName;
    tmp2.makelower();
    mstring sql = FormatA("select count(1) from tFunctionDesc where module='%hs' and name='%hs'", tmp1.c_str(), tmp2.c_str());
    SqliteResult result = opt.Select(sql);
    mstring err = opt.GetError();
    return (result.begin().GetValue("count(1)") != "0");
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

        mstring tmp = *it;
        tmp.makelower();
        mStructCache[tmp] = newPtr;
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

        mstring tmp = *it;
        tmp.makelower();
        mStructCache[tmp] = newPtr;
    }
    return true;
}

bool CDescCache::LinkPtr(const mstring &nameSet, const mstring &linked) {
    mstring tmp = linked;
    tmp.makelower();
    map<mstring, StructDesc *>::const_iterator it = mStructCache.find(tmp);

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

        tmp = *ij;
        tmp.makelower();
        mStructCache[tmp] = newPtr;
    }
    return true;
}

bool CDescCache::LoadNewStructFromDb() {
    SqliteOperator opt(mDbPath);
    mstring sql = FormatA("select id, content from tStructDesc where id > %d", mLastStructUpdateId);
    SqliteResult result = opt.Select(sql.c_str());

    map<mstring, StructDesc *> cache;
    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        int id = atoi(it.GetValue("id").c_str());
        if (id > mLastStructUpdateId)
        {
            mLastStructUpdateId = id;
        }

        StructDesc *desc = StringToStruct(it.GetValue("content"));
        mstring tmp = desc->mTypeName;
        tmp.makelower();
        cache[tmp] = desc;
    }

    list<StructDesc *> structSet;
    map<mstring, StructDesc *>::const_iterator it;
    for (it = cache.begin() ; it != cache.end() ; it++)
    {
        StructDesc *ptr = it->second;

        //填充指针长度
        if (ptr->mType == STRUCT_TYPE_PTR)
        {
            ptr->mLength = sizeof(void *);
            mstring tmp2 = ptr->mEndType;
            tmp2.makelower();
            map<mstring, StructDesc *>::const_iterator ij = cache.find(tmp2);
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
            //desc->mMemberType.push_back(single["memberType"].asString());
            for (vector<mstring>::const_iterator it2 = ptr->mMemberType.begin() ; it2 != ptr->mMemberType.end() ; it2++)
            {
                mstring tmpStr = *it2;
                tmpStr.makelower();
                map<mstring, StructDesc *>::const_iterator i3 = mStructCache.find(tmpStr);

                if (i3 == mStructCache.end())
                {
                    i3 = cache.find(tmpStr);

                    if (i3 == cache.end())
                    {
                        throw (new CParserException(FormatA("未识别的类型:%hs", it2->c_str())));
                    }
                }

                ptr->mMemberSet.push_back(i3->second);
            }
            structSet.push_back(ptr);
        }
    }

    //计算结构体长度和成员偏移,因为可能会出现结构体之间互相依赖
    //的问题，需要循环逐级进行结构体成员数据的填充
    while (!structSet.empty()) {
        int countStart = structSet.size();
        for (list<StructDesc *>::iterator ik = structSet.begin() ; ik != structSet.end() ;)
        {
            StructDesc *tmp = *ik;
            if (!tmp->mLength)
            {
                int offset1 = 0;
                size_t s = 0;
                for (s = 0 ; s < tmp->mMemberSet.size() ; s++)
                {
                    StructDesc *tmp2 = tmp->mMemberSet[s];
                    if (!tmp2->mLength)
                    {
                        break;
                    }
                    tmp->mMemberOffset.push_back(offset1);
                    offset1 += tmp2->mLength;
                }

                if (s == tmp->mMemberSet.size())
                {
                    tmp->mLength = offset1;
                    ik = structSet.erase(ik);
                } else {
                    ik++;
                }
            }
        }

        if (countStart == structSet.size())
        {
            throw new CParserException("struct依赖序存在问题");
        }
    }
    mStructCache.insert(cache.begin(), cache.end());
    return true;
}

bool CDescCache::LoadNewFunctionFromDb() {
    SqliteOperator opt(mDbPath);
    mstring sql = FormatA("select id, content from tFunctionDesc where id > %d", mLastFunctionUpdateId);
    SqliteResult result = opt.Select(sql.c_str());

    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        FunDesc *desc = StringToFunction(it.GetValue("content"));
        int id = atoi(it.GetValue("id").c_str());

        if (id > mLastFunctionUpdateId)
        {
            mLastFunctionUpdateId = id;
        }

        mstring d = FormatA("%hs!%hs", desc->mDllName.c_str(), desc->mProcName.c_str());
        mstring tmp = desc->mDllName;
        tmp.makelower();
        mFunSetByFunction[tmp].push_back(desc);
    }
    return true;
}

bool CDescCache::UpdateStructToDb(const StructDesc *desc, const mstring &content) const {
    SqliteOperator opt(mDbPath);
    mstring tmp = desc->mTypeName;
    tmp.makelower();
    SqliteResult result = opt.Select(FormatA("select count(1) from tStructDesc where name='%hs'", tmp.c_str()));

    int count = atoi(result.begin().GetValue("count(1)").c_str());
    mstring curTime = GetCurTimeStr1(TIME_FORMAT1);
    if (0 == count)
    {
        opt.Insert(
            FormatA("insert into tStructDesc(name, content, checksum, time)values('%hs', '%hs', %u, '%hs')",
            tmp.c_str(),
            content.c_str(),
            desc->mCheckSum,
            curTime.c_str())
            );
    } else {
        opt.Update(
            FormatA(
            "update tStructDesc set checksum=%u, content='%hs', time='%hs' where name='%hs'",
            desc->mCheckSum,
            content.c_str(),
            curTime.c_str(),
            tmp.c_str())
            );
    }
    return true;
}

bool CDescCache::UpdateFunctionToDb(FunDesc *desc, const mstring &content) const {
    SqliteOperator opt(mDbPath);
    mstring tmp1 = desc->mDllName;
    tmp1.makelower();
    mstring tmp2 = desc->mProcName;
    tmp2.makelower();

    SqliteResult result = opt.Select(FormatA("select count(1) from tFunctionDesc where module='%hs' and name='%hs'", tmp1.c_str(), tmp2.c_str()));

    int count = atoi(result.begin().GetValue("count(1)").c_str());
    mstring curTime = GetCurTimeStr1(TIME_FORMAT1);
    if (0 == count)
    {
        opt.Insert(
            FormatA("insert into tFunctionDesc(module, name, content, checksum, time)values('%hs', '%hs', '%hs', %u, '%hs')",
            tmp1.c_str(),
            tmp2.c_str(),
            content.c_str(),
            desc->mCheckSum,
            curTime.c_str())
            );
    } else {
        opt.Update(
            FormatA(
            "update tFunctionDesc set checksum=%u, content='%hs', time='%hs' where module='%hs' and name='%hs'",
            desc->mCheckSum,
            content.c_str(),
            curTime.c_str(),
            tmp1.c_str(),
            tmp2.c_str())
            );
    }
    return true;
}

bool CDescCache::InitDescCache() {
    char dbPath[256] = {0};
    GetModuleFileNameA(NULL, dbPath, 256);
    PathAppendA(dbPath, "..\\..\\db\\DescDb.db");
    mDbPath = dbPath;

    SqliteOperator opt(mDbPath);
    opt.Exec("create table if not exists tStructDesc (id INTEGER PRIMARY KEY, name TEXT, content TEXT, checksum BIGINT, time CHAR(32))");
    opt.Exec("create table if not exists tFunctionDesc (id INTEGER PRIMARY KEY, module TEXT, name TEXT,content TEXT, checksum BIGINT, time CHAR(32))");

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
    InsertBaseType(STRUCT_TYPE_BASETYPE, "unsigned int;ULONG;UINT;uint32_t;UINT32;DWORD", 4, "0x%x(%u)");

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

    LoadNewStructFromDb();
    LoadNewFunctionFromDb();
    return true;
}

bool CDescCache::InsertStructToDb(StructDesc *desc) {
    mstring str = StructToString(desc);
    desc->mCheckSum = crc32(str.c_str(), str.size(), 0xffee1122);

    mstring tmp = desc->mTypeName;
    tmp.makelower();
    mStructCache[tmp] = desc;

    UpdateStructToDb(desc, str);
    UpdateTimeStamp();
    return true;
}

bool CDescCache::InsertFunToDb(FunDesc *funDesc) {
    mstring d = FormatA("%hs!%hs", funDesc->mDllName.c_str(), funDesc->mProcName.c_str());
    mstring str = FunctionToString(funDesc);
    funDesc->mCheckSum = crc32(str.c_str(), str.size(), 0xffee1122);
    mstring tmp = funDesc->mDllName;
    tmp.makelower();
    mFunSetByFunction[tmp].push_back(funDesc);

    UpdateFunctionToDb(funDesc, str);
    UpdateTimeStamp();
    return true;
}

bool CDescCache::InsertStructToCache(StructDesc *structDesc) {
    mTempCache[structDesc->mTypeName] = structDesc;
    return true;
}

void CDescCache::ClearTempCache() {
    mTempCache.clear();
}

StructDesc *CDescCache::GetStructByName(const mstring &name) const {
    mstring tmp = name;
    tmp.makelower();
    map<mstring, StructDesc *>::const_iterator it = mStructCache.find(tmp);

    if (it != mStructCache.end())
    {
        return it->second;
    }

    it = mTempCache.find(name);
    if (it != mTempCache.end())
    {
        return it->second;
    }
    return NULL;
}

list<FunDesc *> CDescCache::GetFunByName(const mstring &dll, const mstring &fun) const {
    mstring tmp1 = dll;
    tmp1.makelower();
    mstring tmp2 = fun;
    tmp2.makelower();
    map<mstring, list<FunDesc *>>::const_iterator it = mFunSetByFunction.find(tmp1);
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
        mstring tmp3 = (*ij)->mDllName;
        tmp3.makelower();

        if (tmp3 == dll)
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
    mLastStructUpdateId = 0;
    mLastFunctionUpdateId = 0;
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
        ptr->mParam.push_back(param);
    }
    ptr->mReturn.mReturnType = content["returnType"].asString();
    ptr->mReturn.mStruct = GetStructByName(ptr->mReturn.mReturnType);
    return ptr;
}

bool CDescCache::UpdateTimeStamp() const {
    static DWORD s_serial = 0;
    mstring time = GetCurTimeStr1("%04d%02d%02d_%02d%02d%02d%03d");
    mstring content = FormatA("%d_%d_%hs", GetCurrentProcessId(), s_serial++, time.c_str());

    RegSetStrValueA(HKEY_LOCAL_MACHINE, REG_VDEBUG_DESC, "lastUpdate", content.c_str());
    return true;

}

DWORD CDescCache::ImportThread(LPVOID pParam) {
    HKEY notifyKey = NULL;
    HANDLE hNotifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    RegCreateKeyA(HKEY_LOCAL_MACHINE, REG_VDEBUG_DESC, &notifyKey);
    RegNotifyChangeKeyValue(notifyKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, hNotifyEvent, TRUE);

    while (true) {
        WaitForSingleObject(hNotifyEvent, 1000 * 10);
        GetInst()->LoadNewStructFromDb();
        GetInst()->LoadNewFunctionFromDb();
    }
    CloseHandle(hNotifyEvent);
    RegCloseKey(notifyKey);
    return 0;
}