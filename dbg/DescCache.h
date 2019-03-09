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

    //��desc��������db���ڴ���
    bool InsertStructToDb(StructDesc *structDesc);
    bool InsertFunToDb(FunDesc *funDesc);

    //��desc�������뻺������ʱʹ��
    bool InsertStructToCache(StructDesc *structDesc);
    //�����ʱ����
    void ClearTempCache();
    //�ṹ�����Ƿ���db�洢��,��Ϊ���ֽ���db������
    bool IsStructInDb(const StructDesc *structDesc);
    //���������Ƿ���db�洢��
    bool IsFunctionInDb(const FunDesc *funDesc);

    //��Db�洢����ʱ�����в�ѯ�ṹ����
    StructDesc *GetStructByName(const mstring &name) const;
    list<FunDesc *> GetFunByName(const mstring &dll, const mstring &fun) const;

    StructDesc *GetLinkDescByType(int level, const mstring &linkName) const;
    StructDesc *GetLinkDescByDesc(int level, StructDesc *desc) const;
    StructDesc *CreatePtrStruct() const;
    //��ȡ��ʽ����ɵĽ��
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
    //��ʱ���ڴ��е��������棬��ʱʹ��
    map<mstring, StructDesc *> mTempCache;
    //db�е��������棬�ڴ��dbͬ��
    map<mstring, StructDesc *> mStructCache;
    map<mstring, list<FunDesc *>> mFunSetByFunction;
};
#endif //DESCCACHE_H_H_H_