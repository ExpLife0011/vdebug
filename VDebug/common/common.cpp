#include <Windows.h>
#include <Psapi.h>
#include <Winternl.h>
#include <shobjidl.h>
#include "common.h"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Version.lib")

VOID WINAPI CentreWindow(HWND hSrcWnd, HWND hDstWnd)
{
    if (!hDstWnd)
    {
        hDstWnd = GetDesktopWindow();
    }

    RECT rt = {0};
    GetWindowRect(hDstWnd, &rt);
    RECT crt = {0};
    GetWindowRect(hSrcWnd, &crt);
    int iX = 0;
    int iY = 0;
    int icW = crt.right - crt.left;
    int iW = rt.right - rt.left;
    int icH = crt.bottom - crt.top;
    int iH = rt.bottom - rt.top;
    iX = rt.left + (iW - icW) / 2;
    iY = rt.top + (iH - icH) / 2;
    MoveWindow(hSrcWnd, iX, iY, icW, icH, TRUE);
}

std::wstring FormatW(const wchar_t *format, ...)
{
    wchar_t szText[2048];
    va_list val;

    va_start(val, format);
    wvnsprintfW(szText, RTL_NUMBER_OF(szText), format, val);
    va_end(val);

    return szText;
}

void ErrMessage(const wchar_t *format, ...)
{
    wchar_t szText[2048] = {0};
    va_list val;

    va_start(val, format);
    wvnsprintfW(szText, RTL_NUMBER_OF(szText), format, val);
    va_end(val);
    MessageBoxW(0, szText, L"Error", MB_TOPMOST);
}

ustring GetStdErrorStr(DWORD dwErr)
{
    LPVOID lpMsgBuf = NULL;
    FormatMessageW(  
        FORMAT_MESSAGE_ALLOCATE_BUFFER |  
        FORMAT_MESSAGE_FROM_SYSTEM |  
        FORMAT_MESSAGE_IGNORE_INSERTS, 
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), //Default language  
        (LPWSTR)&lpMsgBuf,  
        0,  
        NULL  
        ); 
    ustring wstrMsg((LPWSTR)lpMsgBuf);
    if (lpMsgBuf)
    {
        LocalFlags(lpMsgBuf);
    }
    return wstrMsg;
}

VOID PrintDbgInternal(LPCWSTR wszTarget, LPCSTR szFile, DWORD dwLine, LPCWSTR wszFormat, ...)
{
    WCHAR wszFormat1[1024] = {0};
    WCHAR wszFormat2[1024] = {0};
    lstrcpyW(wszFormat1, L"[VDebug][%hs.%d]%ls");
    StrCatW(wszFormat1, L"\n");
    wnsprintfW(wszFormat2, RTL_NUMBER_OF(wszFormat2), wszFormat1, szFile, dwLine, wszFormat);

    WCHAR wszLogInfo[1024];
    va_list vList;
    va_start(vList, wszFormat);
    wvnsprintfW(wszLogInfo, sizeof(wszLogInfo), wszFormat2, vList);
    va_end(vList);
    OutputDebugStringW(wszLogInfo);
}

void IterateProcW(pfnProcHandlerW handler, void* lpParam)
{
    do
    {
        if (!handler)
        {
            break;
        }

        PROCESSENTRY32W pe32 = {0};
        pe32.dwSize = sizeof(pe32);

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnap)
        {
            break;
        }

        BOOL bMore = Process32FirstW(hSnap, &pe32);
        while (bMore)
        {
            if (!handler(&pe32, lpParam))
            {
                // 回调方要求退出
                break;
            }

            bMore = Process32NextW(hSnap, &pe32);
        }

        CloseHandle(hSnap);
    } while (FALSE);
}

