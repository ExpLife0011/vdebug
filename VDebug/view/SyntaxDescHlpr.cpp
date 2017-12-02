#include "SyntaxDescHlpr.h"
#include "common.h"

CSyntaxDescHlpr &CSyntaxDescHlpr::FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc, DWORD dwFormatLength)
{
    m_vCurrentDesc.push_back(SyntaxColourNode(wstrInfo, m_wstrCurrentLine.size(), vDesc));
    m_wstrCurrentLine += wstrInfo;
    //中英文对其
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