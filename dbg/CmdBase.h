#ifndef CMDBASE_VDEBUG_H_H_
#define CMDBASE_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <DbgCtrl/DbgCtrlCom.h>
#include <ComLib/ComLib.h>
#include "UserContext.h"

using namespace std;

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

typedef CtrlReply (* pfnCmdHandler)(const mstring &, HUserCtx);

class CCmdBase
{
public:
    CCmdBase();
    virtual ~CCmdBase();

    CtrlReply RunCommand(const mstring &request, HUserCtx ctx);

    BOOL InsertFunMsg(const mstring &strIndex, const DbgFunInfo &vProcInfo);
    //eg: kernel32!createfilew+0x1234
    static DWORD64 GetFunAddr(const mstring &wstr);
    bool IsCommand(const mstring &command) const;
public:
    static bool IsNumber(const mstring &str);
    static bool IsKeyword(const mstring &str);
    static vector<WordNode> GetWordSet(const mstring &strStr);
    static BOOL GetNumFromStr(const mstring &strNumber, DWORD64 &dwNumber);

protected:
    static bool IsFilterStr(mstring &strData, mstring &strFilter);
    static bool IsHightStr(mstring &strData, mstring &strHight);
    static DWORD64 GetSizeAndParam(const mstring &strParam, mstring &strOut);

    virtual CtrlReply OnCommand(const mstring &strCmd, const mstring &strCmdParam, HUserCtx ctx);
    void RegisterHandler(const mstring &cmd, pfnCmdHandler handler);
private:
    map<mstring, pfnCmdHandler> mCmdHandler;
};
#endif