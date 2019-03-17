#ifndef STRUTIL_COMSTATIC_H_H_
#define STRUTIL_COMSTATIC_H_H_
#include <list>
#include <string>
#include "mstring.h"

std::wstring COMAPI __stdcall FormatW(const wchar_t *format, ...);
std::string COMAPI __stdcall FormatA(const char *fmt, ...);

std::string COMAPI __stdcall AtoU(const std::string &);
std::string COMAPI __stdcall UtoA(const std::string &);
std::wstring COMAPI __stdcall AtoW(const std::string &);
std::string COMAPI _stdcall WtoA(const std::wstring &);
std::wstring COMAPI __stdcall UtoW(const std::string &);
std::string COMAPI __stdcall WtoU(const std::wstring &);

std::list<std::mstring> COMAPI SplitStrA(const std::mstring &str, const std::mstring &split);
std::list<std::ustring> COMAPI SplitStrW(const std::ustring &str, const std::ustring &split);
#endif //STRUTIL_COMSTATIC_H_H_