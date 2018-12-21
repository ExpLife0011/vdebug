#ifndef COMMON_VDEBUG_H_H_
#define COMMON_VDEBUG_H_H_
#include <Windows.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <list>
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
ustring GetStdErrorStr(DWORD dwErr = GetLastError());

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

enum ThreadStat
{
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition,
    StateUnknown
};

enum ThreadWaitReason
{
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
};

struct ThreadInformation
{
    DWORD m_dwThreadId;
    DWORD64 m_dwStartAddr;
    DWORD m_dwSwitchCount;
    DWORD64 m_dwTebBase;
    FILETIME m_vCreateTime;
    LONG m_Priority; 
    ThreadStat m_eStat;
    ThreadWaitReason m_eWaitReason;

    ThreadInformation()
    {
        ZeroMemory(this, sizeof(ThreadInformation));
    }
};

BOOL GetThreadInformation(DWORD dwProcressId, list<ThreadInformation> &vThreads);
#endif