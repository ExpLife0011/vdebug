#ifndef MINIDUMP_VDEBUG_H_H_
#define MINIDUMP_VDEBUG_H_H_
#include <Windows.h>
#include <DbgHelp.h>
#include <list>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>

using namespace std;

struct DumpContext
{
    DWORD64 m_dwCsp;
    DWORD64 m_dwCsi;
    DWORD64 m_dwCbp;
    DWORD64 m_dwCip;
};

struct DumpModuleInfo
{
    mstring m_strModuleName;
    mstring m_strModulePath;
    DWORD64 m_dwModuleSize;
    DWORD64 m_dwBaseAddr;
    DWORD64 m_dwTimeStamp;
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
    DWORD m_dwThreadId;
    DWORD m_dwSuspendCount;
    DWORD m_dwTeb;
    DumpContext m_context;
    DumpMemoryInfo m_stack;
};

struct DumpSystemInfo
{
    ProcessorType m_eCpuType;
    DWORD m_dwMajVer;
    DWORD m_dwMinVer;
    DWORD m_dwBuildNum;
    mstring m_strSpStr;
};

class CMiniDumpHlpr
{
public:
    CMiniDumpHlpr();

    virtual ~CMiniDumpHlpr();

    bool WriteDump(const mstring &strFilePath) const;

    bool LodeDump(const mstring &strFilePath);

    bool GetModuleSet(list<DumpModuleInfo> &vModules);

    bool GetThreadSet(list<DumpThreadInfo> &vThreads) const;

    bool GetSystemInfo(DumpSystemInfo &vSystem) const;

    bool GetMemoryInfo(list<DumpMemoryInfo> &vMemory) const;

    bool GetCallStack();
protected:
    static DWORD64 CALLBACK GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr);
    static BOOL CALLBACK ReadDumpMemoryProc64(HANDLE hProcess, DWORD64 lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);

    LPVOID GetSpecStream(MINIDUMP_STREAM_TYPE eType, DWORD &dwStreamSize) const;

protected:
    list<DumpModuleInfo> m_vModuleSet;
    list<DumpThreadInfo> m_vThreadSet;
    list<DumpMemoryInfo> m_vMemory;
    PFILE_MAPPING_STRUCT m_pDumpFile;
};

CMiniDumpHlpr *GetMiniDumpHlpr();
#endif