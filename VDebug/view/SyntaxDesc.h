#ifndef SYNTAXDESC_VDEBUG_H_H_
#define SYNTAXDESC_VDEBUG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <vector>
#include <SyntaxHlpr/SyntaxCfg.h>

using namespace std;

struct SyntaxColourNode
{
    std::mstring m_strContent;
    DWORD m_dwStartPos;
    DWORD m_dwLength;

    SyntaxColourDesc m_vHightLightDesc;

    SyntaxColourNode(
        const mstring &strKeyWord,
        DWORD dwStartPos = 0,
        SyntaxColourDesc vDesc = SyntaxColourDesc()
        )
    {
        m_strContent = strKeyWord;
        m_dwStartPos = dwStartPos;
        m_dwLength = (DWORD)m_strContent.size();
        m_vHightLightDesc = vDesc;
    }
};

struct SyntaxDesc
{
    SyntaxDesc(const ustring &wstrContent, const SyntaxColourDesc &colour);
    SyntaxDesc(const ustring &wstrContent);
    SyntaxDesc();

    SyntaxDesc &operator+=(const SyntaxDesc &desc)
    {
        m_vSyntaxDesc.insert(m_vSyntaxDesc.end(), desc.m_vSyntaxDesc.begin(), desc.m_vSyntaxDesc.end());
        m_vShowInfo.insert(m_vShowInfo.end(), desc.m_vShowInfo.begin(), desc.m_vShowInfo.end());
        return *this;
    }

    SyntaxDesc &operator+=(const ustring &wstr)
    {
        *this += SyntaxDesc(wstr);
        return *this;
    }

    bool operator=(const SyntaxDesc &desc)
    {
        m_vSyntaxDesc = desc.m_vSyntaxDesc;
        m_vShowInfo = desc.m_vShowInfo;
        return true;
    }

    BOOL IsValid() const
    {
        return TRUE;
    }

    VOID Clear()
    {
        m_vSyntaxDesc.clear();
        m_vShowInfo.clear();
    }

    vector<vector<SyntaxColourNode>> m_vSyntaxDesc;
    vector<mstring> m_vShowInfo;
};
#endif