static BOOL WINAPI _PeIsPeFile(LPVOID buffer, BOOL* b64)
{
    BOOL bRet = FALSE;

    do
    {
        if (!buffer)
        {
            break;
        }

        IMAGE_DOS_HEADER* pDosHdr = (IMAGE_DOS_HEADER*)buffer;
        if (IMAGE_DOS_SIGNATURE != pDosHdr->e_magic)
        {
            break;
        }
        if (pDosHdr->e_lfanew > 1024)
        {
            break;
        }

        IMAGE_NT_HEADERS32* pNtHdr = (IMAGE_NT_HEADERS32*)((byte*)pDosHdr + pDosHdr->e_lfanew);
        if (IsBadReadPtr(pNtHdr, sizeof(void*)))
        {
            break;
        }

        if (IMAGE_NT_SIGNATURE != pNtHdr->Signature)
        {
            break;
        }

        if (b64)
        {
            *b64 = (pNtHdr->FileHeader.SizeOfOptionalHeader == sizeof(IMAGE_OPTIONAL_HEADER64));
        }

        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

#define _PE_MAX_MAP_SIZE    (1024 * 1024 * 4)

static BOOL WINAPI _PeCheckPeMapping(PFILE_MAPPING_STRUCT pfms, BOOL* b64)
{
    if (!pfms || !pfms->lpView || !pfms->fileSize.QuadPart)
    {
        return FALSE;
    }

    if (pfms->fileSize.QuadPart < (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32)))
    {
        return FALSE;
    }

    if (!_PeIsPeFile(pfms->lpView, b64))
    {
        return FALSE;
    }

    return TRUE;
}

PFILE_MAPPING_STRUCT MappingFileW(LPCWSTR fileName, BOOL bWrite, DWORD maxViewSize)
{
    PFILE_MAPPING_STRUCT pfms = NULL;

    do
    {
        if (!fileName)
        {
            break;
        }

        pfms = (PFILE_MAPPING_STRUCT)malloc(sizeof(FILE_MAPPING_STRUCT));
        if (!pfms)
        {
            break;
        }
        RtlZeroMemory(pfms, sizeof(FILE_MAPPING_STRUCT));

        pfms->hFile = CreateFileW(
            fileName,
            GENERIC_READ | (bWrite ? GENERIC_WRITE : 0),
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (INVALID_HANDLE_VALUE == pfms->hFile)
        {
            break;
        }

        if (!GetFileSizeEx(pfms->hFile, &(pfms->fileSize)) || !pfms->fileSize.QuadPart)
        {
            break;
        }

        if (!maxViewSize)
        {
            pfms->mappedSize = pfms->fileSize.QuadPart > 0xffffffff ? 0xffffffff : pfms->fileSize.LowPart;
        }
        else
        {
            pfms->mappedSize = pfms->fileSize.QuadPart > maxViewSize ? maxViewSize : pfms->fileSize.LowPart;
        }

        pfms->hMap = CreateFileMapping(pfms->hFile, NULL, bWrite ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);
        if (!pfms->hMap)
        {
            break;
        }

        pfms->lpView = MapViewOfFile(pfms->hMap, FILE_MAP_READ | (bWrite ? FILE_MAP_WRITE : 0), 0, 0, pfms->mappedSize);
        if (!pfms->lpView)
        {
            break;
        }
    } while (FALSE);

    return pfms;
}

void CloseFileMapping(PFILE_MAPPING_STRUCT pfms)
{
    if (pfms)
    {
        if (pfms->lpView)
        {
            UnmapViewOfFile(pfms->lpView);
        }

        if (pfms->hMap)
        {
            CloseHandle(pfms->hMap);
        }

        if (INVALID_HANDLE_VALUE != pfms->hFile)
        {
            CloseHandle(pfms->hFile);
        }

        free((void*)pfms);
    }
}

static PFILE_MAPPING_STRUCT WINAPI _PeMappingFile(LPCWSTR fileName, BOOL bWrite, BOOL* b64)
{
    PFILE_MAPPING_STRUCT pfms = MappingFileW(fileName, bWrite, _PE_MAX_MAP_SIZE);
    if (!_PeCheckPeMapping(pfms, b64))
    {
        CloseFileMapping(pfms);
        return NULL;
    }

    return pfms;
}

BOOL IsPeFileW(LPCWSTR fileName, BOOL* b64)
{
    PFILE_MAPPING_STRUCT pfms = NULL;
    if (!fileName)
    {
        return FALSE;
    }

    BOOL bRet = FALSE;

    do
    {
        pfms = _PeMappingFile(fileName, FALSE, b64);
        if (!pfms)
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    CloseFileMapping(pfms);

    return bRet;
}

BOOL GetPeVersionW(LPCWSTR lpszFileName, LPWSTR outBuf, UINT size)
{
    WCHAR* szVersionBuffer = NULL;
    BOOL bRet = FALSE;

    do
    {
        if (!lpszFileName || !outBuf)
        {
            break;
        }

        DWORD dwVerSize;
        DWORD dwHandle;

        dwVerSize = GetFileVersionInfoSizeW(lpszFileName, &dwHandle);
        if (!dwVerSize)
        {
            break;
        }

        szVersionBuffer = (WCHAR*)malloc(dwVerSize * sizeof(WCHAR));
        if (!szVersionBuffer)
        {
            break;
        }
        RtlZeroMemory(szVersionBuffer, dwVerSize);

        if (!GetFileVersionInfoW(lpszFileName, 0, dwVerSize, szVersionBuffer))
        {
            break;
        }

        VS_FIXEDFILEINFO* pInfo;
        unsigned int nInfoLen;
        if (!VerQueryValueW(szVersionBuffer, L"\\", (void**)&pInfo, &nInfoLen))
        {
            break;
        }

        wnsprintfW(
            outBuf,
            size,
            L"%d.%d.%d.%d",
            HIWORD(pInfo->dwFileVersionMS),
            LOWORD(pInfo->dwFileVersionMS),
            HIWORD(pInfo->dwFileVersionLS),
            LOWORD(pInfo->dwFileVersionLS)
            );

        bRet = TRUE;
    } while (FALSE);

    if (szVersionBuffer)
    {
        free((void*)szVersionBuffer);
    }

    return bRet;
}

//Dos文件路径转为Nt路径
ustring DosPathToNtPath(LPCWSTR wszSrc)
{
    DWORD dwDrivers = GetLogicalDrives();
    int iIdex = 0;
    WCHAR wszNT[] = L"X:";
    WCHAR wszDos[MAX_PATH] = {0x00};
    ustring wstrHeader;
    ustring wstrNtPath;
    ustring wstrDos(wszSrc);
    for (iIdex = 0 ; iIdex < 26 ; iIdex++)
    {
        if ((1 << iIdex) & dwDrivers)
        {
            wszNT[0] = 'A' + iIdex;
            if (QueryDosDeviceW(wszNT, wszDos, MAX_PATH))
            {
                if (0 == wstrDos.comparei(wszDos))
                {
                    wstrNtPath += wszNT;
                    wstrNtPath += (wstrDos.c_str() + lstrlenW(wszDos));
                    return wstrNtPath;
                }
            }
        }
    }
    return L"";
}

ustring GetProcPathByPid(IN DWORD dwPid)
{
    if (4 == dwPid || 0 == dwPid)
    {
        return L"";
    }

    ustring wstrPath;
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);

    WCHAR wszImage[MAX_PATH] = {0x00};
    if (process)
    {
        GetProcessImageFileNameW(process, wszImage, MAX_PATH);
        if (wszImage[4] != 0x00)
        {
            wstrPath = DosPathToNtPath(wszImage);
        }
    }
    else
    {
        return L"";
    }

    if (process && INVALID_HANDLE_VALUE != process)
    {
        CloseHandle(process);
    }
    return wstrPath;
}

BOOL IsSystem64()
{
#if WIN64 || _WIN64
    return TRUE;
#else
    typedef void (WINAPI* pfnGetNativeSystemInfo)(LPSYSTEM_INFO);
    pfnGetNativeSystemInfo pGetNativeSystemInfo;
    SYSTEM_INFO SysInfo;

    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (!hKernel32)
    {
        return FALSE;
    }

    pGetNativeSystemInfo = (pfnGetNativeSystemInfo)GetProcAddress(hKernel32, "GetNativeSystemInfo");
    if (pGetNativeSystemInfo == NULL)
    {
        return FALSE;
    }

    pGetNativeSystemInfo(&SysInfo);

    if (SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        return FALSE;
    }

    return TRUE;
#endif
}

PVOID DisableWow64Red()
{
#if _WIN64 || WIN64
    return (PVOID)0xffffffff;
#else
    if (!IsSystem64())
    {
        return (PVOID)0xffffffff;
    }

    typedef BOOL (WINAPI* pfnDisable)(PVOID*);
    pfnDisable disable = (pfnDisable)GetProcAddress(GetModuleHandleA("kernel32.dll"), "Wow64DisableWow64FsRedirection");
    if (disable)
    {
        PVOID oldValue = NULL;
        if (disable(&oldValue))
        {
            return oldValue;
        }
    }

    return (PVOID)0xffffffff;
#endif
}

BOOL RevertWow64Red(PVOID oldValue)
{
#if _WIN64 || WIN64
    return FALSE;
#else
    if ((PVOID)-1 == oldValue)
    {
        return FALSE;
    }

    typedef BOOL (WINAPI* pfnRevert)(PVOID);
    pfnRevert revert = (pfnRevert)GetProcAddress(GetModuleHandleA("kernel32.dll"), "Wow64RevertWow64FsRedirection");
    if (revert)
    {
        return revert(oldValue); 
    }

    return FALSE;
#endif
}

ustring GetFilePathFromHandle(HANDLE hFile)
{
    DWORD dwFileSizeLow = GetFileSize(hFile, NULL); 
    HANDLE hFileMap = NULL;
    void* pMem = NULL;
    ustring wstrPath;

    do 
    {
        if (!dwFileSizeLow)
        {
            break;
        }

        hFileMap = CreateFileMapping(hFile, 
            NULL, 
            PAGE_READONLY,
            0, 
            1,
            NULL
            );
        if (!hFileMap)
        {
            break;
        }

        pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
        if (!pMem)
        {
            break;
        }
        WCHAR wszFileName[MAX_PATH] = {0};
        GetMappedFileNameW(
            GetCurrentProcess(), 
            pMem, 
            wszFileName,
            MAX_PATH
            );
        if (!wszFileName[0])
        {
            break;
        }

        wstrPath = DosPathToNtPath(wszFileName);
    } while (FALSE);

    if (hFileMap)
    {
        CloseHandle(hFileMap);
    }

    if (pMem)
    {
        UnmapViewOfFile(pMem);
    }
    return wstrPath;
}

void IterateModulesW(DWORD procId, pfnModuleHandlerW handler, void* lpParam)
{
    do
    {
        if (!handler)
        {
            break;
        }

        MODULEENTRY32W me32 = {0};
        me32.dwSize = sizeof(me32);

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procId);
        if (INVALID_HANDLE_VALUE == hSnap)
        {
            int e = GetLastError();
            break;
        }

        BOOL bMore = Module32FirstW(hSnap, &me32);
        while (bMore)
        {
            if (!handler(&me32, lpParam))
            {
                // 回调方要求退出
                break;
            }

            bMore = Module32NextW(hSnap, &me32);
        }

        CloseHandle(hSnap);
    } while (FALSE);
}

#define NEW new

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
        char *buffer = NEW char[count];

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
        wchar_t *buffer = NEW wchar_t[count];

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
        char *buffer = NEW char[count];

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
        wchar_t *buffer = NEW wchar_t[count];

        if (buffer != 0)
        {
            MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, count);
            ret = buffer;

            delete []buffer;
        }
    }

    return ret;
}

