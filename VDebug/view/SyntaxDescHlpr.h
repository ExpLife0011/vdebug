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
        if (m_strCurrentLine.size())
        {
            NextLine();
        }
        return m_vResult;
    }

protected:
    vector<SyntaxColourNode> m_vCurrentDesc;
    mstring m_strCurrentLine;
    SyntaxDesc m_vResult;
    BOOL m_bValid;
};

//�Զ��������﷨����������
class CEasySyntaxHlpr
{
public:
    CEasySyntaxHlpr();
    virtual ~CEasySyntaxHlpr();
    void AppendWord(const ustring &wstrWord);
    void AppendWord(const ustring &wstrWord, SyntaxColourDesc &vDesc);
    void NextLine();
    SyntaxDesc GetResult();
    void Clear();

    //��ʽ�������
    SyntaxDesc Format();
protected:
    int m_iCurCol;
    map<int, int> m_vColMax;                        //ÿһ�����ֵ
    vector<SyntaxColourNode> m_vCurLineDesc;        //��ǰ�л���
    vector<vector<SyntaxColourNode>> m_vDescCache;  //��ǰ�����л���
};
#endif