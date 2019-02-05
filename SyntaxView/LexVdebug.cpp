/**
LexVdebug Syntax Rule by lougd 2018-12-6
*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <list>
#include <map>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "LexVdebug.h"

using namespace std;

static const char *const pythonWordListDesc[] = {
    "Keywords",
    "Highlighted identifiers",
    0
};

struct LexRuleNode {
    mstring m_content;
    mstring m_label;
    int m_startPos;
    int m_endPos;
};

class LexVdebugParser {
private:
    LexVdebugParser() {}
    virtual ~LexVdebugParser(){}

public:
    static LexVdebugParser *GetInstance();
    void PushVdebugRule(const VdebugRuleParam *param);
    void ClearVdebugRule();

    void RegisterParser(const string &label, pfnColouriseTextProc pfn);

    static void ColouriseVdebugDoc(
        unsigned int startPos,
        int length,
        int initStyle,
        WordList *keywordlists[],
        Accessor &styler
        );

    static void FoldPyDoc(
        unsigned int startPos,
        int length,
        int /*initStyle - unused*/,
        WordList *[],
        Accessor &styler
        );
private:
    LexRuleNode *GetRuleFromPos(int pos);
    void OnParserStr(const mstring &lebal, int startPos, const string &data, int initStyle, StyleContext &sc);

private:
    list<LexRuleNode *> m_ruleSet;
    map<string, pfnColouriseTextProc> m_parser;
};

LexVdebugParser *LexVdebugParser::GetInstance() {
    static LexVdebugParser *s_ptr = NULL;

    if (s_ptr == NULL)
    {
        s_ptr = new LexVdebugParser();
    }
    return s_ptr;
}

void LexVdebugParser::PushVdebugRule(const VdebugRuleParam *param) {
    LexRuleNode *ptr = new LexRuleNode();
    ptr->m_content = param->content;
    ptr->m_label = param->label;
    ptr->m_startPos = param->startPos;
    ptr->m_endPos = param->endPos;
    m_ruleSet.push_back(ptr);
}

void LexVdebugParser::ClearVdebugRule() {
    for (list<LexRuleNode *>::const_iterator it = m_ruleSet.begin() ; it != m_ruleSet.end() ; it++)
    {
        delete (*it);
    }
    m_ruleSet.clear();
}

LexRuleNode *LexVdebugParser::GetRuleFromPos(int pos) {
    for (list<LexRuleNode *>::const_iterator it = m_ruleSet.begin() ; it != m_ruleSet.end() ; it++)
    {
        LexRuleNode *ptr = *it;
        if (pos >= ptr->m_startPos && pos < ptr->m_endPos)
        {
            return ptr;
        }
    }
    return NULL;
}

void LexVdebugParser::ColouriseVdebugDoc(
    unsigned int startPos,
    int length,
    int initStyle,
    WordList *keywordlists[],
    Accessor &styler
    )
{
    int curLine = styler.GetLine(startPos);
    StyleContext sc(startPos, length, initStyle, styler);

    int count2 = length;
    while (true) {
        LexRuleNode *pRuleNode = LexVdebugParser::GetInstance()->GetRuleFromPos(startPos);
        if (!pRuleNode)
        {
            break;
        }

        int ruleLength = pRuleNode->m_endPos - pRuleNode->m_startPos;
        if (count2 < ruleLength)
        {
            mstring sub = pRuleNode->m_content.substr(0, count2);
            GetInstance()->OnParserStr(pRuleNode->m_label, startPos, sub, sc.GetStat(), sc);
            break;
        }

        mstring sub2 = pRuleNode->m_content;
        GetInstance()->OnParserStr(pRuleNode->m_label, startPos, sub2, sc.GetStat(), sc);
        count2 -= ruleLength;
        startPos += ruleLength;

        if (count2 <= 0)
        {
            break;
        }
    }
    sc.Complete();
}

void LexVdebugParser::FoldPyDoc(
   unsigned int startPos,
   int length,
   int /*initStyle - unused*/,
   WordList *[],
   Accessor &styler
   )
{
    printf("abcdef");
}

void LexVdebugParser::OnParserStr(const mstring &lebal, int startPos, const string &data, int initStyle, StyleContext &sc) {
    dp(L"parser start:%d, length:%d", startPos, data.size());
    map<string, pfnColouriseTextProc>::const_iterator it = m_parser.find(lebal);

    /**
    int initStyle,
    unsigned int startPos,
    int length,
    StyleContextBase *sc
    */
    if (it != m_parser.end())
    {
        it->second(initStyle, startPos, data.c_str(), data.length(), &sc);
    }
}

void LexVdebugParser::RegisterParser(const string &label, pfnColouriseTextProc pfn) {
    m_parser[label] = pfn;
}

void ClearVdebugRule() {
    LexVdebugParser::GetInstance()->ClearVdebugRule();
}

void PushVdebugRule(VdebugRuleParam *ptr) {
    LexVdebugParser::GetInstance()->PushVdebugRule(ptr);
}

void __stdcall RegisterSyntaxProc(const char *label, pfnColouriseTextProc pfn) {
    LexVdebugParser::GetInstance()->RegisterParser(label, pfn);
}

LexerModule lmVdebug(SCLEX_VDEBUG, LexVdebugParser::ColouriseVdebugDoc, "vdebug", LexVdebugParser::FoldPyDoc, pythonWordListDesc);