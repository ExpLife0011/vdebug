#ifndef SYNTAXDESCHLPR_VDEBUG_H_H_
#define SYNTAXDESCHLPR_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include "mstring.h"
#include "ParserBase.h"
#include "SyntaxView.h"

using namespace std;

class CSyntaxDescHlpr
{
public:
    CSyntaxDescHlpr()
    {}

public:
    CSyntaxDescHlpr &FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc = COLOUR_MSG, DWORD dwFormatLength = 0);
    CSyntaxDescHlpr &FormatWithLength(const ustring &wstrInfo, DWORD dwFormatLength = 0);
    CSyntaxDescHlpr &FormatWithColour(const ustring &wstrInfo, const SyntaxColourDesc &vDesc = COLOUR_MSG);

    VOID NextLine();

    VOID AddEmptyLine()
    {
        NextLine();
        NextLine();
    }

    VOID Clear();

    SyntaxDesc GetResult()
    {
        if (m_wstrCurrentLine.size())
        {
            NextLine();
        }
        return m_vResult;
    }

protected:
    vector<SyntaxColourNode> m_vCurrentDesc;
    ustring m_wstrCurrentLine;
    SyntaxDesc m_vResult;
    BOOL m_bValid;
};

#endif