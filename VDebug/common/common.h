#ifndef COMMON_VDEBUG_H_H_
#define COMMON_VDEBUG_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include "mstring.h"
#include "json.h"

#pragma comment(lib, "shlwapi.lib")

using namespace std;
using namespace Json;

#define EXPAND_ARG(arg)                 (arg), RTL_NUMBER_OF((arg))

VOID WINAPI CentreWindow(HWND hSrcWnd, HWND hDstWnd = NULL);

//Json Helpr
static int JsonGetIntValue(const Value &vJson, LPCSTR szName)
{
    if (!szName || !szName[0] || intValue!= (vJson[szName].type()))
    {
        return 0;
    }

    return vJson[szName].asInt();
}

static std::string JsonGetStrValue(const Value &vJson, LPCSTR szName)
{
    if (!szName || !szName[0] || stringValue != (vJson[szName].type()))
    {
        return 0;
    }

    return vJson[szName].asString();
}

static DWORD GetColourFromStr(LPCSTR szColour)
{
    if (!szColour || !szColour[0])
    {
        return 0;
    }

    mstring str(szColour);
    if (str == "null")
    {
        return 0xffffffff;
    }
    str.delchar(' ');
    str += ",";
    int vCol[3] = {0};
    int iIdex = 0;
    size_t iLast = 0;
    size_t iCur = 0;
    while (mstring::npos != (iCur = str.find(",", iLast)))
    {
        mstring strSub = str.substr(iLast, iCur - iLast);
        vCol[iIdex++] = atoi(strSub.c_str());
        iLast = (iCur + 1);

        if (3 == iIdex)
        {
            break;
        }
    }

    return RGB(vCol[0], vCol[1], vCol[2]);
}

std::wstring FormatW(const wchar_t *format, ...);

//char conver
_STD_BEGIN
//strutf8类型定义，同string同类型
typedef string strutf8;
//tstring类型定义
#ifdef _UNICODE
typedef wstring tstring;
#else
typedef string tstring;
#endif
_STD_END

strutf8 ToUtf8A(const string &str);
#define AtoU ToUtf8A

strutf8 ToUtf8W(const wstring &str);
#define WtoU ToUtf8W

string ToCommonA(const strutf8 &str);
#define UtoA ToCommonA

wstring ToCommonW(const strutf8 &str);
#define UtoW ToCommonW

string ToMultiByte(const wstring &str);
#define WtoA ToMultiByte

wstring ToWideChar(const string &str);
#define AtoW ToWideCharh

VOID PrintDbgInternal(LPCWSTR wszTarget, LPCSTR wszFile, DWORD dwLine, LPCWSTR wszFormat, ...);
#define dp(f, ...) PrintDbgInternal(L"[gdprotect]", __FILE__, __LINE__, f, ##__VA_ARGS__)

typedef BOOL (WINAPI* pfnProcHandlerW)(PPROCESSENTRY32W, void*);
void IterateProcW(pfnProcHandlerW handler, void* lpParam);

BOOL IsPeFileW(LPCWSTR fileName, BOOL* b64);

BOOL GetPeVersionW(LPCWSTR lpszFileName, LPWSTR outBuf, UINT size);

typedef struct _FILE_MAPPING_STRUCT
{
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpView;
    LARGE_INTEGER fileSize;
    DWORD mappedSize;
} FILE_MAPPING_STRUCT, *PFILE_MAPPING_STRUCT;

PFILE_MAPPING_STRUCT MappingFileW(LPCWSTR fileName, BOOL bWrite, DWORD maxViewSize);

void CloseFileMapping(PFILE_MAPPING_STRUCT pfms);

ustring GetProcPathByPid(IN DWORD dwPid);

PVOID DisableWow64Red();

BOOL RevertWow64Red(PVOID oldValue);

ustring GetFilePathFromHandle(HANDLE hFile);

typedef BOOL (WINAPI* pfnModuleHandlerW)(PMODULEENTRY32W, void*);
void IterateModulesW(DWORD procId, pfnModuleHandlerW handler, void* lpParam);

void ErrMessage(const wchar_t *format, ...);
ustring GetStdErrorStr();

ustring GetProcessCommandLine(_In_ DWORD dwPid, BOOL bx64);
void test();

typedef struct _GDS_LINKINFO
{
    WCHAR wszPath[MAX_PATH];
    WCHAR wszArgs[MAX_PATH * 2];
    WCHAR wszIcon[MAX_PATH];
    int nIconIdx;
    WCHAR wszWorkDir[MAX_PATH];
    WCHAR wszDesc[MAX_PATH];
} GDS_LINKINFO, *PGDS_LINKINFO;

BOOL ShlParseShortcutsW(LPCWSTR wszLnkFile, PGDS_LINKINFO info);
#endif