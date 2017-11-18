#include <Windows.h>
#include <set>
#include "ParserBase.h"
#include "mstring.h"

using namespace std;

bool CParserBase::IsNumberStr(LPCWSTR wszBuffer)
{
    DWORD dwIdex = 0;
    WCHAR cLetter = 0;

    ustring wstr(wszBuffer);
    if (0 == wstr.comparei(L"0x"))
    {
        dwIdex = 2;
    }

    while (TRUE)
    {
        cLetter = wszBuffer[dwIdex];

        if (!cLetter)
        {
            break;
        }

        if (cLetter >= 'A' && cLetter <= 'F')
        {
            cLetter |= 32;
        }

        if (!((cLetter >= 'a' && cLetter <= 'f') || (cLetter >= '0' && cLetter <= '9') || cLetter == 'h'))
        {
            return FALSE;
        }
        dwIdex++;
    }
    return (0 != dwIdex);
}

bool CParserBase::IsWordCutChar(WCHAR cLetter)
{
    static set<WCHAR> s_vWordCutChars;
    if (s_vWordCutChars.empty())
    {
        s_vWordCutChars.insert(L' ');
        s_vWordCutChars.insert(L'[');
        s_vWordCutChars.insert(L']');
        s_vWordCutChars.insert(L'+');
        s_vWordCutChars.insert(L'-');
        s_vWordCutChars.insert(L'(');
        s_vWordCutChars.insert(L')');
        s_vWordCutChars.insert(L',');
        s_vWordCutChars.insert(L'!');
        s_vWordCutChars.insert(L'`');
    }

    return (s_vWordCutChars.end() != s_vWordCutChars.find(cLetter));
}

std::wstring CParserBase::GetSingleWorld(LPCWSTR wszBuffer, DWORD dwCurrentPos, DWORD &dwWordPos)
{
    DWORD dwIdex = dwCurrentPos;
    WCHAR cLetter = 0;

    while(TRUE)
    {
        cLetter = wszBuffer[dwIdex];
        if (!IsWordCutChar(cLetter) || (0 == cLetter))
        {
            break;
        }
        dwIdex++;
    }

    DWORD dwStart = dwIdex;
    while(TRUE)
    {
        cLetter = wszBuffer[dwIdex];
        if (IsWordCutChar(cLetter) || (0 == cLetter))
        {
            break;
        }
        dwIdex++;
    }

    if (dwIdex > dwStart)
    {
        dwWordPos = dwStart;
        return wstring(wszBuffer + dwStart, dwIdex - dwStart);
    }
    return L"";
}