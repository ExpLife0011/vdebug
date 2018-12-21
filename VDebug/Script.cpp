#include <Shlwapi.h>
#include <list>
#include "Script.h"
#include "memory.h"
#include "common.h"
#include "crc32.h"
#include "memory.h"
#include "MainView.h"
#include <ComLib/global.h>

//传递给lua的错误码
#define SCRIPT_ERROR_SUCCESS            0
#define SCRIPT_ERROR_CMDNOFIND          1
#define SCRIPT_ERROR_LUASTATERR         2
#define SCRIPT_ERROR_PARAMERR           3
#define SCRIPT_ERROR_MEMORYERR          4

#define EXCEPTION_CODE_SYSEXCEPTION    (1)
#define EXCEPTION_STR_SYSEXCEPTION     ("系统异常")
#define EXCEPTION_CODE_LEAVESCRIPT     (2)
#define EXCEPTION_STR_LEAVESCRIPT      ("中断脚本")

#define EXCEPTION_CODE_UNKNOW          (1001)
#define EXCEPTION_STR_UNKNOW           ("未知错误")

//脚本内部异常信息
class CScriptException
{
public:
    CScriptException(int iCode)
    {
        struct ExceptionInfo
        {
            int iErr;
            LPCSTR szErrMsg;
        };

        ExceptionInfo vErrArry[] =
        {
            {EXCEPTION_CODE_SYSEXCEPTION, EXCEPTION_STR_SYSEXCEPTION},
            {EXCEPTION_CODE_LEAVESCRIPT, EXCEPTION_STR_LEAVESCRIPT}
        };

        for (int i = 0 ; i < RTL_NUMBER_OF(vErrArry) ; i++)
        {
            if (vErrArry[i].iErr == iCode)
            {
                m_iErr = iCode;
                m_strErrorMsg = vErrArry[i].szErrMsg;
                return;
            }
        }

        m_iErr = EXCEPTION_CODE_UNKNOW;
        m_strErrorMsg = EXCEPTION_STR_UNKNOW;
    }

    virtual ~CScriptException()
    {}

    const char *GetErrorMsg() const
    {
        return m_strErrorMsg.c_str();
    }

    int GetErrCode()
    {
        return m_iErr;
    }
protected:
    mstring m_strErrorMsg;
    int m_iErr;
};

CScriptEngine *GetScriptEngine()
{
    static CScriptEngine *s_ptr = new CScriptEngine();
    return s_ptr;
}

CScriptEngine::CScriptEngine()
{
    ZeroMemory(&m_vContex, sizeof(m_vContex));
}

void CScriptEngine::SetContext(TITAN_ENGINE_CONTEXT_t &vContext, pfnReadMemoryProc pfnRead, pfnWriteMemoryProc pfnWrite)
{
    m_vContex = vContext;
    ScriptStrInfo info;
    info.m_eType = em_scriptstr_register;

    info.m_dwAddr = m_vContex.cax;
    m_vStrMap[L"cax"] = info;
    info.m_dwAddr = m_vContex.cbx;
    m_vStrMap[L"cbx"] = info;
    info.m_dwAddr = m_vContex.ccx;
    m_vStrMap[L"ccx"] = info;
    info.m_dwAddr = m_vContex.cdx;
    m_vStrMap[L"cdx"] = info;
    info.m_dwAddr = m_vContex.csp;
    m_vStrMap[L"csp"] = info;
    info.m_dwAddr = m_vContex.cbp;
    m_vStrMap[L"cbp"] = info;
    m_pfnReadProc = pfnRead;
    m_pfnWriteProc = pfnWrite;
}

CScriptEngine::~CScriptEngine()
{}

