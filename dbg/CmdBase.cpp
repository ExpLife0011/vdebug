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
bool CCmdBase::OnFilter(SyntaxDesc &desc, const mstring &wstrFilter) const
{
    vector<mstring>::const_iterator it = desc.m_vShowInfo.begin();
    vector<vector<SyntaxColourNode>> ::const_iterator itDesc;
    itDesc = desc.m_vSyntaxDesc.begin();
    int index = 0;
    while (it != desc.m_vShowInfo.end())
    {
        mstring strLow(*it);
        strLow.makelower();
        if (mstring::npos == strLow.find(wstrFilter))
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

bool CCmdBase::OnHight(SyntaxDesc &desc, const mstring &wstrHight) const
{
    vector<vector<SyntaxColourNode>> ::iterator itDesc = desc.m_vSyntaxDesc.begin();
    vector<SyntaxColourNode>::iterator itNode;
    int iSerial = 0;
    for (vector<mstring>::const_iterator it = desc.m_vShowInfo.begin() ; it != desc.m_vShowInfo.end() ; it++, iSerial++, itDesc++)
    {
        mstring wstrLow(*it);
        wstrLow.makelower();
        if (mstring::npos != wstrLow.find(wstrHight))
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

CtrlReply CCmdBase::RunCommand(const mstring &cmd, HUserCtx ctx)
{
    CtrlReply result;
    mstring str(cmd);
    str.makelower();
    str.trim();
    if (str.empty())
    {
        return result;
    }

    bool bFilter = false;
    mstring wstrFilter;
    bool bHight = false;
    mstring wstrHight;
    bFilter = IsFilterStr(str, wstrFilter);
    bHight = IsHightStr(str, wstrHight);
    str.trim();

    mstring strStart;
    mstring strParam;
    size_t i = str.find(" ");
    if (mstring::npos == i)
    {
        strStart = str;
    }
    else
    {
        strStart = str.substr(0, i);
        strParam = str.c_str() + i;
        strParam.trim();
    }

    result = OnCommand(strStart, strParam, ctx);
    return result;
}

bool CCmdBase::IsCommand(const mstring &command, CmdHandlerInfo &info) const {
    mstring str = command;
    size_t pos = command.find(" ");
    if (mstring::npos != pos)
    {
        str = command.substr(0, pos);
    }

    map<mstring, CmdHandlerInfo>::const_iterator it = mCmdHandler.find(str);
    if (mCmdHandler.end() != it) {
        info = it->second;
        return true;
    }
    return false;
}

DWORD64 CCmdBase::GetFunAddr(const mstring &wstr)
{
    mstring strContent(wstr);
    strContent.makelower();
    strContent.trim();
    mstring strFun;
    mstring strOffset;
    size_t pos = 0;
    if (mstring::npos != (pos = strContent.find("+")))
    {
        strFun = strContent.substr(0, pos);
        strOffset = (strContent.c_str() + pos + 1);
    }
    else
    {
        strFun = strContent;
    }

    DWORD64 dwOffset = 0;
    GetNumFromStr(strOffset, dwOffset);

    CSymbolTaskHeader header;
    CTaskGetAddrFromStr param;
    header.m_dwSize = sizeof(header) + sizeof(param);
    header.m_pParam = &param;
    header.m_eTaskType = em_task_addrfromstr;
    param.m_strStr = strContent;
    CSymbolHlpr::GetInst()->SendTask(&header);
    return (param.m_dwAddr + dwOffset);
}

//0x12abcd
//0n115446
//323ffabc
bool CCmdBase::IsNumber(const mstring &str)
{
    if (str.empty())
    {
        return false;
    }

    mstring strLower(str);
    strLower.makelower();
    bool b16 = true;
    size_t i = 0;
    if (strLower.startwith("0n"))
    {
        b16 = false;
    }

    if (strLower.startwith("0n") || strLower.startwith("0x"))
    {
        i += 2;
    }

    bool bResult = false;
    for (; i < strLower.size() ; i++)
    {
        if (b16)
        {
            if (!((strLower[i] >= '0' && strLower[i] <= '9') || (strLower[i] >= 'a' && strLower[i] <= 'f')))
            {
                return false;
            }
        }
        else
        {
            if (!(strLower[i] >= '0' && strLower[i] <= '9'))
            {
                return false;
            }
        }
        bResult = true;
    }
    return bResult;
}

bool CCmdBase::IsKeyword(const mstring &wstr)
{
    static set<mstring> *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new set<mstring>();
        s_ptr->insert("ptr");
        s_ptr->insert("word"), s_ptr->insert("byte");
        s_ptr->insert("dword"), s_ptr->insert("qword");
    }
    mstring wstrLow(wstr);
    wstrLow.makelower();
    return (s_ptr->end() != s_ptr->find(wstrLow));
}

vector<WordNode> CCmdBase::GetWordSet(const mstring &strStr)
{
    static set<mstring> *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new set<mstring>();
        s_ptr->insert(" "), s_ptr->insert(",");
        s_ptr->insert("("), s_ptr->insert(")");
        s_ptr->insert("["), s_ptr->insert("]");
        s_ptr->insert("{"), s_ptr->insert("}");
        s_ptr->insert("*"), s_ptr->insert("+");
        s_ptr->insert("-");
    }

    vector<WordNode> vResult;
    size_t iLastPos = 0;
    WordNode node;
    for (size_t i = 0 ; i < strStr.size() ;)
    {
        while (i < strStr.size() && (s_ptr->end() != s_ptr->find(strStr[i])))
        {
            i++;
            continue;
        }

        if (i == strStr.size())
        {
            break;
        }

        if (i > iLastPos)
        {
            node.m_iStartPos = iLastPos;
            node.m_iLength = (i - iLastPos);
            node.m_strContent = strStr.substr(iLastPos, i - iLastPos);
            vResult.push_back(node);
        }

        iLastPos = i;
        size_t j = i + 1;
        while (j < strStr.size() && (s_ptr->end() == s_ptr->find(strStr[j])))
        {
            j++;
        }
        node.m_iStartPos = iLastPos;
        node.m_iLength = (j - iLastPos);
        node.m_strContent = strStr.substr(iLastPos, j - iLastPos);
        vResult.push_back(node);
        i = j;
        iLastPos = j;
    }

    if (strStr.size() > iLastPos)
    {
        node.m_iStartPos = iLastPos;
        node.m_iLength = strStr.size() - iLastPos;
        node.m_strContent = strStr.substr(iLastPos, node.m_iLength);
        vResult.push_back(node);
    }
    return vResult;
}

//0x43fdad12
//0n12232433
//5454546455
BOOL CCmdBase::GetNumFromStr(const mstring &strNumber, DWORD64 &dwResult)
{
    mstring str(strNumber);
    str.makelower();
    if (str.startwith("0n"))
    {
        return StrToInt64ExA(strNumber.c_str() + 2, STIF_DEFAULT, (LONGLONG *)&dwResult);
    }
    else
    {
        if (!str.startwith("0x"))
        {
            str.insert(0, "0x");
        }
        return StrToInt64ExA(str.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
    }
}

bool CCmdBase::IsFilterStr(mstring &strData, mstring &strFilter)
{
    mstring str(strData);
    str.makelower();
    size_t pos = 0;
    if (mstring::npos == (pos = str.rfind(">> ")))
    {
        return false;
    }

    mstring strCmd = strData.substr(0, pos);
    mstring strSub = str.substr(pos);
    pos = strSub.find("ft ");
    if (mstring::npos == pos)
    {
        return false;
    }
    strSub.erase(0, pos + 3);
    strSub.trim();
    if (strSub.empty())
    {
        return false;
    }
    strFilter = strSub;
    strData = strCmd;
    return true;
}

bool CCmdBase::IsHightStr(mstring &strData, mstring &strHight)
{
    mstring str(strData);
    str.makelower();
    size_t pos = 0;
    if (mstring::npos == (pos = str.rfind(">> ")))
    {
        return false;
    }

    mstring strCmd = strData.substr(0, pos);
    mstring strSub = str.substr(pos);
    pos = strSub.find("ht ");
    if (mstring::npos == pos)
    {
        return false;
    }
    strSub.erase(0, pos + 3);
    strSub.trim();
    if (strSub.empty())
    {
        return false;
    }
    strHight = strSub;
    strData = strCmd;
    return true;
}

DWORD64 CCmdBase::GetSizeAndParam(const mstring &strParam, mstring &strOut)
{
    mstring str(strParam);
    str.makelower();
    str.trim();
    strOut = str;

    DWORD64 dwSize = -1;
    if (str.startwith("l"))
    {
        size_t iEndPos = str.find(" ");
        if (mstring::npos == iEndPos)
        {
            return 0;
        }
        GetNumFromStr(str.substr(1, iEndPos - 1), dwSize);
        strOut = str.c_str() + iEndPos;
        strOut.trim();
    }
    return dwSize;
}

CtrlReply CCmdBase::OnCommand(const mstring &cmd, const mstring &param, HUserCtx ctx)
{
    CScopedLocker locker(this);
    map<mstring, CmdHandlerInfo>::const_iterator it = mCmdHandler.find(cmd);

    if (it == mCmdHandler.end())
    {
        CtrlReply reply;
        reply.mShow = mstring("不支持的命令:") + cmd + "\n";
        return reply;
    } else {
        return it->second.mHandler(param, ctx);
    }
}

void CCmdBase::RegisterHandler(const mstring &cmd, CmdDbgType type, pfnCmdHandler handler) {
    CmdHandlerInfo info;
    info.mCommand = cmd;
    info.mCmdType = type;
    info.mHandler = handler;
    mCmdHandler[cmd] = info;
}