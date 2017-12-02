#ifndef CMDBASE_VDEBUG_H_H_
#define CMDBASE_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
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
    em_dbgstat_cmdnotfound,
    em_dbgstat_syntaxerr,
    em_dbgstat_memoryerr,
    em_dbgstat_faild
};

struct DbgCmdResult
{
    ustring m_wstrResult;       //Json格式的返回结果
    DbgCmdStatus m_eStatus;

    DbgCmdResult()
    {
        m_eStatus = em_dbgstat_cmdnotfound;
        m_wstrResult = L"非内置命令";
    }

    DbgCmdResult(DbgCmdStatus eStatus, const ustring &wstrResult)
    {
        m_eStatus = eStatus;
        m_wstrResult = wstrResult;
    }

    DbgCmdResult(DbgCmdStatus eStatus)
    {
        m_eStatus = eStatus;
    }
};

struct DbgFunInfo
{
    ustring m_wstrModule;
    ustring m_wstrFunName;
    DWORD64 m_dwModuleBase;
    DWORD64 m_dwProcOffset;
    DWORD64 m_dwProcAddr;
};

struct WordNode
{
    size_t m_iStartPos;
    size_t m_iLength;
    ustring m_wstrContent;
};

class CCmdBase
{
public:
    CCmdBase();
    virtual ~CCmdBase();
    DbgCmdResult RunCommand(const ustring &wstrCmd, BOOL bShow = TRUE, const CmdUserParam *pParam = NULL);
    BOOL InsertFunMsg(const ustring &wstrIndex, const DbgFunInfo &vProcInfo);
    //eg: kernel32!createfilew+0x1234
    DWORD64 GetFunAddr(const ustring &wstr);

    //Tools
public:
    bool IsNumber(const ustring &wstr) const;
    bool IsKeyword(const ustring &wstr) const;
    vector<WordNode> GetWordSet(const ustring &wstrStr) const;
    BOOL GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwNumber) const;

protected:
    DWORD64 GetSizeAndParam(const ustring &wstrParam, ustring &wstrOut) const;

protected:
    virtual DbgCmdResult OnCommand(const ustring &wstrCmd, const ustring &wstrCmdParam, BOOL bShow, const CmdUserParam *pParam);
};
#endif