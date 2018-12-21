#include "ComStatic.h"
#include <Windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Psapi.lib")

using namespace std;

//Dos文件路径转为Nt路径
ustring __stdcall DosPathToNtPath(LPCWSTR wszSrc)
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

ustring __stdcall GetProcPathByPid(IN DWORD dwPid)
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

ustring __stdcall GetFilePathFromHandle(HANDLE hFile)
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

ustring __stdcall GetStdErrorStr(DWORD dwErr)
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

std::wstring __stdcall RegGetStrValueExW(HKEY hKey, LPCWSTR wszSubKey, LPCWSTR wszValue)
{
    if (!wszSubKey || !wszValue)
    {
        return L"";
    }
    DWORD dwLength = 0;
    LPVOID pBuf = NULL;
    DWORD dwType = 0;
    SHGetValueW(
        hKey,
        wszSubKey,
        wszValue,
        &dwType,
        (LPVOID)pBuf,
        &dwLength
        );
    if (REG_SZ != dwType || !dwLength)
    {
        return L"";
    }
    dwLength += 2;
    pBuf = new BYTE[dwLength];
    memset(pBuf, 0x00, dwLength);
    SHGetValueW(
        hKey,
        wszSubKey,
        wszValue,
        NULL,
        pBuf,
        &dwLength
        );
    std::wstring wstrRes = (LPCWSTR)pBuf;
    delete []pBuf;
    return wstrRes;
}

typedef LONG NTSTATUS;
typedef LONG KPRIORITY;

