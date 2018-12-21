#include <Windows.h>
#include <Shlwapi.h>
#include <set>
#include "CmdBase.h"
#include "symbol.h"
//#include "MainView.h"
//#include <SyntaxHlpr/SyntaxParser.h>

CCmdBase::CCmdBase()
{}

CCmdBase::~CCmdBase()
{}

/*
bool CCmdBase::OnFilter(SyntaxDesc &desc, const ustring &wstrFilter) const
{
    vector<mstring>::const_iterator it = desc.m_vShowInfo.begin();
    vector<vector<SyntaxColourNode>> ::const_iterator itDesc;
    itDesc = desc.m_vSyntaxDesc.begin();
    int index = 0;
    while (it != desc.m_vShowInfo.end())
    {
        ustring strLow(*it);
        strLow.makelower();
        if (ustring::npos == strLow.find(wstrFilter))
        {
            it = desc.m_vShowInfo.erase(it);
            desc.m_vSyntaxDesc.erase(desc.m_vSyntaxDesc.begin() + index);
        }
        else
        {
            it++;
            index++;
        }
    }
    return true;
}

bool CCmdBase::OnHight(SyntaxDesc &desc, const ustring &wstrHight) const
{
    vector<vector<SyntaxColourNode>> ::iterator itDesc = desc.m_vSyntaxDesc.begin();
    vector<SyntaxColourNode>::iterator itNode;
    int iSerial = 0;
    for (vector<mstring>::const_iterator it = desc.m_vShowInfo.begin() ; it != desc.m_vShowInfo.end() ; it++, iSerial++, itDesc++)
    {
        ustring wstrLow(*it);
        wstrLow.makelower();
        if (ustring::npos != wstrLow.find(wstrHight))
        {
            for (itNode = itDesc->begin() ; itNode != itDesc->end() ; itNode++)
            {
                itNode->m_vHightLightDesc = COLOUR_HIGHT;
            }
        }
    }
    return true;
}
*/

DbgCmdResult CCmdBase::RunCommand(const ustring &wstrCmd, BOOL bShow, const CmdUserParam *pParam)
{
    DbgCmdResult res;
    ustring wstr(wstrCmd);
    wstr.makelower();
    wstr.trim();
    if (wstr.empty())
    {
        return res;
    }

    bool bFilter = false;
    ustring wstrFilter;
    bool bHight = false;
    ustring wstrHight;
    bFilter = IsFilterStr(wstr, wstrFilter);
    bHight = IsHightStr(wstr, wstrHight);
    wstr.trim();

    ustring wstrStart;
    ustring wstrParam;
    size_t i = wstr.find(L" ");
    if (ustring::npos == i)
    {
        wstrStart = wstr;
    }
    else
    {
        wstrStart = wstr.substr(0, i);
        wstrParam = wstr.c_str() + i;
        wstrParam.trim();
    }
    res = OnCommand(wstrStart, wstrParam, bShow, pParam);

    //GetSyntaxView()->AppendText(SCI_LABEL_DEFAULT, "\n");
    if (bFilter)
    {
        //OnFilter(res.m_vSyntaxDesc, wstrFilter);
    }
    else if (bHight)
    {
        //OnHight(res.m_vSyntaxDesc, wstrHight);
    }
    return res;
}

DWORD64 CCmdBase::GetFunAddr(const ustring &wstr)
{
    ustring wstrContent(wstr);
    wstrContent.makelower();
    wstrContent.trim();
    ustring wstrFun;
    ustring wstrOffset;
    size_t pos = 0;
    if (ustring::npos != (pos = wstrContent.find(L"+")))
    {
        wstrFun = wstrContent.substr(0, pos);
        wstrOffset = (wstrContent.c_str() + pos + 1);
    }
    else
    {
        wstrFun = wstrContent;
    }

    DWORD64 dwOffset = 0;
    GetNumFromStr(wstrOffset, dwOffset);

    CSymbolTaskHeader header;
    CTaskGetAddrFromStr param;
    header.m_dwSize = sizeof(header) + sizeof(param);
    header.m_pParam = &param;
    header.m_eTaskType = em_task_addrfromstr;
    param.m_wstrStr = wstrContent;
    GetSymbolHlpr()->SendTask(&header);
    return (param.m_dwAddr + dwOffset);
}

//0x12abcd
//0n115446
//323ffabc
bool CCmdBase::IsNumber(const ustring &wstr) const
{
    if (wstr.empty())
    {
        return false;
    }

    ustring wstrLower(wstr);
    wstrLower.makelower();
    bool b16 = true;
    size_t i = 0;
    if (wstrLower.startwith(L"0n"))
    {
        b16 = false;
    }

    if (wstrLower.startwith(L"0n") || wstrLower.startwith(L"0x"))
    {
        i += 2;
    }

    bool bResult = false;
    for (; i < wstrLower.size() ; i++)
    {
        if (b16)
        {
            if (!((wstrLower[i] >= '0' && wstrLower[i] <= '9') || (wstrLower[i] >= 'a' && wstrLower[i] <= 'f')))
            {
                return false;
            }
        }
        else
        {
            if (!(wstrLower[i] >= '0' && wstrLower[i] <= '9'))
            {
                return false;
            }
        }
        bResult = true;
    }
    return bResult;
}

