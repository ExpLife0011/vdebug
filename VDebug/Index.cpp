#include <Windows.h>
#include "Index.h"
#include "md5.h"

#define MAX_INDEX 4

CIndexEngine::CIndexEngine() : m_dwIndexSerial(0)
{}

CIndexEngine::~CIndexEngine()
{}

IndexHandle CIndexEngine::RegisterIndex(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vContent)
{
    Value vResult = FillContent(eType, wstrIndex, wstrShow, vContent);
    ustring wstr = FastWriter().write(vResult);
    ustring wstrUnique = GetUniqueMark(vResult);
    CScopedLocker lock(this);
    map<ustring, IndexInfo *>::const_iterator it;
    IndexInfo *ptr = NULL;
    if (m_vUniqueSet.end() != (it = m_vUniqueSet.find(wstrUnique)))
    {
        return INVALID_INDEX_HANDLE;
    }

    ptr = new IndexInfo();
    ptr->m_eIndexClass = eType;
    ptr->m_vJsonData = vResult;
    ptr->m_wstrJsonStr = wstr;
    ptr->m_wstrMark = wstrUnique;
    ptr->m_wstrShowContent = wstrShow;
    ptr->m_wstrIndex = wstrIndex;
    m_vUniqueSet[wstrUnique] = ptr;

    ustring wstrTemp(wstrIndex);
    wstrTemp.makelower();
    //ptr->m_vIndex.insert(wstrTemp);
    ptr->m_wstrIndex = wstrTemp;

    m_vIndexFull[wstrIndex].insert(ptr);
    for (int iIdex = 1 ; iIdex <= MAX_INDEX ; iIdex++)
    {
        if (wstr.size() < (size_t)iIdex)
        {
            break;
        }
        m_vIndex[iIdex][wstrTemp.substr(0, iIdex)].insert(ptr);
    }

    IndexHandle dwHandle = (m_dwIndexSerial++);
    m_vHandleMap[dwHandle] = ptr;
    return dwHandle;
}

set<IndexInfo *> CIndexEngine::GetStrictIndex(const ustring &wstrIndex) const
{
    ustring wstr(wstrIndex);
    wstr.makelower();

    CScopedLocker lock(this);
    map<ustring, set<IndexInfo *>>::const_iterator it;
    if (m_vIndexFull.end() != (it = m_vIndexFull.find(wstr)))
    {
        return it->second;
    }
    return set<IndexInfo *>();
}

set<IndexInfo *> CIndexEngine::GetFuzzyIndex(const ustring &wstrIndex) const
{
    ustring wstr(wstrIndex);
    wstr.makelower();

    if (wstr.empty())
    {
        return set<IndexInfo *>();
    }

    CScopedLocker(this);
    map<int, map<ustring, set<IndexInfo *>>>::const_iterator it1;
    map<ustring, set<IndexInfo *>>::const_iterator it2;
    if (m_vIndex.size() < wstr.size())
    {
        ustring wstrStart = wstr.substr(0, m_vIndex.size());
        it1 = m_vIndex.find(wstrStart.size());
        if (it1 == m_vIndex.end())
        {
            return set<IndexInfo *>();
        }
        it2 = it1->second.find(wstrStart);
        if (it2 == it1->second.end())
        {
            return set<IndexInfo *>();
        }

        set<IndexInfo *> vResult;
        for (set<IndexInfo *>::const_iterator itInfo = it2->second.begin() ; itInfo != it2->second.end() ; itInfo++)
        {
            IndexInfo *pInfo = (*itInfo);
            if (0 == pInfo->m_wstrIndex.comparei(wstr, 0))
            {
                vResult.insert(pInfo);
                break;
            }
        }
        return vResult;
    }
    else
    {
        it1 = m_vIndex.find(wstr.size());
        if (it1 == m_vIndex.end())
        {
            return set<IndexInfo *>();
        }
        it2 = it1->second.find(wstr);
        if (it2 == it1->second.end())
        {
            return set<IndexInfo *>();
        }
        return it2->second;
    }
}

ustring CIndexEngine::GetUniqueMark(const Value &vContent) const
{
    Value v(vContent);
    //v["index"] = "";   //计算校验排除索引，因为多个相同的数据可能只有索引不一致
    char md5[64] = {0};
    string str = FastWriter().write(v);
    md5_buffer(str.c_str(), str.size(), md5, 64);
    return md5;
}

VOID CIndexEngine::Close(IndexHandle hIndex)
{
    CScopedLocker lock(this);

    map<IndexHandle, IndexInfo *>::const_iterator itData;
    if ((INVALID_INDEX_HANDLE == hIndex) || (m_vHandleMap.end() == (itData = m_vHandleMap.find(hIndex))))
    {
        return;
    }

    IndexInfo *ptr = itData->second;
    ustring wstrUnique = ptr->m_wstrMark;
    ustring wstr(ptr->m_wstrIndex);
    wstr.makelower();
    for (int iIdex = 1 ; iIdex <= MAX_INDEX ; iIdex++)
    {
        if (wstr.size() < (size_t)iIdex)
        {
            break;
        }

        set<IndexInfo *>::const_iterator it;
        map<ustring, set<IndexInfo *>>::iterator itMap = m_vIndex[iIdex].find(wstr.substr(0, iIdex));

        if (itMap == m_vIndex[iIdex].end())
        {
            ErrMessage(L"Idex Err1");
            break;
        }
        itMap->second.erase(ptr);
    }

    m_vIndexFull[wstr].erase(ptr);
    m_vUniqueSet.erase(wstrUnique);
    m_vHandleMap.erase(hIndex);
    delete ptr;
}

Value CIndexEngine::FillContent(IndexType eType, const ustring &wstrIndex, const ustring &wstrShow, const Value &vData)
{
    Value vResult(vData);
    vResult["type"] = eType;
    vResult["index"] = WtoU(wstrIndex);
    vResult["show"] = WtoU(wstrShow);
    return vResult;
}

CIndexEngine *GetIndexEngine()
{
    static CIndexEngine *s_ptr = new CIndexEngine();
    return s_ptr;
}