static BOOL _GetNtVersionNumbers(DWORD &dwMajorVer, DWORD &dwMinorVer, DWORD &dwBuildNumber)
{
    static BOOL s_bInit = FALSE;
    static DWORD s_dwMajorVer;
    static DWORD s_dwMinorVer;
    static DWORD s_dwBuildNumber;

    if (s_bInit)
    {
        dwMajorVer = s_dwMajorVer;
        dwMinorVer = s_dwMinorVer;
        dwBuildNumber = s_dwBuildNumber;
        return TRUE;
    }

    HMODULE hModNtdll = GetModuleHandleA("ntdll.dll");
    if (hModNtdll)
    {
        typedef void (WINAPI* RtlGetNtVersionNumbers)(DWORD*, DWORD*, DWORD*);
        RtlGetNtVersionNumbers pRtlGetNtVersionNumbers = (RtlGetNtVersionNumbers)GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
        if (pRtlGetNtVersionNumbers)
        {
            pRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
            dwBuildNumber &= 0x0ffff;
            s_dwMajorVer = dwMajorVer;
            s_dwMinorVer = dwMinorVer;
            s_dwBuildNumber = dwBuildNumber;
            s_bInit = TRUE;
        }
    }
    return s_bInit;
}

static BOOL _IsWin81Later()
{
    DWORD dwMajorVer = 0;
    DWORD dwMinorVer = 0;
    DWORD dwBuild = 0;

    _GetNtVersionNumbers(dwMajorVer, dwMinorVer, dwBuild);
    if ((dwMajorVer == 6 && dwMinorVer >= 3) || dwMajorVer > 6)
    {
        return TRUE;
    }
    return FALSE;
}

