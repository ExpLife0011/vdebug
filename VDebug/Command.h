#ifndef COMMAND_VDEBUG_H_H_
#define COMMAND_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include "mstring.h"
#include "SyntaxView.h"
#include "LockBase.h"

using namespace std;

enum CmdStatus
{
    em_status_succ,
    em_status_notfound,
    em_status_faild
};

struct CommandResult
{
    ustring m_wstrResult;       //Json格式的返回结果
    SyntaxDesc m_vSyntaxDesc;   //
    CmdStatus m_eStatus;

    CommandResult()
    {
        m_eStatus = em_status_notfound;
        m_wstrResult = L"非内置命令";
    }

    CommandResult(CmdStatus eStatus, const ustring &wstrResult)
    {
        m_eStatus = eStatus;
        m_wstrResult = wstrResult;
    }
};

struct CmdUserContext;

typedef CommandResult (WINAPI *pfnCmdCallbackProc)(const ustring &wstrParam, BOOL bShow, CmdUserContext *pUserCountext); //cmd注册回调，用于处理用户的cmd输入，所有的命令都必须注册该回调处理
typedef void (WINAPI *pfnCmdNotifyCallback)(LPVOID pParam, LPVOID pContext);   //命令回调函数，执行RunCmd时使用，执行的命令出发后调用（如果需要），eg：bp，gu等，选填

struct CmdUserContext
{
    LPVOID m_pUserParam;
    pfnCmdNotifyCallback m_pfn;

    CmdUserContext() : m_pUserParam(NULL), m_pfn(NULL)
    {}

    bool operator==(const CmdUserContext &other) const
    {
        return true;
    }
};

class CCmdEngine : public CCriticalSectionLockable
{
public:
    BOOL CmdRegister(const ustring &wstrCmd, pfnCmdCallbackProc pfnCmdProc);

    BOOL CmdUnRegister(const ustring &wstrCmd);

    CommandResult RunCmd(const ustring &wstrCmd, LPVOID pUerParam = NULL, BOOL bShow = TRUE, pfnCmdNotifyCallback pfn = NULL);

    BOOL CmdIsValid(const ustring &wstrCmd);

    VOID CmdInit();

    VOID OnBreakPoint(DWORD64 dwBreakPointAddr);

    BOOL AddProcMsg(const ustring &wstrIdex, DWORD64 dwAddr);

    DWORD64 GetAddrFromProcStr(const ustring &wstrProc) const;
protected:
    static DWORD64 CALLBACK GetModuelBaseFromAddr(HANDLE hProcess, DWORD64 dwAddr);

    static BOOL GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwNumber);

    static DWORD64 GetAddrFormStr(const ustring &wstrStr);  //由表达式获取内存地址

protected:
    static CommandResult WINAPI CmdRegProc(const ustring &pCmdParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdDisassProc(const ustring &pParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdGo(const ustring &pCmdParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdBp(const ustring &pCmdParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdKv(const ustring &pCmdParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdDu(const ustring &pCmdParam, BOOL bShow, CmdUserContext *pUserCountext = NULL);
    static CommandResult WINAPI CmdScript(const ustring &wstrCmdParam, BOOL bShow, CmdUserContext *pUserContext = NULL);

protected:
    map<ustring, pfnCmdCallbackProc> m_vCmdCallback;

    //此处可能会导致进程退出的时候异常
    static map<ustring, DWORD64> ms_vProcMap;
};

CCmdEngine *GetCmdEngine();

#endif
