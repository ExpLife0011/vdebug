#ifndef COMUTIL_COMLIB_H_H_
#define COMUTIL_COMLIB_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include <string>
#include <stdlib.h>
#include <TlHelp32.h>

#pragma comment(lib, "shlwapi.lib")

#define EXPAND_ARG(arg)                 (arg), RTL_NUMBER_OF((arg))

const wchar_t *__stdcall FormatW(const wchar_t *format, ...);
const char *__stdcall FormatA(const char *fmt, ...);

const char *__stdcall AtoU(const char *);
const char *__stdcall UtoA(const char *);
const wchar_t *__stdcall AtoW(const char *);
const char *_stdcall WtoA(const wchar_t *);
const wchar_t *__stdcall UtoW(const char *);
const char *__stdcall WtoU(const wchar_t *);

typedef struct _FILE_MAPPING_STRUCT
{
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpView;
    LARGE_INTEGER fileSize;
    DWORD mappedSize;
} FILE_MAPPING_STRUCT, *PFILE_MAPPING_STRUCT;

PFILE_MAPPING_STRUCT __stdcall MappingFileW(LPCWSTR fileName, BOOL bWrite = FALSE, DWORD maxViewSize = 1024 * 1024 * 64);
void __stdcall CloseFileMapping(PFILE_MAPPING_STRUCT pfms);

VOID __stdcall CentreWindow(HWND hSrcWnd, HWND hDstWnd = NULL);
DWORD __stdcall GetColourFromStr(LPCSTR szColour);

VOID __stdcall PrintDbgInternal(LPCWSTR wszTarget, LPCSTR wszFile, DWORD dwLine, LPCWSTR wszFormat, ...);
#define dp(f, ...) PrintDbgInternal(L"[gdprotect]", __FILE__, __LINE__, f, ##__VA_ARGS__)

typedef BOOL (__stdcall* pfnProcHandlerW)(PPROCESSENTRY32W, void*);
void __stdcall IterateProcW(pfnProcHandlerW handler, void* lpParam);

BOOL  __stdcall IsPeFileW(LPCWSTR fileName, BOOL* b64);

BOOL __stdcall GetPeVersionW(LPCWSTR lpszFileName, LPWSTR outBuf, UINT size);

PVOID __stdcall DisableWow64Red();

BOOL __stdcall RevertWow64Red(PVOID oldValue);

typedef BOOL (__stdcall* pfnModuleHandlerW)(PMODULEENTRY32W, void*);
void __stdcall IterateModulesW(DWORD procId, pfnModuleHandlerW handler, void* lpParam);

void __stdcall ErrMessage(const wchar_t *format, ...);
const wchar_t *__stdcall GetProcessCommandLine(_In_ DWORD dwPid, BOOL bx64);

typedef struct _GDS_LINKINFO
{
    WCHAR wszPath[MAX_PATH];
    WCHAR wszArgs[MAX_PATH * 2];
    WCHAR wszIcon[MAX_PATH];
    int nIconIdx;
    WCHAR wszWorkDir[MAX_PATH];
    WCHAR wszDesc[MAX_PATH];
} GDS_LINKINFO, *PGDS_LINKINFO;

BOOL __stdcall ShlParseShortcutsW(LPCWSTR wszLnkFile, PGDS_LINKINFO info);

DWORD __stdcall RegGetDWORDFromRegA(HKEY hKey, LPCSTR subKey, LPCSTR value, DWORD defaultVal);
DWORD __stdcall RegGetDWORDFromRegW(HKEY hKey, LPCWSTR subKey, LPCWSTR value, DWORD defaultVal);
BOOL __stdcall RegSetDWORDValueA(HKEY hKey, LPCSTR szSubKey, LPCSTR szValue, DWORD dwData);
BOOL __stdcall RegSetDWORDValueW(HKEY hKey, LPCWSTR wszSubKey, LPCWSTR wszValue, DWORD dwData);
BOOL __stdcall RegSetStrValueW(HKEY hKey, LPCWSTR wszSubKey, LPCWSTR wszValue, LPCWSTR wszData);

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