typedef enum VDPROCESSINFOCLASS
{
    VDProcessBasicInformation,      // 0, q: PROCESS_BASIC_INFORMATION, PROCESS_EXTENDED_BASIC_INFORMATION
    VDProcessQuotaLimits,           // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
    VDProcessIoCounters,            // q: IO_COUNTERS
    VDProcessVmCounters,            // q: VM_COUNTERS, VM_COUNTERS_EX, VM_COUNTERS_EX2
    VDProcessTimes,                 // q: KERNEL_USER_TIMES
    VDProcessBasePriority,          // s: KPRIORITY
    VDProcessRaisePriority,         // s: ULONG
    VDProcessDebugPort,             // q: HANDLE
    VDProcessExceptionPort,         // s: HANDLE
    VDProcessAccessToken,           // s: PROCESS_ACCESS_TOKEN
    VDProcessLdtInformation,        // 10, qs: PROCESS_LDT_INFORMATION
    VDProcessLdtSize,               // s: PROCESS_LDT_SIZE
    VDProcessDefaultHardErrorMode,  // qs: ULONG
    VDProcessIoPortHandlers,        // (kernel-mode only)
    VDProcessPooledUsageAndLimits,  // q: POOLED_USAGE_AND_LIMITS
    VDProcessWorkingSetWatch,       // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
    VDProcessUserModeIOPL,
    VDProcessEnableAlignmentFaultFixup, // s: BOOLEAN
    VDProcessPriorityClass,         // qs: PROCESS_PRIORITY_CLASS
    VDProcessWx86Information,
    VDProcessHandleCount,           // 20, q: ULONG, PROCESS_HANDLE_INFORMATION
    VDProcessAffinityMask,          // s: KAFFINITY
    VDProcessPriorityBoost,         // qs: ULONG
    VDProcessDeviceMap,             // qs: PROCESS_DEVICEMAP_INFORMATION, PROCESS_DEVICEMAP_INFORMATION_EX
    VDProcessSessionInformation,    // q: PROCESS_SESSION_INFORMATION
    VDProcessForegroundInformation, // s: PROCESS_FOREGROUND_BACKGROUND
    VDProcessWow64Information,      // q: ULONG_PTR
    VDProcessImageFileName,         // q: UNICODE_STRING
    VDProcessLUIDDeviceMapsEnabled, // q: ULONG
    VDProcessBreakOnTermination,    // qs: ULONG
    VDProcessDebugObjectHandle,     // 30, q: HANDLE
    VDProcessDebugFlags,            // qs: ULONG
    VDProcessHandleTracing,         // q: PROCESS_HANDLE_TRACING_QUERY; s: size 0 disables, otherwise enables
    VDProcessIoPriority,            // qs: ULONG
    VDProcessExecuteFlags,          // qs: ULONG
    VDProcessResourceManagement,
    VDProcessCookie,                // q: ULONG
    VDProcessImageInformation,      // q: SECTION_IMAGE_INFORMATION
    VDProcessCycleTime,             // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
    VDProcessPagePriority,          // q: ULONG
    VDProcessInstrumentationCallback, // 40
    VDProcessThreadStackAllocation, // s: PROCESS_STACK_ALLOCATION_INFORMATION, PROCESS_STACK_ALLOCATION_INFORMATION_EX
    VDProcessWorkingSetWatchEx,     // q: PROCESS_WS_WATCH_INFORMATION_EX[]
    VDProcessImageFileNameWin32,    // q: UNICODE_STRING
    VDProcessImageFileMapping,      // q: HANDLE (input)
    VDProcessAffinityUpdateMode,    // qs: PROCESS_AFFINITY_UPDATE_MODE
    VDProcessMemoryAllocationMode,  // qs: PROCESS_MEMORY_ALLOCATION_MODE
    VDProcessGroupInformation,      // q: USHORT[]
    VDProcessTokenVirtualizationEnabled, // s: ULONG
    VDProcessConsoleHostProcess,    // q: ULONG_PTR
    VDProcessWindowInformation,     // 50, q: PROCESS_WINDOW_INFORMATION
    VDProcessHandleInformation,     // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since WIN8
    VDProcessMitigationPolicy,      // s: PROCESS_MITIGATION_POLICY_INFORMATION
    VDProcessDynamicFunctionTableInformation,
    VDProcessHandleCheckingMode,
    VDProcessKeepAliveCount,        // q: PROCESS_KEEPALIVE_COUNT_INFORMATION
    VDProcessRevokeFileHandles,     // s: PROCESS_REVOKE_FILE_HANDLES_INFORMATION
    VDProcessWorkingSetControl,     // s: PROCESS_WORKING_SET_CONTROL
    VDProcessHandleTable,           // since WINBLUE
    VDProcessCheckStackExtentsMode,
    VDProcessCommandLineInformation,    // 60, q: UNICODE_STRING
    VDProcessProtectionInformation,     // q: PS_PROTECTION
    VDProcessMemoryExhaustion,          // PROCESS_MEMORY_EXHAUSTION_INFO // since THRESHOLD
    VDProcessFaultInformation,          // PROCESS_FAULT_INFORMATION
    VDProcessTelemetryIdInformation,    // PROCESS_TELEMETRY_ID_INFORMATION
    VDProcessCommitReleaseInformation,  // PROCESS_COMMIT_RELEASE_INFORMATION
    VDProcessDefaultCpuSetsInformation,
    VDProcessAllowedCpuSetsInformation,
    VDProcessReserved1Information,
    VDProcessReserved2Information,
    VDProcessSubsystemProcess,          // 70
    VDProcessJobMemoryInformation,      // PROCESS_JOB_MEMORY_INFO
    VDMaxProcessInfoClass
} VDPROCESSINFOCLASS;