BOOL CScriptEngine::GetNumFromStr(const ustring &wstrNumber, DWORD64 &dwResult) const
{
    if (IsRegisterStr(wstrNumber, dwResult))
    {
        return TRUE;
    }

    ustring wstr(wstrNumber);
    if (0 != wstr.comparei(L"0x") && 0 != wstr.comparei(L"0n"))
    {
        wstr.insert(0, L"0x");
    }
    return StrToInt64ExW(wstr.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
}

BOOL CScriptEngine::IsOperator(WCHAR cLetter) const
{
    switch (cLetter)
    {
    case L'+':
    case L'-':
    case L'*':
    case L'/':
        return TRUE;
    }
    return FALSE;
}

BOOL CScriptEngine::IsRegisterStr(const ustring &wstr, DWORD64 &dwData) const
{
    BOOL bResult = FALSE;
    if (GetCurrentDbgger()->IsDbgProcx64())
    {
        //x64
        if (wstr == L"csp" || wstr == L"rsp")
        {
            dwData = m_vContex.csp;
            bResult = TRUE;
        }
        else if (wstr == L"cbp" || wstr == L"rbp")
        {
            dwData = m_vContex.cbp;
            bResult = TRUE;
        }
        else if (wstr == L"cax" || wstr == L"rax")
        {
            dwData = m_vContex.cax;
            bResult = TRUE;
        }
        else if (wstr == L"cbx" || wstr == L"rbx")
        {
            dwData = m_vContex.cbx;
            bResult = TRUE;
        }
        else if (wstr == L"ccx" || wstr == L"rcx")
        {
            dwData = m_vContex.ccx;
            bResult = TRUE;
        }
        else if (wstr == L"cdx" || wstr == L"rdx")
        {
            dwData = m_vContex.cdx;
            bResult = TRUE;
        }
        #ifdef _WIN64
        else if (wstr == L"r8")
        {
            dwData = m_vContex.r8;
            bResult = TRUE;
        }
        else if (wstr == L"r9")
        {
            dwData = m_vContex.r9;
            bResult = TRUE;
        }
        #endif
    }
    else
    {
        //x86
        if (wstr == L"csp" || wstr == L"esp")
        {
            dwData = m_vContex.csp;
            bResult = TRUE;
        }
        else if (wstr == L"cax" || wstr == L"eax")
        {
            dwData = m_vContex.cax;
            bResult = TRUE;
        }
        else if (wstr == L"cbx" || wstr == L"ebx" || wstr == L"rbx")
        {
            dwData = m_vContex.cbx;
            bResult = TRUE;
        }
        else if (wstr == L"cbp" || wstr == L"ebp" || wstr == L"rbp")
        {
            dwData = m_vContex.cbp;
            bResult = TRUE;
        }
    }
    return bResult;
}

ustring CScriptEngine::GetPointerData(const ustring &wstrPointer) const
{
    DWORD64 dwData = 0;
    DWORD64 dwAddr = 0;
    DWORD dwRead = 0;
    DWORD dwSize = 4;

    if (!IsRegisterStr(wstrPointer, dwData))
    {
        if (!GetNumFromStr(wstrPointer, dwAddr))
        {
            return L"";
        }
    }

    if (GetCurrentDbgger()->IsDbgProcx64())
    {
        dwSize = 8;
    }
    m_pfnReadProc(dwAddr, dwSize, (char *)&dwData);

    WCHAR wszBuf[128] = {0};
    ustring wstrResult = L"0x";
    _ui64tow_s(dwData, wszBuf, 128, 16);
    wstrResult += wszBuf;
    return wstrResult;
}

ustring CScriptEngine::GetTwoNumCalResult(const ustring &wstrFirst, const ustring &wstrSecond, ScriptOperator eOperator) const
{
    DWORD64 dw1 = 0;
    DWORD64 dw2 = 0;

    if (!GetNumFromStr(wstrFirst, dw1) || !GetNumFromStr(wstrSecond, dw2))
    {
        return L"";
    }

    DWORD64 dwResult = 0;
    switch (eOperator)
    {
    case em_operator_add:
        dwResult = dw1 + dw2;
        break;
    case  em_operator_sub:
        dwResult = dw1 - dw2;
        break;
    case em_operator_mult:
        dwResult = dw1 * dw2;
        break;
    case em_operator_dev:
        dwResult = dw1 / dw2;
        break;
    default:
        break;
    }

    WCHAR wszBuf[128] = {0};
    ustring wstrResult = L"0x";
    _ui64tow_s(dwResult, wszBuf, 128, 16);
    wstrResult += wszBuf;
    return wstrResult;
}

bool CScriptEngine::IsHexChar(WCHAR cLetter) const
{
    if (cLetter >= L'a' && cLetter <= 'f')
    {
        return true;
    }

    if (cLetter >= L'0' && cLetter <= '9')
    {
        return true;
    }

    return (cLetter == L'x');
}

ScriptOperator CScriptEngine::GetOperator(WCHAR cOperator) const
{
    switch (cOperator)
    {
    case  L'+':
        return em_operator_add;
        break;
    case  L'-':
        return em_operator_sub;
        break;
    case L'*':
        return em_operator_mult;
        break;
    case  L'/':
        return em_operator_dev;
        break;
    }
    return em_operator_unknow;
}

ustring CScriptEngine::DeleteOperator(const ustring &wstrScript, const ustring &wstrOpt) const
{
    ustring wstrResult(wstrScript);
    for (int i = 0 ; i < (int)wstrResult.size() ;)
    {
        //先计算乘除运算符
        if (ustring::npos != wstrOpt.find(wstrResult[i]))
        {
            ustring wstr1;
            ustring wstr2;
            int iStartPos = 0;
            int iEndPos = 0;

            if (i == 0)
            {
                throw CScriptException(EXCEPTION_CODE_SYSEXCEPTION);
                return L"";
            }

            int j = 0;
            for (j = i - 1 ; j != 0 ; j--)
            {
                if (IsOperator(wstrResult[j]))
                {
                    j += 1;
                    break;
                }
            }
            iStartPos = j;
            wstr1 = wstrResult.substr(j, i - j);
            for (j = i + 1 ; j < (int)wstrResult.size() ; j++)
            {
                if (IsOperator(wstrResult[j]))
                {
                    break;
                }
            }
            wstr2 = wstrResult.substr(i + 1, j - i - 1);
            iEndPos = j;

            ustring wstrOptResult = GetTwoNumCalResult(wstr1, wstr2, GetOperator(wstrResult[i]));
            wstrResult.replace(iStartPos, iEndPos - iStartPos, wstrOptResult);
            i = (int)(iStartPos + wstrOptResult.size() + 1);
        }
        else
        {
            i++;
        }
    }
    return wstrResult;
}

//无方括号表达式计算 先乘除 后加减
//eg: 0x12434 * 4 + 1234
ustring CScriptEngine::GetSimpleResult2(const ustring &wstrScript) const
{
    ustring wstrResult = DeleteOperator(wstrScript, L"*/");
    return DeleteOperator(wstrResult, L"+-");
}

//无圆括号表达式计算
ustring CScriptEngine::GetSimpleResult1(const ustring &wstrScript) const
{
    ustring wstr(wstrScript);
    for (int i = 0 ; i < (int)wstr.size() ;)
    {
        if (wstr[i] == L']')
        {
            int j = 0;
            for (j = i ; j >= 0 ; j--)
            {
                if (wstr[j] == L'[')
                {
                    int iStartPos = j + 1;
                    int iEndPos = i;
                    ustring wstrSub = wstr.substr(iStartPos, iEndPos - iStartPos);

                    ustring wstResult = GetSimpleResult2(wstrSub);
                    wstResult = GetPointerData(wstResult);
                    wstr.replace(j, i - j + 1, wstResult);
                    i = (int)(i - 1 + wstrSub.size());
                    break;
                }
            }

            if (j < 0)
            {
                throw CScriptException(EXCEPTION_CODE_SYSEXCEPTION);
                return L"";
            }
        }
        else
        {
            i++;
        }
    }

    return wstr;
}

//[esp + 4]
//0xffabcd + 0x1234
//0x123 + [esp + 4] + eax * (4 + 0x1122)
DWORD64 CScriptEngine::Compile(const ustring &wstrScript) const
{
    try
    {
        ustring wstr = wstrScript;
        wstr.makelower();
        wstr.trim();
        wstr.delchar(' ');

        //先消掉()
        for (int i = 0 ; i < (int)wstr.size() ;)
        {
            if (wstr[i] == L')')
            {
                int j = 0;
                for (j = i ; j >= 0 ; j--)
                {
                    if (wstr[j] == L'(')
                    {
                        int iStartPos = j + 1;
                        int iEndPos = i;
                        ustring wstrSub = wstr.substr(iStartPos, iEndPos - iStartPos);

                        ustring wstrResult = GetSimpleResult1(wstrSub);
                        wstrResult = GetSimpleResult2(wstrResult);
                        wstr.replace(j, i - j + 1, wstrResult);
                        i = (iStartPos - 1 + (int)wstrResult.size());
                        break;
                    }
                }

                if (j < 0)
                {
                    throw CScriptException(EXCEPTION_CODE_SYSEXCEPTION);
                }
            }
            else
            {
                i++;
            }
        }

        DWORD64 dwResult = 0;
        wstr = GetSimpleResult1(wstr);
        wstr = GetSimpleResult2(wstr);
        GetNumFromStr(wstr, dwResult);
        return dwResult;
    }
    catch (CScriptException &e)
    {
        dp(L"Error:%hs", e.GetErrorMsg());
        return SCRIPT_RESULT_ERROR;
    }
}

void CScriptEngine::InsertProc(LPCWSTR wszModule, LPCWSTR wszProc, DWORD64 dwAddr)
{
    ustring wstr = wszModule;
    wstr += L"!";
    wstr += wszProc;
    ScriptStrInfo info;
    info.m_dwAddr = dwAddr;
    info.m_eType = em_scriptstr_proc;
    wstr.makelower();

    m_vStrMap[wstr] = info;
}

DWORD64 CScriptEngine::GetAddrForStr(const ustring &wstr)
{
    ustring wstrLower(wstr);
    wstrLower.makelower();
    map<ustring, ScriptStrInfo>::const_iterator it = m_vStrMap.find(wstrLower);

    DWORD64 dwAddr = INVALID_SCRIPT_ADDR;
    if (m_vStrMap.end() == it)
    {
        if (!GetNumFromStr(wstrLower, dwAddr))
        {
            return INVALID_SCRIPT_ADDR;
        }
    }
    else
    {
        dwAddr = it->second.m_dwAddr;
    }
    return dwAddr;
}

enum LuaScriptStat
{
    em_script_stat_init,
    em_script_stat_wait,
    em_script_stat_succ,
    em_script_stat_faild
};

struct LuaScriptInfo
{
    ustring m_wstrUniqueMark;
    ustring m_wstrFilePath;
    HANDLE m_hNotifyEvent;
    HANDLE m_hLeaveEvent;
    DWORD m_dwThreadId;
    HANDLE m_hThread;
    LuaScriptStat m_eScriptStat;
    lua_State *m_pLuaStat;
    list<LPSTR> m_vMemoryList;

    bool IsValid()
    {
        return (!m_wstrFilePath.empty());
    }

    LuaScriptInfo()
    {
        m_hNotifyEvent = NULL;
        m_hLeaveEvent = NULL;
        m_dwThreadId = 0;
        m_hThread = NULL;
        m_eScriptStat = em_script_stat_init;
        m_pLuaStat = NULL;
    }
};

static map<ustring, LuaScriptInfo> gs_vScriptSet;

LuaScriptInfo GetScriptInfo(const ustring &wstrScript)
{
    if (wstrScript.empty() || INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wstrScript.c_str()))
    {
        return LuaScriptInfo();
    }

    ustring wstr(wstrScript);
    wstr.trim().makelower();
    LuaScriptInfo info;
    info.m_wstrFilePath = wstr;
    mstring str(wstr);
    info.m_wstrUniqueMark.format(L"%08x", std_crc32(str.c_str(), (int)str.size()));
    return info;
}

