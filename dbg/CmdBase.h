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

enum CmdDbgType {
    em_cmd_sync,    //同步的命令
    em_cmd_async    //异步的命令
};

//命令缓存
struct CmdHandlerInfo {
    mstring mCommand;           //调试命令名称
    CmdDbgType mCmdType;        //同步还是异步执行,先在只有断点命令是异步的
    pfnCmdHandler mHandler;     //命令执行函数
};

class CCmdBase : public CCriticalSectionLockable
{
public:
    CCmdBase();
    virtual ~CCmdBase();

    CtrlReply RunCommand(const mstring &request, HUserCtx ctx);

    BOOL InsertFunMsg(const mstring &strIndex, const DbgFunInfo &vProcInfo);
    //eg: kernel32!createfilew+0x1234
    static DWORD64 GetFunAddr(const mstring &wstr);
    bool IsCommand(const mstring &command, CmdHandlerInfo &cmdInfo) const;
public:
    static bool IsNumber(const mstring &str);
    static bool IsKeyword(const mstring &str);
    static vector<WordNode> GetWordSet(const mstring &strStr);
    static BOOL GetNumFromStr(const mstring &strNumber, DWORD64 &dwNumber);

protected:
    static bool IsFilterStr(mstring &strData, mstring &strFilter);
    static bool IsHightStr(mstring &strData, mstring &strHight);
    static DWORD64 GetSizeAndParam(const mstring &strParam, mstring &strOut);

    virtual CtrlReply OnCommand(const mstring &cmd, const mstring &param, HUserCtx ctx);
    void RegisterHandler(const mstring &cmd, CmdDbgType type, pfnCmdHandler handler);
private:
    map<mstring, CmdHandlerInfo> mCmdHandler;
};
#endif