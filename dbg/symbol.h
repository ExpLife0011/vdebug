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
    em_task_initsymbol, //��ʼ����������
    em_task_loadsym,    //���ż���
    em_task_strfromaddr,//��ȡ��ַ��Ӧ���ַ�������
    em_task_addrfromstr,//��ȡ�ַ������ƶ�Ӧ�ķ��ŵ�ַ
    em_task_setpath,    //���÷��ŵ�ַ
    em_task_stackwalk,  //ջ��˷
    em_task_findfile,   //�ڱ�������dump�е�ģ��
    em_task_unloadsym,  //ж��ָ������
    em_task_unloadall   //ж�����з���
};

struct CTaskSymbolInit  //��ʼ������
{
    HANDLE m_hDstProc;  //Ŀ�����
    ustring m_wstrSearchPath;

    CTaskSymbolInit()
    {
        m_hDstProc = NULL;
    }
};

struct SymbolLoadInfo   //�Ѽ���ģ��ķ�����Ϣ
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
    CONTEXT *m_pThreadContext;  //IN 64λʹ��
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

    ustring m_wstrModuleName;   //IN ģ������
    PVOID m_TimeStamp;          //IN ʱ���
    PVOID m_SizeofImage;        //IN �ļ���С

    ustring m_wstrFullPath;     //OUT ����ģ��·��
};

struct CTaskUnloadSymbol
{
    DWORD64 m_dwModuleAddr;     //ж��ģ���ַ
};

struct CTaskGetAddrFromStr
{
    ustring m_wstrStr;          //��ȡ��ַ���ַ��� eg:createfilew kernelbase!createfilew
    DWORD64 m_dwBaseOfModule;   //ģ���ַ

    DWORD64 m_dwAddr;           //OUT ����ķ������ڵ�ַ

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
    //�����Ƿ��Ѿ�������
    bool IsSymbolLoaded(const ustring &wstrDll, DWORD64 dwBaseOfModule);
    //���¼���ָ��ģ��
    bool ReloadModule(const ustring &wstrDll, DWORD64 dwBaseOfModule);
    //ж��ָ��ģ�����
    bool UnLoadModuleByName(const ustring &wstrDllName);
    //ж��ָ��ģ�����
    bool UnLoadModuleByAddr(DWORD64 dwAddr);
    //���ط���
    bool LoadModule(HANDLE hFile, const ustring &wstrDllPath, DWORD64 dwBaseOfModule);
    //ж�ص�ǰ���е�ģ��
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