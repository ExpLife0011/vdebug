#include "SyntaxDescHlpr.h"
#include "common.h"

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc, DWORD dwFormatLength)
{
    m_vCurrentDesc.push_back(SyntaxColourNode(wstrInfo, m_wstrCurrentLine.size(), vDesc));
    m_wstrCurrentLine += wstrInfo;
    //中英文对齐
    mstring strData = WtoA(wstrInfo);
    if (strData.size() < dwFormatLength)
    {
        for (DWORD dwIdex = strData.size() ; dwIdex < dwFormatLength ; dwIdex++)
        {
            m_wstrCurrentLine += L" ";
        }
    }
    return *this;
}

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo, DWORD dwFormatLength)
{
    return FormatDesc(wstrInfo, COLOUR_MSG, dwFormatLength);
}

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc)
{
    return FormatDesc(wstrInfo, vDesc, 0);
}

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo)
{
    return FormatDesc(wstrInfo, COLOUR_MSG, 0);
}

VOID CSyntaxDescHlpr::NextLine()
{
    m_vResult.m_vSyntaxDesc.push_back(m_vCurrentDesc);
    m_vResult.m_vShowInfo.push_back(m_wstrCurrentLine);

    m_vCurrentDesc.clear();
    m_wstrCurrentLine.clear();
}

VOID CSyntaxDescHlpr::Clear()
{
    m_vResult.Clear();
    m_vCurrentDesc.clear();
    m_wstrCurrentLine.clear();
}

CEasySyntaxHlpr::CEasySyntaxHlpr()
{
    m_iCurCol = 0;
}

CEasySyntaxHlpr::~CEasySyntaxHlpr()
{}

void CEasySyntaxHlpr::AppendWord(const ustring &wstrWord)
{
    SyntaxColourNode node(wstrWord);
    map<int, int>::iterator it;
    //一个汉字的长度相当于两个英文字符
    int iCurSize = WtoA(wstrWord).size();
    if (m_vColMax.end() == (it = m_vColMax.find(m_iCurCol)))
    {
        m_vColMax[m_iCurCol] = iCurSize;
    }
    else if(it->second < (int)wstrWord.size())
    {
        it->second = iCurSize;
    }
    m_vCurLineDesc.push_back(node);
}

void CEasySyntaxHlpr::NextLine()
{
    m_vDescCache.push_back(m_vCurLineDesc);
    m_vCurLineDesc.clear();
}

SyntaxDesc CEasySyntaxHlpr::GetResult()
{
    return SyntaxDesc();
}

void CEasySyntaxHlpr::Format()
{
    if (!m_vCurLineDesc.empty())
    {
        NextLine();
    }

    for (vector<vector<SyntaxColourNode>>::iterator itLine = m_vDescCache.begin() ; itLine != m_vDescCache.end() ; itLine++)
    {
        int iColumn = 0;
        int iPos = 0;
        for (vector<SyntaxColourNode>::iterator itNode = itLine->begin() ; itNode != itLine->end() ; itNode++, iColumn++)
        {
            int iMaxLength = m_vColMax[iColumn];
            itNode->m_dwStartPos = iPos;
            itNode->m_dwLength = itNode->m_wstrContent.size();
            int iCurSize = WtoA(itNode->m_wstrContent).size();
            iPos += (iMaxLength - iCurSize + 1);
        }
    }
}