bool CCmdBase::IsKeyword(const ustring &wstr) const
{
    static set<ustring> *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new set<ustring>();
        s_ptr->insert(L"ptr");
        s_ptr->insert(L"word"), s_ptr->insert(L"byte");
        s_ptr->insert(L"dword"), s_ptr->insert(L"qword");
    }
    ustring wstrLow(wstr);
    wstrLow.makelower();
    return (s_ptr->end() != s_ptr->find(wstrLow));
}

vector<WordNode> CCmdBase::GetWordSet(const ustring &wstrStr) const
{
    static set<ustring> *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new set<ustring>();
        s_ptr->insert(L" "), s_ptr->insert(L",");
        s_ptr->insert(L"("), s_ptr->insert(L")");
        s_ptr->insert(L"["), s_ptr->insert(L"]");
        s_ptr->insert(L"{"), s_ptr->insert(L"}");
        s_ptr->insert(L"*"), s_ptr->insert(L"+");
        s_ptr->insert(L"-");
    }

    vector<WordNode> vResult;
    size_t iLastPos = 0;
    WordNode node;
    for (size_t i = 0 ; i < wstrStr.size() ;)
    {
        while (i < wstrStr.size() && (s_ptr->end() != s_ptr->find(wstrStr[i])))
        {
            i++;
            continue;
        }

        if (i == wstrStr.size())
        {
            break;
        }

        if (i > iLastPos)
        {
            node.m_iStartPos = iLastPos;
            node.m_iLength = (i - iLastPos);
            node.m_wstrContent = wstrStr.substr(iLastPos, i - iLastPos);
            vResult.push_back(node);
        }

        iLastPos = i;
        size_t j = i + 1;
        while (j < wstrStr.size() && (s_ptr->end() == s_ptr->find(wstrStr[j])))
        {
            j++;
        }
        node.m_iStartPos = iLastPos;
        node.m_iLength = (j - iLastPos);
        node.m_wstrContent = wstrStr.substr(iLastPos, j - iLastPos);
        vResult.push_back(node);
        i = j;
        iLastPos = j;
    }

    if (wstrStr.size() > iLastPos)
    {
        node.m_iStartPos = iLastPos;
        node.m_iLength = wstrStr.size() - iLastPos;
        node.m_wstrContent = wstrStr.substr(iLastPos, node.m_iLength);
        vResult.push_back(node);
    }
    return vResult;
}

//0x43fdad12
//0n12232433
//5454546455
BOOL CCmdBase::GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwResult) const
{
    ustring wstr(wstrNumber);
    wstr.makelower();
    if (wstr.startwith(L"0n"))
    {
        return StrToInt64ExW(wstrNumber.c_str() + 2, STIF_DEFAULT, (LONGLONG *)&dwResult);
    }
    else
    {
        if (!wstr.startwith(L"0x"))
        {
            wstr.insert(0, L"0x");
        }
        return StrToInt64ExW(wstr.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
    }
}

bool CCmdBase::IsFilterStr(ustring &wstrData, ustring &wstrFilter) const
{
    ustring wstr(wstrData);
    wstr.makelower();
    size_t pos = 0;
    if (ustring::npos == (pos = wstr.rfind(L">> ")))
    {
        return false;
    }

    ustring wstrCmd = wstrData.substr(0, pos);
    ustring wstrSub = wstr.substr(pos);
    pos = wstrSub.find(L"ft ");
    if (ustring::npos == pos)
    {
        return false;
    }
    wstrSub.erase(0, pos + 3);
    wstrSub.trim();
    if (wstrSub.empty())
    {
        return false;
    }
    wstrFilter = wstrSub;
    wstrData = wstrCmd;
    return true;
}

bool CCmdBase::IsHightStr(ustring &wstrData, ustring &wstrHight) const
{
    ustring wstr(wstrData);
    wstr.makelower();
    size_t pos = 0;
    if (ustring::npos == (pos = wstr.rfind(L">> ")))
    {
        return false;
    }

    ustring wstrCmd = wstrData.substr(0, pos);
    ustring wstrSub = wstr.substr(pos);
    pos = wstrSub.find(L"ht ");
    if (ustring::npos == pos)
    {
        return false;
    }
    wstrSub.erase(0, pos + 3);
    wstrSub.trim();
    if (wstrSub.empty())
    {
        return false;
    }
    wstrHight = wstrSub;
    wstrData = wstrCmd;
    return true;
}

DWORD64 CCmdBase::GetSizeAndParam(const ustring &wstrParam, ustring &wstrOut) const
{
    ustring wstr(wstrParam);
    wstr.makelower();
    wstr.trim();
    wstrOut = wstr;

    DWORD64 dwSize = -1;
    if (wstr.startwith(L"l"))
    {
        size_t iEndPos = wstr.find(L" ");
        if (ustring::npos == iEndPos)
        {
            return 0;
        }
        GetNumFromStr(wstr.substr(1, iEndPos - 1), dwSize);
        wstrOut = wstr.c_str() + iEndPos;
        wstrOut.trim();
    }
    return dwSize;
}

DbgCmdResult CCmdBase::OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam)
{
    return DbgCmdResult();
}