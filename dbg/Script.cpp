#include <Shlwapi.h>
#include <list>
#include "Script.h"
#include "memory.h"
//#include "common.h"
//#include "crc32.h"
#include "memory.h"
//#include "MainView.h"
#include <ComLib/ComLib.h>

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
    m_vStrMap["cax"] = info;
    info.m_dwAddr = m_vContex.cbx;
    m_vStrMap["cbx"] = info;
    info.m_dwAddr = m_vContex.ccx;
    m_vStrMap["ccx"] = info;
    info.m_dwAddr = m_vContex.cdx;
    m_vStrMap["cdx"] = info;
    info.m_dwAddr = m_vContex.csp;
    m_vStrMap["csp"] = info;
    info.m_dwAddr = m_vContex.cbp;
    m_vStrMap["cbp"] = info;
    m_pfnReadProc = pfnRead;
    m_pfnWriteProc = pfnWrite;
}

CScriptEngine::~CScriptEngine()
{}

BOOL CScriptEngine::GetNumFromStr(const mstring &strNumber, DWORD64 &dwResult) const
{
    if (IsRegisterStr(strNumber, dwResult))
    {
        return TRUE;
    }

    mstring str(strNumber);
    if (0 != str.comparei("0x") && 0 != str.comparei("0n"))
    {
        str.insert(0, "0x");
    }
    return StrToInt64ExA(str.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
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

BOOL CScriptEngine::IsRegisterStr(const mstring &str, DWORD64 &dwData) const
{
    BOOL bResult = FALSE;
#if WIN64 || _WIN64
    //x64
    if (str == "csp" || str == "rsp")
    {
        dwData = m_vContex.csp;
        bResult = TRUE;
    }
    else if (str == "cbp" || str == "rbp")
    {
        dwData = m_vContex.cbp;
        bResult = TRUE;
    }
    else if (str == "cax" || str == "rax")
    {
        dwData = m_vContex.cax;
        bResult = TRUE;
    }
    else if (str == "cbx" || str == "rbx")
    {
        dwData = m_vContex.cbx;
        bResult = TRUE;
    }
    else if (str == "ccx" || str == "rcx")
    {
        dwData = m_vContex.ccx;
        bResult = TRUE;
    }
    else if (str == "cdx" || str == "rdx")
    {
        dwData = m_vContex.cdx;
        bResult = TRUE;
    }
    else if (str == "r8")
    {
        dwData = m_vContex.r8;
        bResult = TRUE;
    }
    else if (str == "r9")
    {
        dwData = m_vContex.r9;
        bResult = TRUE;
    }
#else
    //x86
    if (str == "csp" || str == "esp")
    {
        dwData = m_vContex.csp;
        bResult = TRUE;
    }
    else if (str == "cax" || str == "eax")
    {
        dwData = m_vContex.cax;
        bResult = TRUE;
    }
    else if (str == "cbx" || str == "ebx")
    {
        dwData = m_vContex.cbx;
        bResult = TRUE;
    }
    else if (str == "cbp" || str == "ebp")
    {
        dwData = m_vContex.cbp;
        bResult = TRUE;
    }
#endif
    return bResult;
}

mstring CScriptEngine::GetPointerData(const mstring &strPointer) const
{
    DWORD64 dwData = 0;
    DWORD64 dwAddr = 0;
    DWORD dwRead = 0;
    DWORD dwSize = 4;

    if (!IsRegisterStr(strPointer, dwData))
    {
        if (!GetNumFromStr(strPointer, dwAddr))
        {
            return "";
        }
    }

    /*
    if (GetCurrentDbgger()->IsDbgProcx64())
    {
        dwSize = 8;
    }
    */
    m_pfnReadProc(dwAddr, dwSize, (char *)&dwData);

    char szBuf[128] = {0};
    mstring strResult = "0x";
    _ui64toa_s(dwData, szBuf, 128, 16);
    strResult += szBuf;
    return strResult;
}

mstring CScriptEngine::GetTwoNumCalResult(const mstring &strFirst, const mstring &strSecond, ScriptOperator eOperator) const
{
    DWORD64 dw1 = 0;
    DWORD64 dw2 = 0;

    if (!GetNumFromStr(strFirst, dw1) || !GetNumFromStr(strSecond, dw2))
    {
        return "";
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

    char szBuf[128] = {0};
    mstring strResult = "0x";
    _ui64toa_s(dwResult, szBuf, 128, 16);
    strResult += szBuf;
    return strResult;
}

bool CScriptEngine::IsHexChar(char cLetter) const
{
    if (cLetter >= 'a' && cLetter <= 'f')
    {
        return true;
    }

    if (cLetter >= '0' && cLetter <= '9')
    {
        return true;
    }

    return (cLetter == 'x');
}

ScriptOperator CScriptEngine::GetOperator(char cOperator) const
{
    switch (cOperator)
    {
    case  '+':
        return em_operator_add;
        break;
    case  '-':
        return em_operator_sub;
        break;
    case  '*':
        return em_operator_mult;
        break;
    case  '/':
        return em_operator_dev;
        break;
    }
    return em_operator_unknow;
}

mstring CScriptEngine::DeleteOperator(const mstring &strScriptStr, const mstring &wstrOpt) const
{
    mstring strScript(strScriptStr);
    for (int i = 0 ; i < (int)strScript.size() ;)
    {
        //先计算乘除运算符
        if (mstring::npos != wstrOpt.find(strScript[i]))
        {
            mstring str1;
            mstring str2;
            int iStartPos = 0;
            int iEndPos = 0;

            if (i == 0)
            {
                throw CScriptException(EXCEPTION_CODE_SYSEXCEPTION);
                return "";
            }

            int j = 0;
            for (j = i - 1 ; j != 0 ; j--)
            {
                if (IsOperator(strScript[j]))
                {
                    j += 1;
                    break;
                }
            }
            iStartPos = j;
            str1 = strScript.substr(j, i - j);
            for (j = i + 1 ; j < (int)strScript.size() ; j++)
            {
                if (IsOperator(strScript[j]))
                {
                    break;
                }
            }
            str2 = strScript.substr(i + 1, j - i - 1);
            iEndPos = j;

            mstring strOptResult = GetTwoNumCalResult(str1, str2, GetOperator(strScript[i]));
            strScript.replace(iStartPos, iEndPos - iStartPos, strOptResult);
            i = (int)(iStartPos + strOptResult.size() + 1);
        }
        else
        {
            i++;
        }
    }
    return strScript;
}

//无方括号表达式计算 先乘除 后加减
//eg: 0x12434 * 4 + 1234
mstring CScriptEngine::GetSimpleResult2(const mstring &wstrScript) const
{
    mstring wstrResult = DeleteOperator(wstrScript, "*/");
    return DeleteOperator(wstrResult, "+-");
}

//无圆括号表达式计算
mstring CScriptEngine::GetSimpleResult1(const mstring &script) const
{
    mstring str(script);
    for (int i = 0 ; i < (int)str.size() ;)
    {
        if (str[i] == L']')
        {
            int j = 0;
            for (j = i ; j >= 0 ; j--)
            {
                if (str[j] == '[')
                {
                    int iStartPos = j + 1;
                    int iEndPos = i;
                    mstring sub = str.substr(iStartPos, iEndPos - iStartPos);

                    mstring result = GetSimpleResult2(sub);
                    result = GetPointerData(result);
                    str.replace(j, i - j + 1, result);
                    i = (int)(i - 1 + sub.size());
                    break;
                }
            }

            if (j < 0)
            {
                throw CScriptException(EXCEPTION_CODE_SYSEXCEPTION);
                return "";
            }
        }
        else
        {
            i++;
        }
    }

    return str;
}

//[esp + 4]
//0xffabcd + 0x1234
//0x123 + [esp + 4] + eax * (4 + 0x1122)
DWORD64 CScriptEngine::Compile(const mstring &script) const
{
    try
    {
        mstring str = script;
        str.makelower();
        str.trim();
        str.delchar(' ');

        //先消掉()
        for (int i = 0 ; i < (int)str.size() ;)
        {
            if (str[i] == ')')
            {
                int j = 0;
                for (j = i ; j >= 0 ; j--)
                {
                    if (str[j] == L'(')
                    {
                        int iStartPos = j + 1;
                        int iEndPos = i;
                        mstring sub = str.substr(iStartPos, iEndPos - iStartPos);

                        mstring result = GetSimpleResult1(sub);
                        result = GetSimpleResult2(result);
                        str.replace(j, i - j + 1, result);
                        i = (iStartPos - 1 + (int)result.size());
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
        str = GetSimpleResult1(str);
        str = GetSimpleResult2(str);
        GetNumFromStr(str, dwResult);
        return dwResult;
    }
    catch (CScriptException &e)
    {
        dp(L"Error:%hs", e.GetErrorMsg());
        return SCRIPT_RESULT_ERROR;
    }
}

void CScriptEngine::InsertProc(LPCSTR szModule, LPCSTR szProc, DWORD64 dwAddr)
{
    mstring str = szModule;
    str += "!";
    str += szProc;
    ScriptStrInfo info;
    info.m_dwAddr = dwAddr;
    info.m_eType = em_scriptstr_proc;
    str.makelower();

    m_vStrMap[str] = info;
}

DWORD64 CScriptEngine::GetAddrForStr(const mstring &str)
{
    mstring lower(str);
    lower.makelower();
    map<mstring, ScriptStrInfo>::const_iterator it = m_vStrMap.find(lower);

    DWORD64 dwAddr = INVALID_SCRIPT_ADDR;
    if (m_vStrMap.end() == it)
    {
        if (!GetNumFromStr(lower, dwAddr))
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
    mstring m_strUniqueMark;
    mstring m_strFilePath;
    HANDLE m_hNotifyEvent;
    HANDLE m_hLeaveEvent;
    DWORD m_dwThreadId;
    HANDLE m_hThread;
    LuaScriptStat m_eScriptStat;
    lua_State *m_pLuaStat;
    list<LPSTR> m_vMemoryList;

    bool IsValid()
    {
        return (!m_strFilePath.empty());
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

static map<mstring, LuaScriptInfo> gs_vScriptSet;

LuaScriptInfo GetScriptInfo(const mstring &strScript)
{
    if (strScript.empty() || INVALID_FILE_ATTRIBUTES == GetFileAttributesA(strScript.c_str()))
    {
        return LuaScriptInfo();
    }

    mstring str(strScript);
    str.trim().makelower();
    LuaScriptInfo info;
    info.m_strFilePath = str;
    info.m_strUniqueMark.format("%08x", crc32(str.c_str(), (int)str.size(), 0xffffffff));
    return info;
}

mstring CScriptEngine::GetScriptPath(LPCSTR szScript) const
{
    if (!szScript || !szScript[0])
    {
        return "";
    }

    mstring str(szScript);
    str.trim();
    size_t pos = str.rfind('.');
    if (ustring::npos == pos)
    {
        str += ".lua";
    }

    char szPath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    PathAppendA(szPath, "..\\script");
    PathAppendA(szPath, str.c_str());

    if (INVALID_FILE_ATTRIBUTES != GetFileAttributesA(szPath))
    {
        return szPath;
    }
    return "";
}

BOOL CScriptEngine::RunScript(const mstring &strScript) const
{
    mstring str = GetScriptPath(strScript.c_str());

    if (str.empty())
    {
        return FALSE;
    }

    LuaScriptInfo info = GetScriptInfo(str);
    if (!info.IsValid())
    {
        return FALSE;
    }

    {
        CScopedLocker lock(this);
        map<mstring, LuaScriptInfo>::iterator it = gs_vScriptSet.find(info.m_strUniqueMark);
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
            gs_vScriptSet[info.m_strUniqueMark] = info;
            it = gs_vScriptSet.find(info.m_strUniqueMark);
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

    ustring wstr = AtoW(lua_tostring(pLuaStat, 1));
    wstr.trim();
    wstr.makelower();

    /*
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
    */
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

    ustring wstr = AtoW(lua_tostring(pLuaStat, 1));
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

    ustring wstr = AtoW(lua_tostring(pLuaStat, 1));
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
        if (0 != luaL_dofile(pLuaStat, pLuaInfo->m_strFilePath.c_str()))
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