#ifndef COMSTATIC_H_H_
#define COMSTATIC_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/mstring.h>
#include <ComStatic/StrUtil.h>
#include <ComStatic/servhlpr.h>

#ifndef COMLIB_EXPORTS
    #if _WIN64 || WIN64
        #ifdef _DEBUG
        #pragma comment(lib, "../Debug/x64/ComStatic64.lib")
        #else
        #pragma comment(lib, "../Release/x64/ComStatic64.lib")
        #endif //_DEBUG
    #else
        #ifdef _DEBUG
        #pragma comment(lib, "../Debug/x32/ComStatic32.lib")
        #else
        #pragma comment(lib, "../Release/x32/ComStatic32.lib")
        #endif //_DEBUG
    #endif //_WIN64
#endif //COMLIB_EXPORTS

std::ustring __stdcall DosPathToNtPath(LPCWSTR wszSrc);
std::ustring __stdcall GetProcPathByPid(IN DWORD dwPid);
std::ustring __stdcall GetFilePathFromHandle(HANDLE hFile);

std::ustring __stdcall GetStdErrorStr(DWORD dwErr = GetLastError());
std::wstring __stdcall RegGetStrValueExW(HKEY hKey, LPCWSTR wszSubKey, LPCWSTR wszValue);

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

BOOL __stdcall GetThreadInformation(DWORD dwProcressId, std::list<ThreadInformation> &vThreads);

HANDLE WINAPI CreateLowsdEvent(BOOL bReset, BOOL bInitStat, LPCWSTR wszName);

BOOL WINAPI RunInSession(LPCWSTR wszImage, LPCWSTR wszCmd, DWORD dwSessionId, DWORD dwShell);
#endif