#define RTL_MAX_DRIVE_LETTERS 32

typedef NTSTATUS (WINAPI *pfnNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;

    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PVOID Environment;

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectories[32];

    ULONG EnvironmentSize;
    ULONG EnvironmentVersion;
    PVOID PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

#define WOW64_POINTER(Type) ULONG

typedef struct _STRING32
{
    USHORT Length;
    USHORT MaximumLength;
    ULONG Buffer;
} STRING32, *PSTRING32;

typedef struct _RTL_DRIVE_LETTER_CURDIR32
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32, *PRTL_DRIVE_LETTER_CURDIR32;

typedef STRING32 UNICODE_STRING32, *PUNICODE_STRING32;

typedef struct _CURDIR32
{
    UNICODE_STRING32 DosPath;
    WOW64_POINTER(HANDLE) Handle;
} CURDIR32, *PCURDIR32;

typedef struct _RTL_USER_PROCESS_PARAMETERS32
{
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    WOW64_POINTER(HANDLE) ConsoleHandle;
    ULONG ConsoleFlags;
    WOW64_POINTER(HANDLE) StandardInput;
    WOW64_POINTER(HANDLE) StandardOutput;
    WOW64_POINTER(HANDLE) StandardError;

    CURDIR32 CurrentDirectory;
    UNICODE_STRING32 DllPath;
    UNICODE_STRING32 ImagePathName;
    UNICODE_STRING32 CommandLine;
    WOW64_POINTER(PVOID) Environment;

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING32 WindowTitle;
    UNICODE_STRING32 DesktopInfo;
    UNICODE_STRING32 ShellInfo;
    UNICODE_STRING32 RuntimeData;
    RTL_DRIVE_LETTER_CURDIR32 CurrentDirectories[RTL_MAX_DRIVE_LETTERS];

    ULONG EnvironmentSize;
    ULONG EnvironmentVersion;
    WOW64_POINTER(PVOID) PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS32, *PRTL_USER_PROCESS_PARAMETERS32;

typedef enum _PH_PEB_OFFSET
{
    PhpoCurrentDirectory,
    PhpoDllPath,
    PhpoImagePathName,
    PhpoCommandLine,
    PhpoWindowTitle,
    PhpoDesktopInfo,
    PhpoShellInfo,
    PhpoRuntimeData,
    PhpoTypeMask = 0xffff,

    PhpoWow64 = 0x10000
} PH_PEB_OFFSET;

pfnNtQueryInformationProcess GetNtQueryInformationProc()
{
    static pfnNtQueryInformationProcess s_pfn = NULL;
    if (!s_pfn)
    {
        s_pfn = (pfnNtQueryInformationProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
    }
    return s_pfn;
}

static BOOL _PhpQueryProcessVariableSize(
    _In_ HANDLE ProcessHandle,
    _In_ VDPROCESSINFOCLASS ProcessInformationClass,
    _Out_ char *pBuffer,
    IN OUT DWORD *pLength
    )
{
    pfnNtQueryInformationProcess pfn = GetNtQueryInformationProc();
    NTSTATUS status;
    ULONG retLength = 0;

    status = pfn(
        ProcessHandle,
        ProcessInformationClass,
        NULL,
        0,
        &retLength
        );

    if (!retLength)
    {
        return FALSE;
    }

    if (pLength[0] < retLength)
    {
        pLength[0] = retLength;
        return FALSE;
    }

    status = pfn(
        ProcessHandle,
        ProcessInformationClass,
        pBuffer,
        pLength[0],
        &retLength
        );
    return (0 == status);
}

#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))

#define GDI_HANDLE_BUFFER_SIZE32 34
#define GDI_HANDLE_BUFFER_SIZE64 60

#ifndef WIN64
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE32
#else
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE64
#endif

typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE ShutdownThreadId;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _VDPEB
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN SpareBits : 1;
        };
    };
    HANDLE Mutant;

    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
        ULONG EnvironmentUpdateCount;
    };
    union
    {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    PVOID ApiSetMap;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID HotpatchInformation;
    PVOID *ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;

    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;

    PRTL_CRITICAL_SECTION LoaderLock;

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ImageProcessAffinityMask;
    GDI_HANDLE_BUFFER GdiHandleBuffer;
    PVOID PostProcessInitRoutine;

    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];

    ULONG SessionId;

    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;

    UNICODE_STRING CSDVersion;

    PVOID ActivationContextData;
    PVOID ProcessAssemblyStorageMap;
    PVOID SystemDefaultActivationContextData;
    PVOID SystemAssemblyStorageMap;

    SIZE_T MinimumStackCommit;

    PVOID *FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
    PVOID pContextData;
    PVOID pImageHeaderHash;
    union
    {
        ULONG TracingFlags;
        struct
        {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    ULONGLONG CsrServerReadOnlySharedMemoryBase;
} VDPEB, *PVDPEB;

typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

typedef struct _PEB32
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsLegacyProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN SpareBits : 1;
        };
    };
    WOW64_POINTER(HANDLE) Mutant;

    WOW64_POINTER(PVOID) ImageBaseAddress;
    WOW64_POINTER(PPEB_LDR_DATA) Ldr;
    WOW64_POINTER(PRTL_USER_PROCESS_PARAMETERS) ProcessParameters;
    WOW64_POINTER(PVOID) SubSystemData;
    WOW64_POINTER(PVOID) ProcessHeap;
    WOW64_POINTER(PRTL_CRITICAL_SECTION) FastPebLock;
    WOW64_POINTER(PVOID) AtlThunkSListPtr;
    WOW64_POINTER(PVOID) IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ReservedBits0 : 27;
        };
        ULONG EnvironmentUpdateCount;
    };
    union
    {
        WOW64_POINTER(PVOID) KernelCallbackTable;
        WOW64_POINTER(PVOID) UserSharedInfoPtr;
    };
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    WOW64_POINTER(PVOID) ApiSetMap;
    ULONG TlsExpansionCounter;
    WOW64_POINTER(PVOID) TlsBitmap;
    ULONG TlsBitmapBits[2];
    WOW64_POINTER(PVOID) ReadOnlySharedMemoryBase;
    WOW64_POINTER(PVOID) HotpatchInformation;
    WOW64_POINTER(PVOID *) ReadOnlyStaticServerData;
    WOW64_POINTER(PVOID) AnsiCodePageData;
    WOW64_POINTER(PVOID) OemCodePageData;
    WOW64_POINTER(PVOID) UnicodeCaseTableData;

    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

    LARGE_INTEGER CriticalSectionTimeout;
    WOW64_POINTER(SIZE_T) HeapSegmentReserve;
    WOW64_POINTER(SIZE_T) HeapSegmentCommit;
    WOW64_POINTER(SIZE_T) HeapDeCommitTotalFreeThreshold;
    WOW64_POINTER(SIZE_T) HeapDeCommitFreeBlockThreshold;

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    WOW64_POINTER(PVOID *) ProcessHeaps;

    WOW64_POINTER(PVOID) GdiSharedHandleTable;
    WOW64_POINTER(PVOID) ProcessStarterHelper;
    ULONG GdiDCAttributeList;

    WOW64_POINTER(PRTL_CRITICAL_SECTION) LoaderLock;

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    WOW64_POINTER(ULONG_PTR) ImageProcessAffinityMask;
    GDI_HANDLE_BUFFER32 GdiHandleBuffer;
    WOW64_POINTER(PVOID) PostProcessInitRoutine;

    WOW64_POINTER(PVOID) TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];

    ULONG SessionId;

    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    WOW64_POINTER(PVOID) pShimData;
    WOW64_POINTER(PVOID) AppCompatInfo;

    UNICODE_STRING32 CSDVersion;

    WOW64_POINTER(PVOID) ActivationContextData;
    WOW64_POINTER(PVOID) ProcessAssemblyStorageMap;
    WOW64_POINTER(PVOID) SystemDefaultActivationContextData;
    WOW64_POINTER(PVOID) SystemAssemblyStorageMap;

    WOW64_POINTER(SIZE_T) MinimumStackCommit;

    WOW64_POINTER(PVOID *) FlsCallback;
    LIST_ENTRY32 FlsListHead;
    WOW64_POINTER(PVOID) FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;

    WOW64_POINTER(PVOID) WerRegistrationData;
    WOW64_POINTER(PVOID) WerShipAssertPtr;
    WOW64_POINTER(PVOID) pContextData;
    WOW64_POINTER(PVOID) pImageHeaderHash;
    union
    {
        ULONG TracingFlags;
        struct
        {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        };
    };
    ULONGLONG CsrServerReadOnlySharedMemoryBase;
} PEB32, *PPEB32;

