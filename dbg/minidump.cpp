#include <Windows.h>
#include <DbgHelp.h>
#include "minidump.h"
//#include "common.h"
#include "symbol.h"

#ifndef _WIN64
typedef struct DECLSPEC_ALIGN(16) _M128A {
    ULONGLONG Low;
    LONGLONG High;
} M128A, *PM128A;

typedef struct _XMM_SAVE_AREA32 {
    WORD   ControlWord;
    WORD   StatusWord;
    BYTE  TagWord;
    BYTE  Reserved1;
    WORD   ErrorOpcode;
    DWORD ErrorOffset;
    WORD   ErrorSelector;
    WORD   Reserved2;
    DWORD DataOffset;
    WORD   DataSelector;
    WORD   Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    BYTE  Reserved4[96];
} XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;
#endif

typedef struct DECLSPEC_ALIGN(16) _CONTEXTx64 {

    //
    // Register parameter home addresses.
    //
    // N.B. These fields are for convience - they could be used to extend the
    //      context record in the future.
    //
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;

    //
    // Control flags.
    //
    DWORD ContextFlags;
    DWORD MxCsr;

    //
    // Segment Registers and processor flags.
    //
    WORD   SegCs;
    WORD   SegDs;
    WORD   SegEs;
    WORD   SegFs;
    WORD   SegGs;
    WORD   SegSs;
    DWORD EFlags;

    //
    // Debug registers
    //
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;

    //
    // Integer registers.
    //
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;

    //
    // Program counter.
    //

    DWORD64 Rip;

    //
    // Floating point state.
    //
    union {
        XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        };
    };

    //
    // Vector registers.
    //
    M128A VectorRegister[26];
    DWORD64 VectorControl;

    //
    // Special debug control registers.
    //
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
} CONTEXTx64, *PCONTEXTx64;

CMiniDumpHlpr::CMiniDumpHlpr()
{}

CMiniDumpHlpr::~CMiniDumpHlpr()
{}

bool CMiniDumpHlpr::WriteDump(const mstring &strFilePath) const
{
    return true;
}

bool CMiniDumpHlpr::LodeDump(const mstring &strFilePath)
{
    m_pDumpFile = MappingFileA(strFilePath.c_str(), FALSE, 0);
    return (m_pDumpFile && m_pDumpFile->lpView);
}

bool CMiniDumpHlpr::GetModuleSet(list<DumpModuleInfo> &vModules)
{
    DWORD dwStreamSize = 0;
    LPVOID pStream = GetSpecStream(ModuleListStream, dwStreamSize);

    if (!pStream)
    {
        return false;
    }

    PMINIDUMP_MODULE_LIST pModuleList = (PMINIDUMP_MODULE_LIST)pStream;
    DumpModuleInfo info;
    for (int i = 0 ; i < (int)pModuleList->NumberOfModules ; i++)
    {
        PMINIDUMP_MODULE pModule = pModuleList->Modules + i;
        info.m_dwBaseAddr = pModule->BaseOfImage;
        info.m_dwModuleSize = pModule->SizeOfImage;
        PMINIDUMP_STRING pStr= (PMINIDUMP_STRING)((const char *)m_pDumpFile->lpView + pModule->ModuleNameRva);

        info.m_strModulePath.append(WtoA(ustring(pStr->Buffer, pStr->Length / sizeof(WCHAR))));
        info.m_dwTimeStamp = pModule->TimeDateStamp;
        info.m_strModuleName = PathFindFileNameA(info.m_strModulePath.c_str());

        //在本地获取dump中的文件路径，可能会从微软服务器上下载
        CTaskFindFileForDump task;
        CSymbolTaskHeader header;
        header.m_dwSize = sizeof(CTaskFindFileForDump) + sizeof(CSymbolTaskHeader);
        header.m_eTaskType = em_task_findfile;

        task.m_SizeofImage = (PVOID)pModule->SizeOfImage;
        task.m_TimeStamp = (PVOID)pModule->TimeDateStamp;
        task.m_strModuleName = info.m_strModuleName;
        header.m_pParam = &task;

        if (GetSymbolHlpr()->SendTask(&header))
        {
            info.m_strModulePath = task.m_strFullPath;
        }
    }
    return true;
}

bool CMiniDumpHlpr::GetThreadSet(list<DumpThreadInfo> &vThreads) const
{
    DWORD dwStreamSize = 0;
    LPVOID pStream = GetSpecStream(ThreadListStream, dwStreamSize);

    if (!pStream)
    {
        return false;
    }

    LPVOID ptr = m_pDumpFile->lpView;
    int d = (int)ptr;

    PMINIDUMP_THREAD_LIST pThreadList = (PMINIDUMP_THREAD_LIST)pStream;
    for (int i = 0 ; i < (int)pThreadList->NumberOfThreads ; i++)
    {
        MINIDUMP_THREAD *pThread = (pThreadList->Threads + i);
        CONTEXTx64 *pContext = (CONTEXTx64 *)((const char *)m_pDumpFile->lpView + pThread->ThreadContext.Rva);
        DumpMemoryInfo stack;
        stack.m_dwStartAddr = pThread->Stack.StartOfMemoryRange;
        stack.m_dwRva = pThread->Stack.Memory.Rva;
        stack.m_dwSize = pThread->Stack.Memory.DataSize;

        DumpThreadInfo info;
        info.m_context.m_dwCbp = pContext->Rbp;
        info.m_context.m_dwCsi = pContext->Rsi;
        info.m_context.m_dwCsp = pContext->Rsp;
        info.m_context.m_dwCip = pContext->Rip;
        info.m_stack = stack;
        info.m_dwThreadId = pThread->ThreadId;
        vThreads.push_back(info);
    }
    return true;
}

