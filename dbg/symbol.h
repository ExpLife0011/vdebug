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
    mstring m_wstrSearchPath;

    CTaskSymbolInit()
    {
        m_hDstProc = NULL;
    }
};

struct SymbolLoadInfo   //�Ѽ���ģ��ķ�����Ϣ
{
    mstring m_strModulePath;
    mstring m_strMuduleName;
    DWORD64 m_dwBaseOfModule;
    DWORD64 m_dwModuleSize;

    SymbolLoadInfo() {
        m_dwBaseOfModule = 0;
        m_dwModuleSize = 0;
    }
};

struct CTaskLoadSymbol
{
    HANDLE m_hImgaeFile;        //IN
    DWORD64 m_dwBaseOfModule;   //IN

    DbgModuleInfo m_ModuleInfo; //OUT
};

struct CTaskSymbolFromAddr
{
    DWORD64 mAddr;           //IN
    mstring mDllName;        //DLL
    mstring mSymbol;         //OUT

    mstring mFilePath;       //source path
    DWORD mLineNumber;       //source line

    CTaskSymbolFromAddr() {
        mAddr = 0;
        mLineNumber = 0;
    }
};

struct CTaskSetSymbolPath
{
    mstring m_wstrSymbolPath;   //IN
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

    mstring m_strModuleName;   //IN ģ������
    PVOID m_TimeStamp;          //IN ʱ���
    PVOID m_SizeofImage;        //IN �ļ���С

    mstring m_strFullPath;     //OUT ����ģ��·��
};

struct CTaskUnloadSymbol
{
    DWORD64 m_dwModuleAddr;     //ж��ģ���ַ
};

struct CTaskGetAddrFromStr
{
    mstring m_strStr;          //��ȡ��ַ���ַ��� eg:createfilew kernelbase!createfilew
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
private:
    CSymbolHlpr();
    virtual ~CSymbolHlpr();
public:
    static CSymbolHlpr *GetInst();
    bool InitSymbol(const std::mstring &strSymbolPath, HANDLE hProc);
    bool SetSymbolPath(const mstring &wstrSymbolPath);
    bool SendTask(CSymbolTaskHeader *pTask, BOOL bAtOnce = TRUE);
    bool PostTask(CSymbolTaskHeader *pTask, BOOL bAtOnce = TRUE);
    bool DeleteTask(CSymbolTaskHeader *pTask);
    bool UnloadAll();

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
    SymbolLoadInfo GetLoadInfoByAddr(DWORD64 addr) const;
    //�����Ƿ��Ѿ�������
    bool IsSymbolLoaded(const mstring &dllName, DWORD64 addr) const;
    //���¼���ָ��ģ��
    bool ReloadModule(const mstring &wstrDll, DWORD64 dwBaseOfModule);
    //ж��ָ��ģ�����
    bool UnLoadModuleByName(const mstring &strDllName);
    //ж��ָ��ģ�����
    bool UnLoadModuleByAddr(DWORD64 dwAddr);
    //���ط���
    bool LoadModule(HANDLE hFile, const mstring &strDllPath, DWORD64 dwBaseOfModule);
    //ж�ص�ǰ���е�ģ��
    bool UnloadAllModules();

protected:
    bool mInit;
    list<SymbolLoadInfo> m_vSymbolInfo;
    list<CSymbolTaskHeader *> m_vTaskQueue;
    HANDLE m_hNotifyEvent;
    HANDLE m_hInitEvent;
    HANDLE m_hDbgProc;
    mstring m_strSymbolPath;
};
#endif //SYMBOL_VDEBUG_H_H_