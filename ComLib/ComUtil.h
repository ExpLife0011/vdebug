#ifndef COMUTIL_COMLIB_H_H_
#define COMUTIL_COMLIB_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include <stdlib.h>
#include <TlHelp32.h>
#include <ComStatic/ComStatic.h>
#include "cJSON.h"

#pragma comment(lib, "shlwapi.lib")

#define EXPAND_ARG(arg)                 (arg), RTL_NUMBER_OF((arg))

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
#define dp(f, ...) PrintDbgInternal(L"vdebug", __FILE__, __LINE__, f, ##__VA_ARGS__)

typedef BOOL (__stdcall* pfnProcHandlerW)(PPROCESSENTRY32W, void*);
void __stdcall IterateProcW(pfnProcHandlerW handler, void* lpParam);

BOOL  __stdcall IsPeFileW(LPCWSTR fileName, BOOL* b64);

BOOL __stdcall IsSameFileW(LPCWSTR file1, LPCWSTR file2);

BOOL __stdcall GetPeVersionW(LPCWSTR lpszFileName, LPWSTR outBuf, UINT size);

PVOID __stdcall DisableWow64Red();

BOOL __stdcall RevertWow64Red(PVOID oldValue);

typedef BOOL (__stdcall* pfnModuleHandlerW)(PMODULEENTRY32W, void*);
void __stdcall IterateModulesW(DWORD procId, pfnModuleHandlerW handler, void* lpParam);

void __stdcall ErrMessage(const wchar_t *format, ...);
std::ustring __stdcall GetProcessCommandLine(_In_ DWORD dwPid, BOOL bx64);

//获取pe文件属性
//Comments InternalName ProductName 
//CompanyName LegalCopyright ProductVersion 
//FileDescription LegalTrademarks PrivateBuild 
//FileVersion OriginalFilename SpecialBuild 
std::ustring __stdcall GetPeDescStr(const std::ustring &path, const std::ustring &attr);

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

class JsonAutoDelete {
public:
    inline JsonAutoDelete(cJSON *ptr) {
        mPtr = ptr;
    }

    inline virtual ~JsonAutoDelete() {
        if (mPtr)
        {
            cJSON_Delete(mPtr);
        }
    }

private:
    cJSON *mPtr;
};

class HandleAutoClose {
public:
    inline HandleAutoClose(HANDLE h) {
        mHandle = h;
    }

    inline virtual ~HandleAutoClose() {
        if (mHandle&& INVALID_HANDLE_VALUE != mHandle)
        {
            CloseHandle(mHandle);
        }
    }

private:
    HANDLE mHandle;
};

template <class T>
class MemoryAlloc {
public:
    MemoryAlloc() {
        mBuffer = NULL;
        mSize = 0;
    }

    virtual ~MemoryAlloc() {
        if (mBuffer)
        {
            delete []mBuffer;
        }
    }

    T *GetMemory(int size) {
        if (size < mSize)
        {
            return mBuffer;
        } else {
            if (mBuffer)
            {
                delete []mBuffer;
            }
            mSize = size;
            mBuffer = new T[size];
        }
        return mBuffer;
    }

    T *GetPtr() {
        return mBuffer;
    }

    int GetSize() {
        return mSize;
    }

private:
    T *mBuffer;
    int mSize;
};

std::mstring __stdcall GetStrFormJson(cJSON *json, const std::mstring &name);
int __stdcall GetIntFromJson(cJSON *json, const std::mstring &name);

std::ustring __stdcall GetWindowStrW(HWND hwnd);

static std::mstring __stdcall GetCurTimeStr1(const char *fmt) {
    SYSTEMTIME time;
    GetLocalTime(&time);
    return FormatA(
        fmt,
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds
        );
}
#endif //COMUTIL_COMLIB_H_H_