bool CMiniDumpHlpr::GetSystemInfo(DumpSystemInfo &vSystem) const
{
    DWORD dwStreamSize = 0;
    LPVOID pStream = GetSpecStream(SystemInfoStream, dwStreamSize);

    if (!pStream)
    {
        return false;
    }
    MINIDUMP_SYSTEM_INFO *pSysStream = (MINIDUMP_SYSTEM_INFO *)pStream;

    DumpSystemInfo system;
    PMINIDUMP_STRING pWstr = (PMINIDUMP_STRING)((const char *)m_pDumpFile->lpView + pSysStream->CSDVersionRva);
    system.m_strSpStr.append(WtoA(ustring(pWstr->Buffer, pWstr->Length / sizeof(WCHAR))));
    return true;
}

bool CMiniDumpHlpr::GetMemoryInfo(list<DumpMemoryInfo> &vMemory) const
{
    DWORD dwStreamSize = 0;
    LPVOID pStream = GetSpecStream(MemoryListStream , dwStreamSize);

    if (!pStream)
    {
        return false;
    }

    MINIDUMP_MEMORY_LIST *pMemList = (MINIDUMP_MEMORY_LIST *)pStream;
    for (int i = 0 ; i < (int)pMemList->NumberOfMemoryRanges ; i++)
    {
        DumpMemoryInfo info;
        MINIDUMP_MEMORY_DESCRIPTOR  *pMemDesc = pMemList->MemoryRanges + i;
        info.m_dwStartAddr = pMemDesc->StartOfMemoryRange;
        info.m_dwSize = pMemDesc->Memory.DataSize;
        info.m_dwRva = pMemDesc->Memory.Rva;
        vMemory.push_back(info);
    }
    return true;
}

DWORD64 CMiniDumpHlpr::GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr)
{
    for (list<DumpModuleInfo>::const_iterator it = GetMiniDumpHlpr()->m_vModuleSet.begin() ; it != GetMiniDumpHlpr()->m_vModuleSet.end() ; it++)
    {
        if (dwAddr >= it->m_dwBaseAddr && dwAddr <= (it->m_dwBaseAddr + it->m_dwModuleSize))
        {
            return it->m_dwBaseAddr;
        }
    }
    return 0;
}

DWORD64 CALLBACK StackTranslateAddressProc64(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr)
{
    return 0;
}

BOOL CMiniDumpHlpr::ReadDumpMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    //将读取的内存地址转换为RVA地址并进行读取
    DumpMemoryInfo *pStack = (DumpMemoryInfo *)hProcess;
    GetMiniDumpHlpr()->m_vThreadSet;
    DWORD64 d = (DWORD64)pStack;

    for (list<DumpMemoryInfo>::const_iterator it = GetMiniDumpHlpr()->m_vMemory.begin() ; it != GetMiniDumpHlpr()->m_vMemory.end() ; it++)
    {
        if (lpBaseAddress >= it->m_dwStartAddr && lpBaseAddress <= it->m_dwStartAddr + it->m_dwSize)
        {
            DWORD64 dw =(it->m_dwRva + (DWORD64)GetMiniDumpHlpr()->m_pDumpFile->lpView);
            DWORD64 dwOffset = (lpBaseAddress - it->m_dwStartAddr);
            DWORD64 dwDataAddr = dw +dwOffset;
            memcpy(lpBuffer, (void *)dwDataAddr, nSize);
            lpNumberOfBytesRead[0] = nSize;
            return TRUE;
        }
    }

    //if (lpBaseAddress >= pStack->m_dwStartAddr && lpBaseAddress <= pStack->m_dwStartAddr + pStack->m_dwSize)
    //{
    //    return TRUE;
    //}
    return FALSE;
}

bool CMiniDumpHlpr::GetCallStack()
{
    GetModuleSet(m_vModuleSet);
    list<DumpThreadInfo> threads;
    GetThreadSet(threads);
    DumpThreadInfo th1 = *threads.begin();
    GetMemoryInfo(m_vMemory);

    STACKFRAME64 frame = {0};
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = th1.m_context.m_dwCip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = th1.m_context.m_dwCbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = th1.m_context.m_dwCsp;
    frame.AddrStack.Mode = AddrModeFlat;

    CTaskStackWalkInfo info;
    info.m_context = frame;
    info.m_dwMachineType = IMAGE_FILE_MACHINE_I386;
    info.m_pfnGetModuleBaseProc = GetModuelBaseFromAddr;
    info.m_pfnReadMemoryProc = ReadDumpMemoryProc64;

    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskStackWalkInfo) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_stackwalk;
    header.m_pParam = &info;
    GetSymbolHlpr()->SendTask(&header);
    return true;
}

LPVOID CMiniDumpHlpr::GetSpecStream(MINIDUMP_STREAM_TYPE eType, DWORD &dwStreamSize) const
{
    if (!m_pDumpFile || !m_pDumpFile->lpView)
    {
        return false;
    }

    MINIDUMP_DIRECTORY *pDumpDir = NULL;
    PVOID pStreamAddr = NULL;
    MiniDumpReadDumpStream(m_pDumpFile->lpView, eType, &pDumpDir, &pStreamAddr, &dwStreamSize);

    if (!dwStreamSize)
    {
        return NULL;
    }
    return pStreamAddr;
}

CMiniDumpHlpr *GetMiniDumpHlpr()
{
    static CMiniDumpHlpr *s_ptr = new CMiniDumpHlpr();
    return s_ptr;
}