ustring CScriptEngine::GetScriptPath(LPCWSTR wszScript) const
{
    if (!wszScript || !wszScript[0])
    {
        return L"";
    }

    ustring wstr(wszScript);
    wstr.trim();
    size_t pos = wstr.rfind(L'.');
    if (ustring::npos == pos)
    {
        wstr += L".lua";
    }

    WCHAR wszPath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, wszPath, MAX_PATH);
    PathAppendW(wszPath, L"..\\script");
    PathAppendW(wszPath, wstr.c_str());

    if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(wszPath))
    {
        return wszPath;
    }
    return L"";
}

BOOL CScriptEngine::RunScript(const ustring &wstrScript) const
{
    ustring wstr = GetScriptPath(wstrScript.c_str());

    if (wstr.empty())
    {
        return FALSE;
    }

    LuaScriptInfo info = GetScriptInfo(wstr);
    if (!info.IsValid())
    {
        return FALSE;
    }

    {
        CScopedLocker lock(this);
        map<ustring, LuaScriptInfo>::iterator it = gs_vScriptSet.find(info.m_wstrUniqueMark);
        if (it != gs_vScriptSet.end())
        {
            SetEvent(it->second.m_hLeaveEvent);
            if (WAIT_TIMEOUT == WaitForSingleObject(it->second.m_hThread, 3000))
            {
                TerminateThread(it->second.m_hThread, 0);
            }
            ResetEvent(it->second.m_hNotifyEvent);
            ResetEvent(it->second.m_hLeaveEvent);
            it->second.m_eScriptStat = em_script_stat_init;
        }
        else
        {
            info.m_hNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
            info.m_hLeaveEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
            gs_vScriptSet[info.m_wstrUniqueMark] = info;
            it = gs_vScriptSet.find(info.m_wstrUniqueMark);
        }
        it->second.m_hThread =  CreateThread(NULL, 0, ScriptExecThread, &(it->second), 0, NULL);
    }
    return TRUE;
}

