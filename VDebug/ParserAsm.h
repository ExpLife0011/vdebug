#ifndef PARSERASM_VDEBUG_H_H_
#define PARSERASM_VDEBUG_H_H_
#include <Windows.h>
#include "json/json.h"
#include "ParserBase.h"

class CParserAsm : public CParserBase
{
public:
    typedef BOOL (WINAPI *pfnParserAsmProc)(LPCWSTR wszWord, LPVOID pParam);

public:
    CParserAsm();

    virtual ~CParserAsm();

    void InitParserAsm();

    SyntaxColourDesc ParserSingleWord(LPCWSTR wszWord) const;

    virtual std::vector<SyntaxColourNode> ParserString(LPCWSTR wszBuffer) const;

    virtual void RegisterParserClass(LPCWSTR wszDescStr) const;

    virtual std::wstring GetParserClass() const;

    void RegisterParserProc(LPCWSTR wszRuleMark, pfnParserAsmProc pfnProc) const;

    void RegisterParserDesc(LPCWSTR wszRuleMark, SyntaxColourDesc vDesc) const;

    BOOL LoadCfg(const Json::Value &vConfig);

    SyntaxColourDesc GetHightLightDesc(const Json::Value &vJson);
protected:
    static BOOL WINAPI ParserNumber(LPCWSTR wszWord, LPVOID pParam);

    static BOOL WINAPI ParserCommon(LPCWSTR wszWord, LPVOID pParam);

protected:
    SyntaxColourDesc m_vDefaultDesc;
};
#endif