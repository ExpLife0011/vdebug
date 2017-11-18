#ifndef SYMBOL_VDEBUG_H_H_
#define SYMBOL_VDEBUG_H_H_
#include <Windows.h>
#include <list>
#include "mstring.h"
#include "LockBase.h"
#include "Debugger.h"
#include "CmdBase.h"

using namespace std;

typedef BOOL (CALLBACK *pfnReadDumpMemoryProc64)(
    HANDLE hProcess,
    DWORD64 lpBaseAddress,
    PVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

typedef DWORD64 (CALLBACK *pfnGetModuelBaseFromAddr)(HANDLE hProcess, DWORD64 dwAddr);

enum SymbolTask
{
    em_task_loadsym,    //符号加载
    em_task_symaddr,    //
    em_task_setpath,
    em_task_stackwalk,
    em_task_findfile    //在本地下载dump中的模块
};

struct CTaskLoadSymbol
{
    HANDLE m_hImgaeFile;        //IN
    DWORD64 m_dwBaseOfModule;   //IN

    ModuleInfo m_ModuleInfo;    //OUT
    CCmdBase *m_pCmdEngine;     //OUT
};

struct CTaskSymbolFromAddr
{
    DWORD64 m_dwAddr;           //IN
    ustring m_wstrSymbol;       //OUT
};

struct CTaskSetSymbolPath
{
    ustring m_wstrSymbolPath;   //IN
};

struct CTaskStackWalkInfo
{
    CTaskStackWalkInfo()
    {
        m_dwMachineType = IMAGE_FILE_MACHINE_I386;
        m_hDstProcess = NULL;
        m_pfnReadMemoryProc = NULL;
        m_pfnGetModuleBaseProc = NULL;
    }

    DWORD m_dwMachineType;      //IN
    STACKFRAME64 m_context;     //IN
    HANDLE m_hDstProcess;       //IN
    pfnReadDumpMemoryProc64 m_pfnReadMemoryProc;    //IN
    pfnGetModuelBaseFromAddr m_pfnGetModuleBaseProc;//IN

    list<STACKFRAME64> m_FrameSet;  //OUT
};

struct CTaskFindFileForDump
{
    CTaskFindFileForDump()
    {
        m_TimeStamp = NULL;
        m_SizeofImage = NULL;
    }

    ustring m_wstrModuleName;   //IN 模块名称
    PVOID m_TimeStamp;          //IN 时间戳
    PVOID m_SizeofImage;        //IN 文件大小

    ustring m_wstrFullPath;     //OUT 本地模块路径
};

struct CSymbolTaskHeader
{
    CSymbolTaskHeader() : m_dwSize(0), m_hNotify(NULL), m_pParam(NULL), m_bSucc(FALSE)
    {
    }

    DWORD m_dwSize;
    HANDLE m_hNotify;
    SymbolTask m_eTaskType;
    BOOL m_bSucc;
    LPVOID m_pParam;
};

class CSymbolHlpr : public CCriticalSectionLockable
{
public:
    CSymbolHlpr(const ustring &wstrSymboPath);
    virtual ~CSymbolHlpr();

    bool SetSymbolPath(const ustring &wstrSymbolPath);
    bool SendTask(CSymbolTaskHeader *pTask, BOOL bAtOnce = TRUE);
    bool PostTask(CSymbolTaskHeader *pTask, BOOL bAtOnce = TRUE);
    bool DeleteTask(CSymbolTaskHeader *pTask);

protected:
    static bool FindFileForDump(CTaskFindFileForDump *pFileInfo);
    static bool StackWalk(CTaskStackWalkInfo *pStackWalkInfo);
    static bool SetSymbolPath(CTaskSetSymbolPath *pSymbolPath);
    static bool GetSymbolFromAddr(CTaskSymbolFromAddr *pSymbolInfo);
    static BOOL CALLBACK EnumSymbolProc(PSYMBOL_INFOW pSymInfo, ULONG uSymbolSize, PVOID pUserContext);
    static bool LoadSymbol(CTaskLoadSymbol *pModuleInfo);
    static bool DoTask(CSymbolTaskHeader *pTask);
    static DWORD WINAPI WorkThread(LPVOID pParam);

protected:
    list<CSymbolTaskHeader *> m_vTaskQueue;
    HANDLE m_hNotifyEvent;
    HANDLE m_hInitEvent;
    ustring m_wstrSymbolPath;
};

bool InitSymbolHlpr(const ustring &wtrSymbolPath);

CSymbolHlpr *GetSymbolHlpr();
#endif