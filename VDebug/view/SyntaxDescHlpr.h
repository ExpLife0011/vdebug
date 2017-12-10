#ifndef SYNTAXDESCHLPR_VDEBUG_H_H_
#define SYNTAXDESCHLPR_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include <map>
#include "mstring.h"
#include "SyntaxView.h"

using namespace std;

class CSyntaxDescHlpr
{
public:
    CSyntaxDescHlpr()
    {}

public:
    CSyntaxDescHlpr &FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc, DWORD dwFormatLength);
    CSyntaxDescHlpr &FormatDesc(const ustring &wstrInfo, DWORD dwFormatLength);
    CSyntaxDescHlpr &FormatDesc(const ustring &wstrInfo, const SyntaxColourDesc &vDesc);
    CSyntaxDescHlpr &FormatDesc(const ustring &wstrInfo);

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

//自动调整的语法高亮生成类
class CEasySyntaxHlpr
{
public:
    CEasySyntaxHlpr();
    virtual ~CEasySyntaxHlpr();
    void AppendWord(const ustring &wstrWord);
    void AppendWord(const ustring &wstrWord, SyntaxColourDesc &vDesc);
    void NextLine();

    SyntaxDesc GetResult();
protected:
    void Format();

protected:
    int m_iCurCol;
    map<int, int> m_vColMax;                        //每一列最大值
    vector<SyntaxColourNode> m_vCurLineDesc;
    vector<vector<SyntaxColourNode>> m_vDescCache;
};
#endif