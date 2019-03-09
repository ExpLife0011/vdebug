#ifndef DESCCACHE_H_H_H_
#define DESCCACHE_H_H_H_
#include <Windows.h>
#include <map>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include "DescParser.h"

using namespace std;

class CDescCache {
public:
    static CDescCache *GetInst();
    bool InitDescCache();

    //将desc描述插入db和内存中
    bool InsertStructToDb(StructDesc *structDesc);
    bool InsertFunToDb(FunDesc *funDesc);

    //将desc描述插入缓存中临时使用
    bool InsertStructToCache(StructDesc *structDesc);
    //清空临时缓存
    void ClearTempCache();
    //结构描述是否在db存储中,因为部分仅在db缓存中
    bool IsStructInDb(const StructDesc *structDesc);
    //函数描述是否在db存储中
    bool IsFunctionInDb(const FunDesc *funDesc);

    //从Db存储和临时缓存中查询结构描述
    StructDesc *GetStructByName(const mstring &name) const;
    list<FunDesc *> GetFunByName(const mstring &dll, const mstring &fun) const;

    StructDesc *GetLinkDescByType(int level, const mstring &linkName) const;
    StructDesc *GetLinkDescByDesc(int level, StructDesc *desc) const;
    StructDesc *CreatePtrStruct() const;
    //获取格式化完成的结果
    mstring GetFormatStr(const mstring &fmt, const char *ptr, int length) const;

private:
    CDescCache();
    virtual ~CDescCache();

private:
    mstring StructToString(const StructDesc *desc) const;
    StructDesc *StringToStruct(const mstring &str) const;

    mstring FunctionToString(const FunDesc *desc) const;
    FunDesc *StringToFunction(const mstring &str) const;

    void InsertBaseType(int type, const mstring &nameSet, int length, const mstring &fmt);
    bool InsertVoidPtr(const mstring &nameSet);
    bool LinkPtr(const mstring &nameSet, const mstring &linked);

    bool LoadNewStructFromDb();
    bool LoadNewFunctionFromDb();
    bool UpdateStructToDb(const StructDesc *desc, const mstring &content) const;
    bool UpdateFunctionToDb(FunDesc *desc, const mstring &content) const;
    bool UpdateTimeStamp() const;

    DWORD GetStructUnique(StructDesc *desc) const;
    DWORD GetFunctionUnique(FunDesc *desc) const;
    static DWORD __stdcall ImportThread(LPVOID pParam);

private:
    int mLastStructUpdateId;
    int mLastFunctionUpdateId;
    mstring mDbPath;
    //临时内内存中的描述缓存，临时使用
    map<mstring, StructDesc *> mTempCache;
    //db中的描述缓存，内存和db同步
    map<mstring, StructDesc *> mStructCache;
    map<mstring, list<FunDesc *>> mFunSetByFunction;
};
#endif //DESCCACHE_H_H_H_