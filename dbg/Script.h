#ifndef SCRIPT_VDEBUG_H_H_
#define SCRIPT_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <ComStatic/ComStatic.h>
#include "TitanEngine/TitanEngine.h"
#include <ComLib/ComLib.h>
#include "DbgBase.h"

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
};

using namespace std;

enum ScriptOperator
{
    em_operator_add,
    em_operator_sub,
    em_operator_mult,
    em_operator_dev,
    em_operator_unknow
};

enum ScriptStrType
{
    em_scriptstr_proc,
    em_scriptstr_register
};

struct ScriptStrInfo
{
    ScriptStrType m_eType;
    DWORD64 m_dwAddr;
};

#define SCRIPT_RESULT_ERROR     (-1)
#define INVALID_SCRIPT_ADDR     (-1)

/*
脚本引擎
操作符 + - [] && || ()
*/
class CScriptEngine : public CCriticalSectionLockable
{
public:
    CScriptEngine();
    virtual ~CScriptEngine();
    void SetContext(TITAN_ENGINE_CONTEXT_t &vContext, pfnReadMemoryProc pfnRead = NULL, pfnWriteMemoryProc pfnWrite = NULL);

    //[esp + 4]
    //0xffabcd + 0x1234
    //0x123 + [esp + 4] + eax * (4 + 0x1122)
    DWORD64 Compile(const ustring &wstrScript) const;
    void InsertProc(LPCWSTR wszModule, LPCWSTR wszProc, DWORD64 dwAddr);
    //lua script
    BOOL RunScript(const ustring &wstrScript) const;
protected:
    BOOL IsOperator(WCHAR cLetter) const;

    BOOL IsRegisterStr(const ustring &wstr, DWORD64 &dwData) const;

    ustring GetSimpleResult2(const ustring &wstrScript) const;

    ustring GetSimpleResult1(const ustring &wstrScript) const;

    ustring GetPointerData(const ustring &wstrPointer) const;

    BOOL GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwResult) const;

    ustring GetTwoNumCalResult(const ustring &wstr1, const ustring &wstr2, ScriptOperator eOperator) const;

    ScriptOperator CScriptEngine::GetOperator(WCHAR cOperator) const;

    //计算并删除指定的操作符
    ustring DeleteOperator(const ustring &wstrScript, const ustring &wstrOpt) const;

    bool IsHexChar(WCHAR cLetter) const;

    DWORD64 GetAddrForStr(const ustring &wstr);

    ustring GetScriptPath(LPCWSTR wszScript) const;

    /*lua脚本*/
static void CmdNotifyCallback(LPVOID pUserParam, LPVOID pContext);

static void ScriptEnvInit(lua_State *pLuaStat);

static DWORD WINAPI ScriptExecThread(LPVOID pParam);

    /*提供给lua脚本使用的函数*/
static int DbgRunCmd(lua_State *pLuaStat);

static int DbgPrintMsg(lua_State *pLuaState);

static int DbgPrintErr(lua_State *pLuaState);

static int DbgReadStr(lua_State *pLuaState);

static int DbgReadInt32(lua_State *pLuaState);

protected:
    pfnReadMemoryProc m_pfnReadProc;
    pfnWriteMemoryProc m_pfnWriteProc;
    TITAN_ENGINE_CONTEXT_t m_vContex;
    map<ustring, ScriptStrInfo> m_vStrMap;
};

CScriptEngine *GetScriptEngine();
#endif