// 来自Process Hacker源码
// private
// source:http://www.microsoft.com/whdc/system/Sysinternals/MoreThan64proc.mspx
//typedef enum _NTSYSTEM_INFORMATION_CLASS
//{
//    SystemBasicInformation,                 // q: SYSTEM_BASIC_INFORMATION
//    SystemProcessorInformation,             // q: SYSTEM_PROCESSOR_INFORMATION
//    SystemPerformanceInformation,           // q: SYSTEM_PERFORMANCE_INFORMATION
//    SystemTimeOfDayInformation,             // q: SYSTEM_TIMEOFDAY_INFORMATION
//    SystemPathInformation,                  // not implemented
//    SystemProcessInformation,               // q: SYSTEM_PROCESS_INFORMATION
//    SystemCallCountInformation,             // q: SYSTEM_CALL_COUNT_INFORMATION
//    SystemDeviceInformation,                // q: SYSTEM_DEVICE_INFORMATION
//    SystemProcessorPerformanceInformation,  // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
//    SystemFlagsInformation,                 // q: SYSTEM_FLAGS_INFORMATION
//    SystemCallTimeInformation,              // 10, not implemented
//    SystemModuleInformation,                 // q: RTL_PROCESS_MODULES
//    SystemLocksInformation,
//    SystemStackTraceInformation,
//    SystemPagedPoolInformation,             // not implemented
//    SystemNonPagedPoolInformation,          // not implemented
//    SystemHandleInformation,                // q: SYSTEM_HANDLE_INFORMATION
//    SystemObjectInformation,                // q: SYSTEM_OBJECTTYPE_INFORMATION mixed with SYSTEM_OBJECT_INFORMATION
//    SystemPageFileInformation,              // q: SYSTEM_PAGEFILE_INFORMATION
//    SystemVdmInstemulInformation,           // q
//    SystemVdmBopInformation,                // 20, not implemented
//    SystemFileCacheInformation,             // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypeSystemCache)
//    SystemPoolTagInformation,               // q: SYSTEM_POOLTAG_INFORMATION
//    SystemInterruptInformation,             // q: SYSTEM_INTERRUPT_INFORMATION
//    SystemDpcBehaviorInformation,           // q: SYSTEM_DPC_BEHAVIOR_INFORMATION; s: SYSTEM_DPC_BEHAVIOR_INFORMATION (requires SeLoadDriverPrivilege)
//    SystemFullMemoryInformation,            // not implemented
//    SystemLoadGdiDriverInformation,         // s (kernel-mode only)
//    SystemUnloadGdiDriverInformation,       // s (kernel-mode only)
//    SystemTimeAdjustmentInformation,        // q: SYSTEM_QUERY_TIME_ADJUST_INFORMATION; s: SYSTEM_SET_TIME_ADJUST_INFORMATION (requires SeSystemtimePrivilege)
//    SystemSummaryMemoryInformation,         // not implemented
//    SystemMirrorMemoryInformation,          // 30, s (requires license value "Kernel-MemoryMirroringSupported") (requires SeShutdownPrivilege)
//    SystemPerformanceTraceInformation,      // s
//    SystemObsolete0,                        // not implemented
//    SystemExceptionInformation,             // q: SYSTEM_EXCEPTION_INFORMATION
//    SystemCrashDumpStateInformation,        // s (requires SeDebugPrivilege)
//    SystemKernelDebuggerInformation,        // q: SYSTEM_KERNEL_DEBUGGER_INFORMATION
//    SystemContextSwitchInformation,         // q: SYSTEM_CONTEXT_SWITCH_INFORMATION
//    SystemRegistryQuotaInformation,         // q: SYSTEM_REGISTRY_QUOTA_INFORMATION; s (requires SeIncreaseQuotaPrivilege)
//    SystemExtendServiceTableInformation,    // s (requires SeLoadDriverPrivilege) // loads win32k only
//    SystemPrioritySeperation,               // s (requires SeTcbPrivilege)
//    SystemVerifierAddDriverInformation,     // 40, s (requires SeDebugPrivilege)
//    SystemVerifierRemoveDriverInformation,  // s (requires SeDebugPrivilege)
//    SystemProcessorIdleInformation,         // q: SYSTEM_PROCESSOR_IDLE_INFORMATION
//    SystemLegacyDriverInformation,          // q: SYSTEM_LEGACY_DRIVER_INFORMATION
//    SystemCurrentTimeZoneInformation,       // q
//    SystemLookasideInformation,             // q: SYSTEM_LOOKASIDE_INFORMATION
//    SystemTimeSlipNotification,             // s (requires SeSystemtimePrivilege)
//    SystemSessionCreate,                    // not implemented
//    SystemSessionDetach,                    // not implemented
//    SystemSessionInformation,               // not implemented
//    SystemRangeStartInformation,            // 50, q
//    SystemVerifierInformation,              // q: SYSTEM_VERIFIER_INFORMATION; s (requires SeDebugPrivilege)
//    SystemVerifierThunkExtend,              // s (kernel-mode only)
//    SystemSessionProcessInformation,        // q: SYSTEM_SESSION_PROCESS_INFORMATION
//    SystemLoadGdiDriverInSystemSpace,       // s (kernel-mode only) (same as SystemLoadGdiDriverInformation)
//    SystemNumaProcessorMap,                 // q
//    SystemPrefetcherInformation,            // q: PREFETCHER_INFORMATION; s: PREFETCHER_INFORMATION // PfSnQueryPrefetcherInformation
//    SystemExtendedProcessInformation,       // q: SYSTEM_PROCESS_INFORMATION
//    SystemRecommendedSharedDataAlignment,   // q
//    SystemComPlusPackage,                   // q; s
//    SystemNumaAvailableMemory,              // 60
//    SystemProcessorPowerInformation,        // q: SYSTEM_PROCESSOR_POWER_INFORMATION
//    SystemEmulationBasicInformation,        // q
//    SystemEmulationProcessorInformation,
//    SystemExtendedHandleInformation,        // q: SYSTEM_HANDLE_INFORMATION_EX
//    SystemLostDelayedWriteInformation,      // q: ULONG
//    SystemBigPoolInformation,               // q: SYSTEM_BIGPOOL_INFORMATION
//    SystemSessionPoolTagInformation,        // q: SYSTEM_SESSION_POOLTAG_INFORMATION
//    SystemSessionMappedViewInformation,     // q: SYSTEM_SESSION_MAPPED_VIEW_INFORMATION
//    SystemHotpatchInformation,              // q; s
//    SystemObjectSecurityMode,               // 70, q
//    SystemWatchdogTimerHandler,             // s (kernel-mode only)
//    SystemWatchdogTimerInformation,         // q (kernel-mode only); s (kernel-mode only)
//    SystemLogicalProcessorInformation,      // q: SYSTEM_LOGICAL_PROCESSOR_INFORMATION
//    SystemWow64SharedInformationObsolete,   // not implemented
//    SystemRegisterFirmwareTableInformationHandler, // s (kernel-mode only)
//    SystemFirmwareTableInformation,         // not implemented
//    SystemModuleInformationEx,              // q: RTL_PROCESS_MODULE_INFORMATION_EX
//    SystemVerifierTriageInformation,        // not implemented
//    SystemSuperfetchInformation,            // q: SUPERFETCH_INFORMATION; s: SUPERFETCH_INFORMATION // PfQuerySuperfetchInformation
//    SystemMemoryListInformation,            // 80, q: SYSTEM_MEMORY_LIST_INFORMATION; s: SYSTEM_MEMORY_LIST_COMMAND (requires SeProfileSingleProcessPrivilege)
//    SystemFileCacheInformationEx,           // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (same as SystemFileCacheInformation)
//    SystemThreadPriorityClientIdInformation,// s: SYSTEM_THREAD_CID_PRIORITY_INFORMATION (requires SeIncreaseBasePriorityPrivilege)
//    SystemProcessorIdleCycleTimeInformation,// q: SYSTEM_PROCESSOR_IDLE_CYCLE_TIME_INFORMATION[]
//    SystemVerifierCancellationInformation,  // not implemented // name:wow64:whNT32QuerySystemVerifierCancellationInformation
//    SystemProcessorPowerInformationEx,      // not implemented
//    SystemRefTraceInformation,              // q; s // ObQueryRefTraceInformation
//    SystemSpecialPoolInformation,           // q; s (requires SeDebugPrivilege) // MmSpecialPoolTag, then MmSpecialPoolCatchOverruns != 0
//    SystemProcessIdInformation,             // q: SYSTEM_PROCESS_ID_INFORMATION
//    SystemErrorPortInformation,             // s (requires SeTcbPrivilege)
//    SystemBootEnvironmentInformation,       // 90, q: SYSTEM_BOOT_ENVIRONMENT_INFORMATION
//    SystemHypervisorInformation,            // q; s (kernel-mode only)
//    SystemVerifierInformationEx,            // q; s
//    SystemTimeZoneInformation,              // s (requires SeTimeZonePrivilege)
//    SystemImageFileExecutionOptionsInformation, // s: SYSTEM_IMAGE_FILE_EXECUTION_OPTIONS_INFORMATION (requires SeTcbPrivilege)
//    SystemCoverageInformation,              // q; s // name:wow64:whNT32QuerySystemCoverageInformation; ExpCovQueryInformation
//    SystemPrefetchPatchInformation,         // not implemented
//    SystemVerifierFaultsInformation,        // s (requires SeDebugPrivilege)
//    SystemSystemPartitionInformation,       // q: SYSTEM_SYSTEM_PARTITION_INFORMATION
//    SystemSystemDiskInformation,            // q: SYSTEM_SYSTEM_DISK_INFORMATION
//    SystemProcessorPerformanceDistribution, // 100, q: SYSTEM_PROCESSOR_PERFORMANCE_DISTRIBUTION
//    SystemNumaProximityNodeInformation,     // q
//    SystemDynamicTimeZoneInformation,       // q; s (requires SeTimeZonePrivilege)
//    SystemCodeIntegrityInformation,         // q // SeCodeIntegrityQueryInformation
//    SystemProcessorMicrocodeUpdateInformation, // s
//    SystemProcessorBrandString,             // q // HaliQuerySystemInformation -> HalpGetProcessorBrandString, info class 23
//    SystemVirtualAddressInformation,        // q: SYSTEM_VA_LIST_INFORMATION[]; s: SYSTEM_VA_LIST_INFORMATION[] (requires SeIncreaseQuotaPrivilege) // MmQuerySystemVaInformation
//    SystemLogicalProcessorAndGroupInformation, // q: SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX // since WIN7 // KeQueryLogicalProcessorRelationship
//    SystemProcessorCycleTimeInformation,    // q: SYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION[]
//    SystemStoreInformation,                 // q; s // SmQueryStoreInformation
//    SystemRegistryAppendString,             // 110, s: SYSTEM_REGISTRY_APPEND_STRING_PARAMETERS
//    SystemAitSamplingValue,                 // s: ULONG (requires SeProfileSingleProcessPrivilege)
//    SystemVhdBootInformation,               // q: SYSTEM_VHD_BOOT_INFORMATION
//    SystemCpuQuotaInformation,              // q; s // PsQueryCpuQuotaInformation
//    SystemNativeBasicInformation,           // not implemented
//    SystemSpare1,                           // not implemented
//    SystemLowPriorityIoInformation,         // q: SYSTEM_LOW_PRIORITY_IO_INFORMATION
//    SystemTpmBootEntropyInformation,        // q: TPM_BOOT_ENTROPY_NT_RESULT // ExQueryTpmBootEntropyInformation
//    SystemVerifierCountersInformation,      // q: SYSTEM_VERIFIER_COUNTERS_INFORMATION
//    SystemPagedPoolInformationEx,           // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypePagedPool)
//    SystemSystemPtesInformationEx,          // 120, q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypeSystemPtes)
//    SystemNodeDistanceInformation,          // q
//    SystemAcpiAuditInformation,             // q: SYSTEM_ACPI_AUDIT_INFORMATION // HaliQuerySystemInformation -> HalpAuditQueryResults, info class 26
//    SystemBasicPerformanceInformation,      // q: SYSTEM_BASIC_PERFORMANCE_INFORMATION // name:wow64:whNtQuerySystemInformation_SystemBasicPerformanceInformation
//    SystemQueryPerformanceCounterInformation,// q: SYSTEM_QUERY_PERFORMANCE_COUNTER_INFORMATION // since WIN7 SP1
//    SystemSessionBigPoolInformation,        // since WIN8
//    SystemBootGraphicsInformation,
//    SystemScrubPhysicalMemoryInformation,
//    SystemBadPageInformation,
//    SystemProcessorProfileControlArea,
//    SystemCombinePhysicalMemoryInformation,  // 130
//    SystemEntropyInterruptTimingCallback,
//    SystemConsoleInformation,
//    SystemPlatformBinaryInformation,
//    SystemThrottleNotificationInformation,
//    SystemHypervisorProcessorCountInformation,
//    SystemDeviceDataInformation,
//    SystemDeviceDataEnumerationInformation,
//    SystemMemoryTopologyInformation,
//    SystemMemoryChannelInformation,
//    SystemBootLogoInformation,                  // 140
//    SystemProcessorPerformanceInformationEx,    // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_EX // since WINBLUE
//    SystemSpare0,
//    SystemSecureBootPolicyInformation,
//    SystemPageFileInformationEx,                // q: SYSTEM_PAGEFILE_INFORMATION_EX
//    SystemSecureBootInformation,
//    SystemEntropyInterruptTimingRawInformation,
//    SystemPortableWorkspaceEfiLauncherInformation,
//    SystemFullProcessInformation,               // q: SYSTEM_PROCESS_INFORMATION with SYSTEM_PROCESS_INFORMATION_EXTENSION (requires admin)
//    SystemKernelDebuggerInformationEx,          // q: SYSTEM_KERNEL_DEBUGGER_INFORMATION_EX
//    SystemBootMetadataInformation,              // 150
//    SystemSoftRebootInformation,
//    SystemElamCertificateInformation,
//    SystemOfflineDumpConfigInformation,
//    SystemProcessorFeaturesInformation,         // q: SYSTEM_PROCESSOR_FEATURES_INFORMATION
//    SystemRegistryReconciliationInformation,
//    SystemEdidInformation,
//    SystemManufacturingInformation,             // q: SYSTEM_MANUFACTURING_INFORMATION // since THRESHOLD
//    SystemEnergyEstimationConfigInformation,    // q: SYSTEM_ENERGY_ESTIMATION_CONFIG_INFORMATION
//    SystemHypervisorDetailInformation,          // q: SYSTEM_HYPERVISOR_DETAIL_INFORMATION
//    SystemProcessorCycleStatsInformation,       // q: SYSTEM_PROCESSOR_CYCLE_STATS_INFORMATION // 160
//    SystemVmGenerationCountInformation,
//    SystemTrustedPlatformModuleInformation,     // q: SYSTEM_TPM_INFORMATION
//    SystemKernelDebuggerFlags,
//    SystemCodeIntegrityPolicyInformation,
//    SystemIsolatedUserModeInformation,
//    SystemHardwareSecurityTestInterfaceResultsInformation,
//    SystemSingleModuleInformation,              // q: SYSTEM_SINGLE_MODULE_INFORMATION
//    SystemAllowedCpuSetsInformation,
//    SystemDmaProtectionInformation,
//    SystemInterruptCpuSetsInformation,
//    SystemSecureBootPolicyFullInformation,
//    SystemCodeIntegrityPolicyFullInformation,
//    SystemAffinitizedInterruptProcessorInformation,
//    SystemRootSiloInformation,                  // q: SYSTEM_ROOT_SILO_INFORMATION
//    MaxSystemInfoClass
//} NTSYSTEM_INFORMATION_CLASS;

