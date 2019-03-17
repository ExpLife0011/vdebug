#ifndef MINIDUMP_VDEBUG_H_H_
#define MINIDUMP_VDEBUG_H_H_
#include <Windows.h>
#include <DbgHelp.h>
#include <list>
#include <vector>
#include <ComLib/ComLib.h>
#include <ComLib/ComLib.h>

using namespace std;

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

struct DumpContext
{
    DWORD64 m_dwCsp;
    DWORD64 m_dwCsi;
    DWORD64 m_dwCbp;
    DWORD64 m_dwCip;
    CONTEXTx64 *mFullContext;

    DumpContext() {
        memset(this, 0x00, sizeof(DumpContext));
    }
};

struct DumpModuleInfo
{
    mstring m_strModuleName;
    mstring m_strModulePath;
    DWORD64 m_dwModuleSize;
    DWORD64 m_dwBaseAddr;
    DWORD64 m_dwTimeStamp;
    mstring mTimeStr;
    mstring mVersion;
    BOOL mLoadSymbol;
    BOOL mLoadFaild;

    DumpModuleInfo() {
        m_dwBaseAddr = 0;
        m_dwModuleSize = 0;
        m_dwTimeStamp = 0;
        mLoadSymbol = FALSE;
        mLoadFaild = FALSE;
    }
};

enum ProcessorType
{
    em_processor_amd64,
    em_processor_ia64,
    em_processor_x86,
    em_processor_unknown
};

struct DumpMemoryInfo
{
    DWORD64 m_dwStartAddr; //位于生成dump文件进程的内存地址
    DWORD64 m_dwRva;       //相对于dump映射基址的偏移
    DWORD64 m_dwSize;      //该内存块的大小
};

struct DumpThreadInfo
{
    DWORD mIndex;
    DWORD m_dwThreadId;
    DWORD m_dwSuspendCount;
    DWORD64 m_dwTeb;
    DumpContext m_context;
    DumpMemoryInfo m_stack;
    std::mstring mCipSymbol;

    DumpThreadInfo() {
        mIndex = 0;
        m_dwThreadId = 0;
        m_dwSuspendCount = 0;
        m_dwTeb = 0;
    }
};

struct DumpSystemInfo
{
    ProcessorType m_eCpuType;
    DWORD m_dwMajVer;
    DWORD m_dwMinVer;
    DWORD m_dwBuildNum;
    DWORD mProductType;
    mstring m_strSpStr;

    DumpSystemInfo() {
        m_eCpuType = em_processor_x86;
        m_dwMajVer = 0;
        m_dwMinVer = 0;
        m_dwBuildNum = 0;
    }
};

struct DumpException {
    DWORD mExceptionCode;
    DWORD mExceptionFlags;
    DWORD64 mExceptionAddress;
    DWORD mThreadId;
    CONTEXTx64 mThreadContext;
    mstring mSymbol;

    DumpException() {
        mExceptionCode = 0;
        mExceptionFlags = 0;
        mExceptionAddress = 0;
        mThreadId = 0;
        memset(&mThreadContext, 0x00, sizeof(mThreadContext));
    }
};

class CMiniDumpHlpr
{
private:
    CMiniDumpHlpr();
public:
    static CMiniDumpHlpr *GetInst();
    virtual ~CMiniDumpHlpr();
    bool WriteDump(const mstring &strFilePath) const;
    bool LodeDump(const mstring &strFilePath);
    list<DumpModuleInfo> GetModuleSet() const;
    vector<DumpThreadInfo> GetThreadSet() const;
    DumpThreadInfo GetCurThread() const;
    list<DumpMemoryInfo> GetMemoryInfo() const;
    DumpSystemInfo GetSystemInfo() const;
    bool GetCallStack();
    DumpException GetException();
    bool SetCurThread(DWORD tid);
    list<STACKFRAME64> GetStackFrame(int tid);
    list<STACKFRAME64> GetCurrentStackFrame();
    DumpThreadInfo GetThreadByTid(int tid) const;
    //获取dump符号信息，dump和普通的进程调试不同，dump可能需要获取本地没有的pe文件的符号，需要特殊处理
    DumpModuleInfo GetModuleFromAddr(DWORD64 addr) const;
    bool LoadSymbolByAddr(DWORD64 addr);

protected:
    bool LoadSystemInfo();
    bool LoadModuleSet();
    bool LoadThreadSet();
    bool LoadMemorySet();
    bool LoadException();
    bool LoadSymbolFromImage(const mstring &image, DWORD64 baseAddr) const;
    void LoadAllSymbol();
    bool LoadSymbol(DWORD64 baseAddr);
    static DWORD64 CALLBACK GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr);
    static BOOL CALLBACK ReadDumpMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);
    LPVOID GetSpecStream(MINIDUMP_STREAM_TYPE eType, DWORD &dwStreamSize) const;
    mstring GetVersionStr(const VS_FIXEDFILEINFO &version) const;
    bool ResetStat();

protected:
    list<DumpModuleInfo> m_vModuleSet;
    vector<DumpThreadInfo> m_vThreadSet;
    list<DumpMemoryInfo> m_vMemory;
    DumpException mException;
    DumpSystemInfo mSystemInfo;
    PFILE_MAPPING_STRUCT m_pDumpFile;
    DumpThreadInfo mCurThread;
};
#endif