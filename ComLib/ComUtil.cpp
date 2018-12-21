#include "ComUtil.h"
#include "SyntaxFormat.h"
#include <string>
#include "StrSafe.h"

#pragma comment(lib, "shlwapi.lib")

using namespace std;

typedef string strutf8;
strutf8 ToUtf8W(const wstring &str);
wstring ToWideChar(const string &str);
string ToMultiByte(const wstring &str);
wstring ToCommonW(const strutf8 &str);

strutf8 ToUtf8A(const string &str)
{
    return ToUtf8W(ToWideChar(str));
}

strutf8 ToUtf8W(const wstring &str)
{
    strutf8 ret;

    int count = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

    if (count > 0)
    {
        char *buffer = new char[count];

        if (buffer != 0)
        {
            WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, buffer, count, NULL, NULL);
            ret = buffer;

            delete []buffer;
        }
    }

    return ret;
}

string ToCommonA(const strutf8 &str)
{
    return ToMultiByte(ToCommonW(str));
}

wstring ToCommonW(const strutf8 &str)
{
    wstring ret;

    int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    if (count > 0)
    {
        wchar_t *buffer = new wchar_t[count];

        if (buffer != 0)
        {
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, count);
            ret = buffer;

            delete []buffer;
        }
    }

    return ret;
}

string ToMultiByte(const wstring &str)
{
    string ret;

    int count = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

    if (count > 0)
    {
        char *buffer = new char[count];

        if (buffer != 0)
        {
            WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, buffer, count, NULL, NULL);
            ret = buffer;

            delete []buffer;
        }
    }

    return ret;
}

wstring ToWideChar(const string &str)
{
    wstring ret;

    int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    if (count > 0)
    {
        wchar_t *buffer = new wchar_t[count];

        if (buffer != 0)
        {
            MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, count);
            ret = buffer;

            delete []buffer;
        }
    }

    return ret;
}

const char *__stdcall AtoU(const char *str) {
    return SafeStrCopyA(ToUtf8A(str).c_str());
}

const char *__stdcall UtoA(const char *str) {
    return SafeStrCopyA(ToCommonA(str).c_str());
}

const wchar_t *__stdcall AtoW(const char *str) {
    return SafeStrCopyW(ToWideChar(str).c_str());
}

const char * __stdcall WtoA(const wchar_t *str) {
    return SafeStrCopyA(ToMultiByte(str).c_str());
}

const wchar_t *__stdcall UtoW(const char *str) {
    return SafeStrCopyW(ToCommonW(str).c_str());
}

const char *__stdcall WtoU(const wchar_t *str) {
    return SafeStrCopyA(ToUtf8W(str).c_str());
}

const char *__stdcall FormatA(const char *format, ...) {
    char szText[2048] = {0};
    va_list val;

    va_start(val, format);
    wvnsprintfA(szText, RTL_NUMBER_OF(szText), format, val);
    va_end(val);
    return SafeStrCopyA(szText);
}

const wchar_t *__stdcall FormatW(const wchar_t *format, ...)
{
    wchar_t szText[2048] = {0};
    va_list val;

    va_start(val, format);
    wvnsprintfW(szText, RTL_NUMBER_OF(szText), format, val);
    va_end(val);

    return SafeStrCopyW(szText);
}