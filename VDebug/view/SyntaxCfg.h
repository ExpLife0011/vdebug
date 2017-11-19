#ifndef SYNTAXCFG_H_H_
#define SYNTAXCFG_H_H_
#include <Windows.h>
#include "../global.h"

/*语法高亮配置 开始*/
#define COLOUR_ADDR             GetSyntaxCfg(L"addr")
#define COLOUR_REGISTER         GetSyntaxCfg(L"register")
#define COLOUR_ERROR            GetSyntaxCfg(L"error")
#define COLOUR_MSG              GetSyntaxCfg(L"msg")
/*语法高亮配置 结束*/

struct SyntaxColourDesc
{
    DWORD m_dwTextColour;
    DWORD m_dwBackColour;

    BOOL m_bBold;
    BOOL m_bItalic;

    SyntaxColourDesc(
        DWORD dwTextColour = NULL_COLOUR,
        DWORD dwBackColour = NULL_COLOUR,
        BOOL bBold = FALSE,
        BOOL bItalic = FALSE
        )
    {
        m_dwTextColour = dwTextColour;
        m_dwBackColour = dwBackColour;
        m_bBold = bBold;
        m_bItalic = bItalic;
    }

    bool IsValid()
    {
        return (m_dwBackColour || m_dwTextColour);
    }
};

BOOL LoadSyntaxCfg(const Value &vSyntaxCfg);

SyntaxColourDesc GetSyntaxCfg(LPCWSTR wszName);
#endif