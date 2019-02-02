#ifndef COMSTATIC_H_H_
#define COMSTATIC_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/mstring.h>
#include <ComStatic/StrUtil.h>
#include <ComStatic/GlobalDef.h>
#include <ComStatic/PrintFormater.h>

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

std::mstring __stdcall DosPathToNtPath(LPCSTR wszSrc);
std::mstring __stdcall GetProcPathByPid(IN DWORD dwPid);
std::mstring __stdcall GetFilePathFromHandle(HANDLE hFile);

std::mstring __stdcall GetStdErrorStr(DWORD dwErr = GetLastError());
std::wstring __stdcall RegGetStrValueExW(HKEY hKey, LPCWSTR wszSubKey, LPCWSTR wszValue);

struct ThreadInformation
{
    DWORD m_dwThreadId;
    void *m_dwStartAddr;
    DWORD m_dwSwitchCount;
    void *m_dwTebBase;
    FILETIME m_vCreateTime;
    LONG m_Priority; 
    ULONG m_eStat;
    ULONG m_eWaitReason;

    ThreadInformation()
    {
        ZeroMemory(this, sizeof(ThreadInformation));
    }
};

HANDLE WINAPI CreateLowsdEvent(BOOL bReset, BOOL bInitStat, LPCSTR szName);

BOOL WINAPI RunInSession(LPCSTR szImage, LPCSTR szCmd, DWORD dwSessionId, DWORD dwShell);
#endif