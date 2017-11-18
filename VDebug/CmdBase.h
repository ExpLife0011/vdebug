#ifndef CMDBASE_VDEBUG_H_H_
#define CMDBASE_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include "mstring.h"

using namespace std;

typedef void (WINAPI *pfnUserNotifyCallback)(LPVOID pParam, LPVOID pContext);

struct CmdUserParam
{
    pfnUserNotifyCallback m_pfnCallback;
    LPVOID m_pParam;

    CmdUserParam() : m_pParam(NULL), m_pfnCallback(NULL)
    {}

    bool operator==(const CmdUserParam &other) const
    {
        return true;
    }
};

enum DbgCmdStatus
{
    em_dbgstat_succ,
    em_dbgstat_notfound,
    em_dbgstat_faild
};

struct DbgCmdResult
{
    ustring m_wstrResult;       //Json格式的返回结果
    DbgCmdStatus m_eStatus;

    DbgCmdResult()
    {
        m_eStatus = em_dbgstat_notfound;
        m_wstrResult = L"非内置命令";
    }

    DbgCmdResult(DbgCmdStatus eStatus, const ustring &wstrResult)
    {
        m_eStatus = eStatus;
        m_wstrResult = wstrResult;
    }
};

class CCmdBase
{
public:
    CCmdBase();
    virtual ~CCmdBase();
    DbgCmdResult RunCommand(const ustring &wstrCmd, BOOL bShow = TRUE, const CmdUserParam *pParam = NULL);
    BOOL AddProcMsg(const ustring &wstrIdex, DWORD64 dwAddr);

    //Tools
protected:
    static BOOL GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwNumber);

protected:
    virtual DbgCmdResult OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);

protected:
    map<ustring, DWORD64> m_vProcMap;
};
#endif