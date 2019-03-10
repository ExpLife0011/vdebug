#ifndef SCRIPT_VDEBUG_H_H_
#define SCRIPT_VDEBUG_H_H_
#include <Windows.h>
#include <map>
#include <ComStatic/ComStatic.h>
#include "TitanEngine/TitanEngine.h"
#include <ComLib/ComLib.h>
#include "DbgBase.h"

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
    DWORD64 Compile(const mstring &script) const;
    void InsertProc(LPCSTR wszModule, LPCSTR wszProc, DWORD64 dwAddr);
    //lua script
    BOOL RunScript(const mstring &strScript) const;
protected:
    BOOL IsOperator(WCHAR cLetter) const;

    BOOL IsRegisterStr(const mstring &wstr, DWORD64 &dwData) const;

    mstring GetSimpleResult2(const mstring &strScript) const;

    mstring GetSimpleResult1(const mstring &strScript) const;

    mstring GetPointerData(const mstring &strPointer) const;

    BOOL GetNumFromStr(const mstring &strNumber, DWORD64 &dwResult) const;

    mstring GetTwoNumCalResult(const mstring &str1, const mstring &str2, ScriptOperator eOperator) const;

    ScriptOperator CScriptEngine::GetOperator(char cOperator) const;

    //计算并删除指定的操作符
    mstring DeleteOperator(const mstring &strScript, const mstring &strOpt) const;

    bool IsHexChar(CHAR cLetter) const;

    DWORD64 GetAddrForStr(const mstring &wstr);

    mstring GetScriptPath(LPCSTR script) const;

    /*lua脚本*/
//static void CmdNotifyCallback(LPVOID pUserParam, LPVOID pContext);

//static void ScriptEnvInit(lua_State *pLuaStat);

//static DWORD WINAPI ScriptExecThread(LPVOID pParam);

    /*提供给lua脚本使用的函数*/
//static int DbgRunCmd(lua_State *pLuaStat);

//static int DbgPrintMsg(lua_State *pLuaState);

//static int DbgPrintErr(lua_State *pLuaState);

//static int DbgReadStr(lua_State *pLuaState);

//static int DbgReadInt32(lua_State *pLuaState);

protected:
    pfnReadMemoryProc m_pfnReadProc;
    pfnWriteMemoryProc m_pfnWriteProc;
    TITAN_ENGINE_CONTEXT_t m_vContex;
    map<mstring, ScriptStrInfo> m_vStrMap;
};

CScriptEngine *GetScriptEngine();
#endif