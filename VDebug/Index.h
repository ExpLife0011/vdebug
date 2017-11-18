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
    em_index_command,       //��ѯ����
    em_index_symbol,        //��ѯ����
    em_index_unknow         //δ֪���ͣ�ȫ��
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
    "type":0,                           //�����ֶΣ���������
    "index":"abcdef"                    //�����ֶΣ���������
    "show":"nt!CreateFile 0x1234ffff",  //�����ֶΣ�չʾ����

    //˽���ֶ�
}
*/
#define INVALID_INDEX_HANDLE    0xffffffffffffffff
typedef DWORD64 IndexHandle;

class CIndexEngine : public CCriticalSectionLockable
{
public:
    CIndexEngine();

    virtual ~CIndexEngine();

    IndexHandle RegisterIndex(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vContent);//ע������
    set<IndexInfo *> GetStrictIndex(const ustring &wstrIndex) const;        //�ϸ�ƥ��
    set<IndexInfo *> GetFuzzyIndex(const ustring &wstrIndex) const;         //ģ��ƥ��
    VOID Close(IndexHandle hIndex);                                         //�Ƴ�����

protected:
    Value FillContent(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vData);   //�������
    ustring GetUniqueMark(const Value &vContent) const;

protected:
    map<ustring, set<IndexInfo *>> m_vIndexFull;        //������IndexInfoӳ��
    map<int, map<ustring, set<IndexInfo *>>> m_vIndex;  //���ټ���ӳ��
    map<ustring, IndexInfo *> m_vUniqueSet;             //Ψһ��ʶ��IndexInfoӳ��
    map<IndexHandle, IndexInfo *> m_vHandleMap;         //�����IndexInfoӳ��
    IndexHandle m_dwIndexSerial;                        //�ṩ���û�ʹ�õľ������Ҫ������ɾ��
};

CIndexEngine *GetIndexEngine();

#endif