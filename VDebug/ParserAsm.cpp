#include <Windows.h>
#include <map>
#include <set>
#include "ParserAsm.h"
#include "common.h"

using namespace std;
using namespace Json;

struct SyntaxAsmRuleNode
{
    wstring m_wstrMark;
    SyntaxColourDesc m_vHightLightDesc;
    CParserAsm::pfnParserAsmProc m_pfn;

    SyntaxAsmRuleNode(
        LPCWSTR wszMark = NULL,
        SyntaxColourDesc vDesc = SyntaxColourDesc(),
        CParserAsm::pfnParserAsmProc pfn = NULL
        )
    {
        if (wszMark)
        {
            m_wstrMark = wszMark;
        }
        m_pfn = pfn;
        m_vHightLightDesc = vDesc;
    }
};

static map<wstring, SyntaxAsmRuleNode> gs_vSyntaxAsmSet;
static set<wstring> gs_vRegister;

CParserAsm::CParserAsm()
{
    InitParserAsm();
}

CParserAsm::~CParserAsm()
{}

void CParserAsm::InitParserAsm()
{
    gs_vRegister.insert(L"eax");
    gs_vRegister.insert(L"ebx");
    gs_vRegister.insert(L"ecx");
    gs_vRegister.insert(L"edx");
    gs_vRegister.insert(L"ebp");
    gs_vRegister.insert(L"esp");

    RegisterParserProc(L"num", ParserNumber);
}

SyntaxColourDesc CParserAsm::ParserSingleWord(LPCWSTR wszWord) const
{
    map<wstring, SyntaxAsmRuleNode>::const_iterator it;
    for (it = gs_vSyntaxAsmSet.begin() ; it != gs_vSyntaxAsmSet.end() ; it++)
    {
        if (it->second.m_pfn(wszWord, (LPVOID)(&it->second)))
        {
            return it->second.m_vHightLightDesc;
        }
    }
    return m_vDefaultDesc;
}

vector<SyntaxColourNode> CParserAsm::ParserString(LPCWSTR wszBuffer) const
{
    vector<SyntaxColourNode> vResult;

    if (!wszBuffer || !wszBuffer[0])
    {
        return vResult;
    }

    wstring wstrWord;
    DWORD dwCurrentPos = 0;
    DWORD dwWordPos = 0;
    while (TRUE)
    {
        wstrWord = GetSingleWorld(wszBuffer, dwCurrentPos, dwWordPos);
        if (wstrWord.empty())
        {
            break;
        }

        SyntaxColourDesc vDesc = ParserSingleWord(wstrWord.c_str());
        vResult.push_back(SyntaxColourNode(wstrWord.c_str(), dwWordPos, vDesc));
        dwCurrentPos = (dwWordPos + (DWORD)wstrWord.size());
    }
    return vResult;
}

void CParserAsm::RegisterParserClass(LPCWSTR wszDescStr) const
{}

wstring CParserAsm::GetParserClass() const
{
    return L"asmParser";
}

void CParserAsm::RegisterParserProc(LPCWSTR wszRuleMark, pfnParserAsmProc pfnProc) const
{
    gs_vSyntaxAsmSet[wszRuleMark] = SyntaxAsmRuleNode(wszRuleMark, SyntaxColourDesc(), pfnProc);
}

void CParserAsm::RegisterParserDesc(LPCWSTR wszRuleMark, SyntaxColourDesc vDesc) const
{
    if (gs_vSyntaxAsmSet.end() == gs_vSyntaxAsmSet.find(wszRuleMark))
    {
        gs_vSyntaxAsmSet[wszRuleMark] = SyntaxAsmRuleNode(wszRuleMark, vDesc, ParserCommon);
    }
    else
    {
        gs_vSyntaxAsmSet[wszRuleMark].m_vHightLightDesc = vDesc;
    }
}

BOOL CParserAsm::ParserCommon(LPCWSTR wszWord, LPVOID pParam)
{
    SyntaxAsmRuleNode *ptr = (SyntaxAsmRuleNode *)pParam;
    if (ptr->m_wstrMark == wszWord)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CParserAsm::ParserNumber(LPCWSTR wszWord, LPVOID pParam)
{
    return IsNumberStr(wszWord);
}

SyntaxColourDesc CParserAsm::GetHightLightDesc(const Value &vJson)
{
    SyntaxColourDesc vDesc;
    vDesc.m_dwTextColour = GetColourFromStr(JsonGetStrValue(vJson, "textColour").c_str());
    vDesc.m_dwBackColour = GetColourFromStr(JsonGetStrValue(vJson, "backColour").c_str());
    vDesc.m_bBold = JsonGetIntValue(vJson, "bold");
    vDesc.m_bItalic = JsonGetIntValue(vJson, "italic");
    return vDesc;
}

BOOL CParserAsm::LoadCfg(const Value &vConfig)
{
    vector<string> vMembers = vConfig.getMemberNames();
    vector<string>::const_iterator it;
    for (it = vMembers.begin() ; it != vMembers.end() ; it++)
    {
        SyntaxColourDesc vDesc = GetHightLightDesc(vConfig[it->c_str()]);
        RegisterParserDesc(UtoW(*it).c_str(), vDesc);
    }
    return TRUE;
}