/*
执行指定的命令，部分命令像bp，gu等会阻塞直到命令被触发
*/
int CScriptEngine::DbgRunCmd(lua_State *pLuaStat)
{
    int count = lua_gettop(pLuaStat);
    if (count != 1)
    {
        lua_pushnumber(pLuaStat, 1);
        return 1;
    }

    ustring wstr = lua_tostring(pLuaStat, 1);
    wstr.trim();
    wstr.makelower();

    DbgCmdResult res = GetCurrentDbgger()->RunCommand(wstr);
    if (res.m_eStatus != em_dbgstat_succ)
    {
        lua_pushnumber(pLuaStat, SCRIPT_ERROR_CMDNOFIND);
        return 1;
    }

    if (0 == wstr.comparei(L"bp "))
    {
        map<ustring, LuaScriptInfo>::const_iterator it;
        {
            CScopedLocker lock(GetScriptEngine());
            for (it = gs_vScriptSet.begin() ; it != gs_vScriptSet.end() ; it++)
            {
                if (it->second.m_pLuaStat == pLuaStat)
                {
                    break;
                }
            }

            if (it == gs_vScriptSet.end())
            {
                lua_pushnumber(pLuaStat, SCRIPT_ERROR_LUASTATERR);
                return 1;
            }
        }

        HANDLE vArry[] = {it->second.m_hNotifyEvent, it->second.m_hLeaveEvent};
        DWORD dwResult = WaitForMultipleObjects(2, vArry, FALSE, INFINITE);
        if ((WAIT_OBJECT_0 + 1) == dwResult)
        {
            throw CScriptException(EXCEPTION_CODE_LEAVESCRIPT);
        }
    }
    lua_pushnumber(pLuaStat, SCRIPT_ERROR_SUCCESS);
    return 1;
}