typedef struct _VM_COUNTERS {
    SIZE_T  PeakVirtualSize;
    SIZE_T  VirtualSize;
    ULONG   PageFaultCount;
    SIZE_T  PeakWorkingSetSize;
    SIZE_T  WorkingSetSize;
    SIZE_T  QuotaPeakPagedPoolUsage;
    SIZE_T  QuotaPagedPoolUsage;
    SIZE_T  QuotaPeakNonPagedPoolUsage;
    SIZE_T  QuotaNonPagedPoolUsage;
    SIZE_T  PagefileUsage;
    SIZE_T  PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _CLIENT_ID {
    DWORD        UniqueProcess;
    DWORD        UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

typedef struct _NTSYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} NTSYSTEM_THREAD_INFORMATION, *NTPSYSTEM_THREAD_INFORMATION;

typedef struct _NT_SYSTEM_PROCESS_INFORMATION {
    ULONG            NextEntryDelta;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY        BasePriority;
    ULONG            ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS        VmCounters;
    IO_COUNTERS        IoCounters;
    NTSYSTEM_THREAD_INFORMATION Threads[5];
} NTSYSTEM_PROCESS_INFORMATION, *PNTSYSTEM_PROCESS_INFORMATION;

BOOL __stdcall GetThreadInformation(DWORD dwProcressId, list<ThreadInformation> &vThreads)
{
    typedef DWORD (WINAPI* pfnNtQuerySystem)(UINT, PVOID, DWORD,PDWORD);
    static pfnNtQuerySystem s_pfnQuerySystem = (pfnNtQuerySystem)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
    static DWORD s_dwBufferSize = (4096 * 16);
    static char *s_pBuffer = new char[s_dwBufferSize];

    if (!s_pfnQuerySystem)
    {
        return FALSE;
    }

    DWORD dwRetLength = 0;
    s_pfnQuerySystem(SystemProcessInformation, s_pBuffer, s_dwBufferSize, &dwRetLength);
    if (dwRetLength > s_dwBufferSize)
    {
        delete []s_pBuffer;
        s_dwBufferSize = dwRetLength;
        s_pBuffer = new char[s_dwBufferSize];
        dwRetLength = 0;
        s_pfnQuerySystem(SystemProcessInformation, s_pBuffer, s_dwBufferSize, &dwRetLength);
    }

    if (!dwRetLength)
    {
        return FALSE;
    }

    PNTSYSTEM_PROCESS_INFORMATION pProcInfo = (PNTSYSTEM_PROCESS_INFORMATION)s_pBuffer;
    while (TRUE)
    {
        if (pProcInfo->ProcessId != dwProcressId)
        {
            if (!pProcInfo->NextEntryDelta)
            {
                break;
            }

            pProcInfo = (PNTSYSTEM_PROCESS_INFORMATION)((char *)pProcInfo + pProcInfo->NextEntryDelta);
            continue;
        }

        for (int i = 0 ; i < (int)pProcInfo->ThreadCount ; i++)
        {
            ThreadInformation td;
            td.m_dwThreadId = pProcInfo->Threads[i].ClientId.UniqueThread;
            memcpy(&td.m_vCreateTime, &(pProcInfo->Threads[i].CreateTime), sizeof(FILETIME));
            td.m_dwStartAddr = (DWORD64)pProcInfo->Threads[i].StartAddress;
            td.m_dwSwitchCount = pProcInfo->Threads[i].ContextSwitchCount;
            td.m_eStat = (ThreadStat)pProcInfo->Threads[i].State;
            td.m_eWaitReason = (ThreadWaitReason)pProcInfo->Threads[i].WaitReason;
            td.m_Priority = pProcInfo->Threads[i].Priority;
            vThreads.push_back(td);
        }
        break;
    }
    return !vThreads.empty();
}