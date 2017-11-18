#ifndef INDEX_VDEBUG_H_H_
#define INDEX_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <set>
#include <list>
#include "LockBase.h"
#include "common.h"

using namespace std;

enum IndexType
{
    em_index_command,       //查询命令
    em_index_symbol,        //查询符号
    em_index_unknow         //未知类型，全部
};

struct IndexInfo
{
    //ustring m_wstrContent;
    //IndexType m_eIndexType;
    //LPVOID m_ptr;

    IndexType m_eIndexClass;
    ustring m_wstrShowContent;
    ustring m_wstrJsonStr;
    Value m_vJsonData;
    //set<ustring> m_vIndex;
    ustring m_wstrIndex;
    ustring m_wstrMark;

    IndexInfo() : m_eIndexClass(em_index_unknow)
    {}
};

/*
{
    "type":0,                           //公有字段：索引类型
    "index":"abcdef"                    //公有字段：索引内容
    "show":"nt!CreateFile 0x1234ffff",  //公有字段：展示内容

    //私有字段
}
*/
#define INVALID_INDEX_HANDLE    0xffffffffffffffff
typedef DWORD64 IndexHandle;

class CIndexEngine : public CCriticalSectionLockable
{
public:
    CIndexEngine();

    virtual ~CIndexEngine();

    IndexHandle RegisterIndex(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vContent);//注册索引
    set<IndexInfo *> GetStrictIndex(const ustring &wstrIndex) const;        //严格匹配
    set<IndexInfo *> GetFuzzyIndex(const ustring &wstrIndex) const;         //模糊匹配
    VOID Close(IndexHandle hIndex);                                         //移除索引

protected:
    Value FillContent(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vData);   //填充数据
    ustring GetUniqueMark(const Value &vContent) const;

protected:
    map<ustring, set<IndexInfo *>> m_vIndexFull;        //索引和IndexInfo映射
    map<int, map<ustring, set<IndexInfo *>>> m_vIndex;  //快速检索映射
    map<ustring, IndexInfo *> m_vUniqueSet;             //唯一标识和IndexInfo映射
    map<IndexHandle, IndexInfo *> m_vHandleMap;         //句柄和IndexInfo映射
    IndexHandle m_dwIndexSerial;                        //提供给用户使用的句柄，主要是用于删除
};

CIndexEngine *GetIndexEngine();

#endif