int CScriptEngine::DbgPrintMsg(lua_State *pLuaStat)
{
    int count = lua_gettop(pLuaStat);
    if (count != 1)
    {
        lua_pushnumber(pLuaStat, 1);
        return 1;
    }

    ustring wstr = lua_tostring(pLuaStat, 1);
    wstr.trim();
    wstr.makelower();

    ////CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(wstr, COLOUR_MSG);
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());

    lua_pushnumber(pLuaStat, SCRIPT_ERROR_SUCCESS);
    return 1;
}

int CScriptEngine::DbgPrintErr(lua_State *pLuaStat)
{
    int count = lua_gettop(pLuaStat);
    if (count != 1)
    {
        lua_pushnumber(pLuaStat, 1);
        return 1;
    }

    ustring wstr = lua_tostring(pLuaStat, 1);
    wstr.trim();
    wstr.makelower();

    ////CSyntaxDescHlpr hlpr;
    //hlpr.FormatDesc(wstr, COLOUR_ERROR);
    //GetSyntaxView()->AppendSyntaxDesc(hlpr.GetResult());
    lua_pushnumber(pLuaStat, SCRIPT_ERROR_SUCCESS);
    return 1;
}

/*
说明：从制定位置读取一段字符串
参数：标识读取位置的字符串
*/
int CScriptEngine::DbgReadStr(lua_State *pLuaState)
{
    int iCount = lua_gettop(pLuaState);
    if (1 != iCount)
    {
        lua_pushnumber(pLuaState, SCRIPT_ERROR_PARAMERR);
        return 1;
    }

    string str = lua_tostring(pLuaState, 1);
    DWORD64 dwAddr = GetScriptEngine()->Compile(str);
    if (!dwAddr)
    {
        lua_pushnumber(pLuaState, SCRIPT_ERROR_PARAMERR);
        return 1;
    }

    //CMemoryOperator m(GetDebugger()->GetDebugProc());
    //mstring strData = m.MemoryReadStrUnicode(dwAddr, 1024);
    //lua_pushstring(pLuaState, strData.c_str());
    return 1;
}

