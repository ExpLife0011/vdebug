#ifndef COMUTIL_COMLIB_H_H_
#define COMUTIL_COMLIB_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include <string>
#include <stdlib.h>

#pragma comment(lib, "shlwapi.lib")

const wchar_t *__stdcall FormatW(const wchar_t *format, ...);
const char *__stdcall FormatA(const char *fmt, ...);

const char *__stdcall AtoU(const char *);
const char *__stdcall UtoA(const char *);
const wchar_t *__stdcall AtoW(const char *);
const char *_stdcall WtoA(const wchar_t *);
const wchar_t *__stdcall UtoW(const char *);
const char *__stdcall WtoU(const wchar_t *);

static HMODULE _GetComLib() {
#if _WIN64 || WIN64
    #define COMLIB_NAME "ComLib64.dll"
#else
    #define COMLIB_NAME "ComLib32.dll"
#endif
    HMODULE m = GetModuleHandleA(COMLIB_NAME);
    if (!m)
    {
        char path[256];
        GetModuleFileNameA(NULL, path, 256);

        PathAppendA(path, ".." COMLIB_NAME);
        return LoadLibraryA(path);
    }
    return m;
}
#endif //COMUTIL_COMLIB_H_H_