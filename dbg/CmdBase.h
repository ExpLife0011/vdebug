#ifndef CMDBASE_VDEBUG_H_H_
#define CMDBASE_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <ComStatic/ComStatic.h>

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

struct DbgFunInfo
{
    mstring m_strModule;
    mstring m_strFunName;
    DWORD64 m_dwModuleBase;
    DWORD64 m_dwProcOffset;
    DWORD64 m_dwProcAddr;
};

struct WordNode
{
    size_t m_iStartPos;
    size_t m_iLength;
    mstring m_strContent;
};

class CCmdBase
{
public:
    CCmdBase();
    virtual ~CCmdBase();
    mstring RunCommand(const mstring &strCmd, const CmdUserParam *pParam = NULL);
    BOOL InsertFunMsg(const mstring &strIndex, const DbgFunInfo &vProcInfo);
    //eg: kernel32!createfilew+0x1234
    DWORD64 GetFunAddr(const mstring &wstr);

    //Tools
public:
    bool IsNumber(const mstring &str) const;
    bool IsKeyword(const mstring &str) const;
    vector<WordNode> GetWordSet(const mstring &strStr) const;
    BOOL GetNumFromStr(const mstring &strNumber, DWORD64 &dwNumber) const;

protected:
    bool IsFilterStr(mstring &strData, mstring &strFilter) const;
    //bool OnFilter(SyntaxDesc &desc, const mstring &strFilter) const;
    bool IsHightStr(mstring &strData, mstring &strHight) const;
    //bool OnHight(SyntaxDesc &desc, const mstring &strHight) const;
    DWORD64 GetSizeAndParam(const mstring &strParam, mstring &strOut) const;

protected:
    virtual mstring OnCommand(const mstring &strCmd, const mstring &strCmdParam, const CmdUserParam *pParam);
};
#endif