/*
说明：从指定位置读取读取32未整型
参数：读取的位置字符串
*/
int CScriptEngine::DbgReadInt32(lua_State *pLuaState)
{
    int iCount = lua_gettop(pLuaState);
    if (1 != iCount)
    {
        lua_pushnumber(pLuaState, SCRIPT_ERROR_PARAMERR);
        return 1;
    }

    string str = lua_tostring(pLuaState, 1);
    DWORD64 dwAddr = GetScriptEngine()->Compile(str);
    if (!dwAddr)
    {
        lua_pushnumber(pLuaState, SCRIPT_ERROR_PARAMERR);
        return 1;
    }

    //CMemoryOperator m(GetDebugger()->GetDebugProc());
    //UINT uData = 0;
    //DWORD dwSize = sizeof(uData);
    //m.MemoryReadSafe(dwAddr, (char *)&uData, dwSize, &dwSize);

    //if (dwSize != sizeof(uData))
    //{
    //    lua_pushnumber(pLuaState, SCRIPT_ERROR_MEMORYERR);
    //    return 1;
    //}
    //lua_pushnumber(pLuaState, uData);
    return 1;
}

void CScriptEngine::CmdNotifyCallback(LPVOID pUserParam, LPVOID pContext)
{}

void CScriptEngine::ScriptEnvInit(lua_State *pLuaStat)
{
    typedef int (* pfnLua)(lua_State *);
    struct ProcInfo
    {
        LPCSTR szProcName;
        pfnLua luaProc;
    };

    ProcInfo vArry[] =
    {
        {"DbgRunCmd", DbgRunCmd},
        {"DbgPrintMsg", DbgPrintMsg},
        {"DbgPrintErr", DbgPrintErr},
        {"DbgReadStr", DbgReadStr},
        {"DbgReadInt32", DbgReadInt32}
    };

    for (int i = 0 ; i < RTL_NUMBER_OF(vArry) ; i++)
    {
        lua_register(pLuaStat, vArry[i].szProcName, vArry[i].luaProc);
    }
}

DWORD CScriptEngine::ScriptExecThread(LPVOID pParam)
{
    LuaScriptInfo *pLuaInfo = (LuaScriptInfo *)pParam;
    lua_State *pLuaStat = luaL_newstate();
    luaL_openlibs(pLuaStat);
    ScriptEnvInit(pLuaStat);

    pLuaInfo->m_eScriptStat = em_script_stat_wait;
    try
    {
        if (0 != luaL_dofile(pLuaStat, WtoA(pLuaInfo->m_wstrFilePath.c_str())))
        {
            dp(L"执行脚本失败，Err:%hs", lua_tostring(pLuaStat, -1));
            pLuaInfo->m_eScriptStat = em_script_stat_faild;
        }
        else
        {
            pLuaInfo->m_eScriptStat = em_script_stat_succ;
        }
    }
    catch (CScriptException &e)
    {
        dp(L"Script Err:%hs", e.GetErrorMsg());
        pLuaInfo->m_eScriptStat = em_script_stat_succ;
    }
    lua_close(pLuaStat);
    return 0;
}