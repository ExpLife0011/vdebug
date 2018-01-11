#include "SyntaxDescHlpr.h"
#include "common.h"

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc, DWORD dwFormatLength)
{
    mstring strInfo(wstrInfo);
    m_vCurrentDesc.push_back(SyntaxColourNode(wstrInfo, (DWORD)m_strCurrentLine.size(), vDesc));
    m_strCurrentLine += strInfo;
    //ÖÐÓ¢ÎÄ¶ÔÆë
    if (strInfo.size() < dwFormatLength)
    {
        for (DWORD dwIdex = (DWORD)strInfo.size() ; dwIdex < dwFormatLength ; dwIdex++)
        {
            m_strCurrentLine += " ";
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
    m_vResult.m_vShowInfo.push_back(m_strCurrentLine);

    m_vCurrentDesc.clear();
    m_strCurrentLine.clear();
}

VOID CSyntaxDescHlpr::Clear()
{
    m_vResult.Clear();
    m_vCurrentDesc.clear();
    m_strCurrentLine.clear();
}

CEasySyntaxHlpr::CEasySyntaxHlpr()
{
    m_iCurCol = 0;
}

CEasySyntaxHlpr::~CEasySyntaxHlpr()
{}

void CEasySyntaxHlpr::AppendWord(const ustring &wstrWord, SyntaxColourDesc &vDesc)
{
    mstring str(wstrWord);
    SyntaxColourNode node(str, 0, vDesc);
    map<int, int>::iterator it;
    int iCurSize = (int)str.size();
    if (m_vColMax.end() == (it = m_vColMax.find(m_iCurCol)))
    {
        m_vColMax[m_iCurCol] = iCurSize;
    }
    else if(it->second < (int)wstrWord.size())
    {
        it->second = iCurSize;
    }
    m_iCurCol++;
    m_vCurLineDesc.push_back(node);
}

void CEasySyntaxHlpr::AppendWord(const ustring &wstrWord)
{
    AppendWord(wstrWord, COLOUR_MSG);
}

void CEasySyntaxHlpr::NextLine()
{
    m_vDescCache.push_back(m_vCurLineDesc);
    m_vCurLineDesc.clear();
    m_iCurCol = 0;
}

SyntaxDesc CEasySyntaxHlpr::GetResult()
{
    return Format();
}

void CEasySyntaxHlpr::Clear()
{
    m_vColMax.clear();
    m_vCurLineDesc.clear();
    m_vDescCache.clear();
}

SyntaxDesc CEasySyntaxHlpr::Format()
{
    if (!m_vCurLineDesc.empty())
    {
        NextLine();
    }

    SyntaxDesc vResult;
    for (vector<vector<SyntaxColourNode>>::iterator itLine = m_vDescCache.begin() ; itLine != m_vDescCache.end() ; itLine++)
    {
        int iColumn = 0;
        int iPos = 0;
        mstring strLine;
        for (vector<SyntaxColourNode>::iterator itNode = itLine->begin() ; itNode != itLine->end() ; itNode++, iColumn++)
        {
            int iMaxLength = m_vColMax[iColumn];
            itNode->m_dwStartPos = iPos;
            itNode->m_dwLength = (DWORD)itNode->m_strContent.size();
            iPos += (iMaxLength + 1);

            int iStrSize = (int)itNode->m_strContent.size();
            strLine += itNode->m_strContent;
            if (iStrSize < (iMaxLength + 1))
            {
                for (int i = 0 ; i < (iMaxLength + 1 - iStrSize) ; i++)
                {
                    strLine += " ";
                }
            }
        }
        vResult.m_vShowInfo.push_back(strLine);
    }
    vResult.m_vSyntaxDesc = m_vDescCache;
    return vResult;
}