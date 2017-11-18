#ifndef PARSER_BASE_VDEBUG_H_H_
#define PARSER_BASE_VDEBUG_H_H_
#include <Windows.h>
#include <string>
#include <vector>
#include "json.h"
#include "../global.h"
#include "mstring.h"
#include "SyntaxCfg.h"

using namespace std;
using namespace Json;

struct SyntaxColourNode
{
    std::wstring m_wstrContent;
    DWORD m_dwStartPos;
    DWORD m_dwLength;

    SyntaxColourDesc m_vHightLightDesc;

    SyntaxColourNode(
        const ustring &wstrKeyWord,
        DWORD dwStartPos = 0,
        SyntaxColourDesc vDesc = SyntaxColourDesc()
        )
    {
        m_wstrContent = wstrKeyWord;
        m_dwStartPos = dwStartPos;
        m_dwLength = wstrKeyWord.size();
        m_vHightLightDesc = vDesc;
    }
};

class CParserBase
{
public:
    CParserBase()
    {}

    virtual ~CParserBase()
    {}

    virtual std::vector<SyntaxColourNode> ParserString(LPCWSTR wszBuffer) const = 0;

    virtual std::wstring GetParserClass() const = 0;

    virtual BOOL LoadCfg(const Json::Value &vConfig) = 0;

protected:
    static bool IsNumberStr(LPCWSTR wszBuffer);

    static bool IsWordCutChar(WCHAR cLetter);

    static std::wstring GetSingleWorld(LPCWSTR wszBuffer, DWORD dwCurrentPos, DWORD &dwWordPos);
};
#endif