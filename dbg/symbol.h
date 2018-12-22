#ifndef SYMBOL_VDEBUG_H_H_
#define SYMBOL_VDEBUG_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include "CmdBase.h"
#include "DbgBase.h"

using namespace std;

typedef BOOL (CALLBACK *pfnReadDumpMemoryProc64)(
    HANDLE hProcess,
    DWORD64 lpBaseAddress,
    PVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

typedef DWORD64 (CALLBACK *pfnGetModuelBaseFromAddr)(HANDLE hProcess, DWORD64 dwAddr);

typedef DWORD64 (CALLBACK *pfnStackTranslateAddressProc64)(HANDLE hProcess, HANDLE hThread, LPADDRESS64 lpaddr);

enum SymbolTask
{
    em_task_initsymbol, //初始化符号引擎
    em_task_loadsym,    //符号加载
    em_task_strfromaddr,//获取地址对应的字符串名称
    em_task_addrfromstr,//获取字符串名称对应的符号地址
    em_task_setpath,    //设置符号地址
    em_task_stackwalk,  //栈回朔
    em_task_findfile,   //在本地下载dump中的模块
    em_task_unloadsym,  //卸载指定符号
    em_task_unloadall   //卸载所有符号
};

struct CTaskSymbolInit  //初始化符号
{
    HANDLE m_hDstProc;  //目标进程
    ustring m_wstrSearchPath;

    CTaskSymbolInit()
    {
        m_hDstProc = NULL;
    }
};

struct SymbolLoadInfo   //已加载模块的符号信息
{
    ustring m_wstrModulePath;
    ustring m_wstrMuduleName;
    DWORD64 m_dwBaseOfModule;
};

struct CTaskLoadSymbol
{
    HANDLE m_hImgaeFile;        //IN
    DWORD64 m_dwBaseOfModule;   //IN

    DbgModuleInfo m_ModuleInfo; //OUT
    CCmdBase *m_pCmdEngine;     //OUT
};

struct CTaskSymbolFromAddr
{
    DbgModuleInfo m_ModuleInfo; //IN
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
        m_hDstThread = NULL;
        m_pfnReadMemoryProc = NULL;
        m_pfnGetModuleBaseProc = NULL;
        m_pfnStackTranslateProc = NULL;
    }

    DWORD m_dwMachineType;      //IN
    STACKFRAME64 m_context;     //IN
    HANDLE m_hDstProcess;       //IN
    HANDLE m_hDstThread;        //IN
    CONTEXT *m_pThreadContext;  //IN 64位使用
    pfnReadDumpMemoryProc64 m_pfnReadMemoryProc;    //IN
    pfnGetModuelBaseFromAddr m_pfnGetModuleBaseProc;//IN
    pfnStackTranslateAddressProc64 m_pfnStackTranslateProc; //IN

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

struct CTaskUnloadSymbol
{
    DWORD64 m_dwModuleAddr;     //卸载模块地址
};

struct CTaskGetAddrFromStr
{
    ustring m_wstrStr;          //获取地址的字符串 eg:createfilew kernelbase!createfilew
    DWORD64 m_dwBaseOfModule;   //模块基址

    DWORD64 m_dwAddr;           //OUT 具体的符号所在地址

    CTaskGetAddrFromStr()
    {
        m_dwBaseOfModule = 0;
        m_dwAddr = 0;
    }
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
    static bool InitEngine(CTaskSymbolInit *pInitInfo);
    static bool UnloadAllModuleTask(LPVOID pParam);
    static bool UnloadModuleTask(CTaskUnloadSymbol *pUnloadInfo);
    static bool FindFileForDump(CTaskFindFileForDump *pFileInfo);
    static bool StackWalk(CTaskStackWalkInfo *pStackWalkInfo);
    static bool SetSymbolPath(CTaskSetSymbolPath *pSymbolPath);
    static bool GetSymbolFromAddr(CTaskSymbolFromAddr *pSymbolInfo);
    static bool GetAddrFromStr(CTaskGetAddrFromStr *pSymbolInfo);
    static bool LoadSymbol(CTaskLoadSymbol *pModuleInfo);
    static bool DoTask(CSymbolTaskHeader *pTask);
    static DWORD WINAPI WorkThread(LPVOID pParam);

protected:
    //符号是否已经被加载
    bool IsSymbolLoaded(const ustring &wstrDll, DWORD64 dwBaseOfModule);
    //重新加载指定模块
    bool ReloadModule(const ustring &wstrDll, DWORD64 dwBaseOfModule);
    //卸载指定模块符号
    bool UnLoadModuleByName(const ustring &wstrDllName);
    //卸载指定模块符号
    bool UnLoadModuleByAddr(DWORD64 dwAddr);
    //加载符号
    bool LoadModule(HANDLE hFile, const ustring &wstrDllPath, DWORD64 dwBaseOfModule);
    //卸载当前所有的模块
    bool UnloadAllModules();

protected:
    list<SymbolLoadInfo> m_vSymbolInfo;
    list<CSymbolTaskHeader *> m_vTaskQueue;
    HANDLE m_hNotifyEvent;
    HANDLE m_hInitEvent;
    HANDLE m_hDbgProc;
    ustring m_wstrSymbolPath;
};

bool InitSymbolHlpr(const ustring &wtrSymbolPath);

CSymbolHlpr *GetSymbolHlpr();
#endif