static ustring _GetProcressPebString(HANDLE hProcress, ULONG eOffsetType)
{
    ULONG uOffset = 0;
    switch (eOffsetType)
    {
    case PhpoCurrentDirectory:
        break;
    case (PhpoCurrentDirectory | PhpoWow64):
        break;
    case  PhpoCommandLine:
        uOffset = FIELD_OFFSET(RTL_USER_PROCESS_PARAMETERS, CommandLine);
        break;
    case (PhpoCommandLine | PhpoWow64):
        uOffset = FIELD_OFFSET(RTL_USER_PROCESS_PARAMETERS32, CommandLine);
        break;
    default:
        break;
    }

    pfnNtQueryInformationProcess pfn = GetNtQueryInformationProc();
    char szBuffer[1024] = {0};
    //x86
    if (eOffsetType & PhpoWow64)
    {
        PVOID peb32;
        ULONG processParameters32;
        UNICODE_STRING32 unicodeString32;

        pfn(hProcress, ProcessWow64Information, &peb32, sizeof(PVOID), NULL);
        ReadProcessMemory(
            hProcress,
            PTR_ADD_OFFSET(peb32, FIELD_OFFSET(PEB32, ProcessParameters)),
            &processParameters32,
            sizeof(ULONG),
            NULL
            );

        ReadProcessMemory(
            hProcress,
            PTR_ADD_OFFSET(processParameters32, uOffset),
            &unicodeString32,
            sizeof(UNICODE_STRING32),
            NULL
            );

        UNICODE_STRING32 *pStr = (UNICODE_STRING32 *)szBuffer;
        ReadProcessMemory(
            hProcress,
            (LPCVOID)unicodeString32.Buffer,
            pStr,
            unicodeString32.Length,
            NULL
            );
        return (LPCWSTR)pStr->Buffer;
    }
    //x64
    else
    {
        PROCESS_BASIC_INFORMATION basicInfo;
        PVOID processParameters;
        UNICODE_STRING unicodeString;

        // Get the PEB address
        pfn(hProcress, ProcessBasicInformation, &basicInfo, sizeof(basicInfo), NULL);
        ReadProcessMemory(
            hProcress,
            PTR_ADD_OFFSET(basicInfo.PebBaseAddress, FIELD_OFFSET(VDPEB, ProcessParameters)),
            &processParameters,
            sizeof(PVOID),
            NULL
            );

        ReadProcessMemory(
            hProcress,
            PTR_ADD_OFFSET(processParameters, uOffset),
            &unicodeString,
            sizeof(UNICODE_STRING),
            NULL
            );

        UNICODE_STRING *pStr = (UNICODE_STRING *)szBuffer;
        ReadProcessMemory(hProcress,
            unicodeString.Buffer,
            pStr,
            unicodeString.Length,
            NULL
            );
        return pStr->Buffer;
    }
}

ustring GetProcessCommandLine(_In_ DWORD dwPid, BOOL bx64)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
    if (!hProcess)
    {
        return L"";
    }

    if (_IsWin81Later())
    {
        char szBuffer[1024] = {0};
        UNICODE_STRING *pStr = (UNICODE_STRING *)szBuffer;
        DWORD dwLength = sizeof(szBuffer);
        _PhpQueryProcessVariableSize(hProcess, VDProcessCommandLineInformation, szBuffer, &dwLength);
        CloseHandle(hProcess);
        return pStr->Buffer;
    }
    else
    {
        ULONG uFlag = (PhpoCommandLine | PhpoWow64);
        if (bx64)
        {
            uFlag = PhpoCommandLine;
        }
        return _GetProcressPebString(hProcess, uFlag);
    }
}

BOOL ShlParseShortcutsW(LPCWSTR wszLnkFile, PGDS_LINKINFO info)
{
    BOOL bRet = FALSE;
    IShellLinkW* pLink = NULL;
    IPersistFile* ppf = NULL;

    do
    {
        if (!wszLnkFile || !info)
        {
            break;
        }

        WIN32_FIND_DATAW wfd = {0};
        if ((S_OK != CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&pLink)) ||
            (S_OK != (pLink->QueryInterface(IID_IPersistFile, (void**)&ppf))) ||
            (S_OK != ppf->Load(wszLnkFile, STGM_READ)) ||
            (S_OK != pLink->Resolve(NULL, SLR_ANY_MATCH | SLR_NO_UI)) ||
            (S_OK != pLink->GetPath(EXPAND_ARG(info->wszPath), &wfd, SLGP_RAWPATH))
            )
        {
            break;
        }

        pLink->GetArguments(EXPAND_ARG(info->wszArgs));
        pLink->GetIconLocation(EXPAND_ARG(info->wszIcon), &(info->nIconIdx));
        pLink->GetWorkingDirectory(EXPAND_ARG(info->wszWorkDir));
        pLink->GetDescription(EXPAND_ARG(info->wszDesc));

        bRet = TRUE;
    } while (FALSE);

    if (pLink)
    {
        pLink->Release();
    }

    if (ppf)
    {
        ppf->Release();
    }

    return bRet;
}

void test()
{
    int offset = (PhpoCurrentDirectory | PhpoWow64);
    switch (offset)
    {
    case  PhpoCurrentDirectory:
        {
            int d = 1;
        }
        break;
    case  PhpoCurrentDirectory | PhpoWow64:
        {
            int d = 2;
        }
        break;
    }
}