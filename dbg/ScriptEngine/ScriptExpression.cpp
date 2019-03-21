#include "ScriptExpression.h"
#include <ComLib/deelx.h>
#include "ScriptHlpr.h"
#include <ComLib/StrUtil.h>
#include "ScriptParser.h"
#include <Shlwapi.h>
#include "ScriptAccessor.h"

DWORD CScriptExpReader::msVarSerial = 0;

CScriptExpReader *CScriptExpReader::GetInst() {
    static CScriptExpReader *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CScriptExpReader();
    }
    return s_ptr;
}

void CScriptExpReader::SetCache(ScriptCache *cache) {
    mCache = cache;
}

void CScriptExpReader::AddVar(VariateDesc *desc) {
    mCache->InsertVar(desc);
}

VariateDesc *CScriptExpReader::GetVarByName(const mstring &name) const {
    return mCache->GetVarByName(name);
}

VariateDesc *CScriptExpReader::ParserExpression(const ScriptCmdContext &ctx) {
    mTempVarSet.clear();
    if (ctx.isDbggerCmd)
    {
        VariateDesc *param = GetGbkDesc(ctx.mCommand);
        vector<VariateDesc *> paramSet;
        paramSet.push_back(param);
        return CallInternalProc("RunCommand", paramSet);
    } else {
        return ParserStr(ctx.mCommand);
    }
}

//变量或者数字判定可能会重合，优先数字判定
bool CScriptExpReader::IsVariable(const mstring &str) const {
    const char *rule = "(^[a-z, A-Z, _]+\\w*$)|(^\\w*[a-z, A-Z, _]+$)|(^\\w*[a-z, A-Z, _]\\w*$)|(^@var_\\d+$)";

    CRegexpT<char> regex;
    regex.Compile(rule);

    MatchResult result = regex.MatchExact(str.c_str());
    return (1 == result.IsMatched());
}

//函数调用判定
bool CScriptExpReader::IsProc(const mstring &str) const {
    const char *rule = "^\\w+\\(.+\\)$";

    CRegexpT<char> regex;
    regex.Compile(rule);

    MatchResult result = regex.MatchExact(str.c_str());
    return (1 == result.IsMatched());
}

//数字判定，为何变量区分，必须为0x或者0n打头
bool CScriptExpReader::IsNumber(const mstring &str) const {
    const char *rule = "(^0x[a-f, 0-9]+$)|(^0n[0-9]+$)|(^[0-9]+$)";

    CRegexpT<char> regex;
    regex.Compile(rule);

    MatchResult result = regex.MatchExact(str.c_str());
    return (1 == result.IsMatched());
}

//gbk字符串判定
bool CScriptExpReader::IsGbkStr(const mstring &str) const {
    const char *rule = "^\".*\"$";

    CRegexpT<char> regex;
    regex.Compile(rule);

    MatchResult result = regex.MatchExact(str.c_str());
    return (1 == result.IsMatched() && (result.GetEnd() == str.size()));
}

//unicode字符串判定
bool CScriptExpReader::IsUnicodeStr(const mstring &wstr) const {
    const char *rule = "^L\".*\"$";

    CRegexpT<char> regex;
    regex.Compile(rule);

    MatchResult result = regex.MatchExact(wstr.c_str());
    return (1 == result.IsMatched() && (result.GetEnd() == wstr.size()));
}

//是否是内置变量 eg:@esp, @ebp, @eax, @param0, @param1
bool CScriptExpReader::IsVarInternal(const mstring &str, VariateDesc *&desc) {
    char buff[16] = {0};

    if (CScriptAccessor::GetInst()->GetInternalVarData(str, buff))
    {
        LPVOID ptr = NULL;
        memcpy(&ptr, buff, sizeof(LPVOID));
        desc = GetPtrDesc(ptr);
        return true;
    }
    return false;
}

bool CScriptExpReader::RegisterProc(
    const mstring &procName,
    VariateType returnType,
    const vector<VariateType> &paramSet,
    pfnProcInternal proc
    )
{
    ScriptProcRegisterInfo *newInfo = new ScriptProcRegisterInfo();
    newInfo->mProcName = procName;
    newInfo->mParamType = paramSet;

    newInfo->mReturnType = returnType;
    newInfo->mProcInternal = proc;
    mProcSet[procName] = newInfo;
    return true;
}

VariateDesc *CScriptExpReader::GetGbkDesc(const mstring &str) {
    VariateDesc *desc = new VariateDesc();
    desc->mVarName = CScriptHlpr::GetTempVarName();
    desc->mVarType = em_var_str_gbk;
    desc->mStrValue = str;
    desc->mStrValue.repsub("\\\"", "\"");

    AddVar(desc);
    return desc;
}

VariateDesc *CScriptExpReader::GetUnicodeDesc(const mstring &str) {
    VariateDesc *desc = new VariateDesc();
    desc->mVarName = CScriptHlpr::GetTempVarName();
    desc->mVarType = em_var_str_unicode;
    desc->mStrValue = str;
    desc->mStrValue.repsub("\\\"", "\"");

    AddVar(desc);
    return desc;
}

VariateDesc *CScriptExpReader::GetIntDesc(DWORD64 d) {
    VariateDesc *desc = new VariateDesc();
    desc->mVarName = CScriptHlpr::GetTempVarName();
    desc->mVarType = em_var_int;
    desc->mIntValue = d;
    desc->mVarLength = sizeof(DWORD64);

    AddVar(desc);
    return desc;
}

VariateDesc *CScriptExpReader::GetPtrDesc(LPVOID ptr) {
    VariateDesc *desc = new VariateDesc();
    desc->mVarName = CScriptHlpr::GetTempVarName();
    desc->mVarType = em_var_ptr;
    desc->mPtrValue = ptr;
    desc->mVarLength = sizeof(LPVOID);

    AddVar(desc);
    return desc;
}

VariateDesc *CScriptExpReader::GetPendingDesc() {
    VariateDesc *desc = new VariateDesc();
    desc->mVarName = CScriptHlpr::GetTempVarName();
    desc->mVarType = em_var_pending;

    AddVar(desc);
    return desc;
}

VariateDesc *CScriptExpReader::CallInternalProc(const mstring &procName, vector<VariateDesc *> &param) {
    map<mstring, ScriptProcRegisterInfo *>::const_iterator it;
    if (mProcSet.end() == (it = mProcSet.find(procName)))
    {
        throw (new CScriptParserException(FormatA("未知的内部函数:%hs", procName.c_str())));
        return NULL;
    }

    ScriptProcRegisterInfo *ptr = it->second;
    if (param.size() != ptr->mParamType.size())
    {
        throw (new CScriptParserException(FormatA("函数 %hs 参数数量不匹配", procName.c_str())));
        return NULL;
    }

    for (size_t i = 0 ; i != param.size() ; i++)
    {
        if (param[i]->mVarType != ptr->mParamType[i])
        {
            throw (new CScriptParserException(FormatA("函数 %hs 参数类型不匹配", procName.c_str())));
            return NULL;
        }
    }
    return it->second->mProcInternal(param);
}

VariateDesc *CScriptExpReader::ProcStrStartWithA(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 2) {
        throw(new CScriptParserException("Call StrStartWithError，param Err1"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_gbk) || (paramSet[1]->mVarType != em_var_str_gbk))
    {
        throw(new CScriptParserException("Call StrStartWithError，param Err2"));
        return NULL;
    }

    mstring str1 = paramSet[0]->mStrValue;
    mstring str2 = paramSet[1]->mStrValue;

    DWORD64 b = str1.startwith(str2.c_str());
    return GetInst()->GetIntDesc(b);
}

VariateDesc *CScriptExpReader::ProcStrStartWithW(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 2) {
        throw(new CScriptParserException("Call StrStartWithError，param Err1"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_unicode) || (paramSet[1]->mVarType != em_var_str_unicode))
    {
        throw(new CScriptParserException("Call StrStartWithError，param Err2"));
        return NULL;
    }

    mstring str1 = paramSet[0]->mStrValue;
    mstring str2 = paramSet[1]->mStrValue;

    DWORD64 b = str1.startwith(str2.c_str());
    return GetInst()->GetIntDesc(b);
}

VariateDesc *CScriptExpReader::ProcStrSubStrA(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 3) {
        throw(new CScriptParserException("调用StrSubStrA参数数量不匹配"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_gbk) || (paramSet[1]->mVarType != em_var_int) || (paramSet[2]->mVarType != em_var_int))
    {
        throw(new CScriptParserException("调用StrSubStrA参数类型不匹配"));
        return NULL;
    }

    mstring str = paramSet[0]->mStrValue;
    DWORD64 pos1 = paramSet[1]->mIntValue;
    DWORD64 count = paramSet[2]->mIntValue;
    mstring sub = str.substr((size_t)pos1, (size_t)count);
    return GetInst()->GetGbkDesc(sub);
}

VariateDesc *CScriptExpReader::ProcStrSubStrW(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 3) {
        throw(new CScriptParserException("调用StrSubStrW参数数量不匹配"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_unicode) || (paramSet[1]->mVarType != em_var_int) || (paramSet[2]->mVarType != em_var_int))
    {
        throw(new CScriptParserException("调用StrSubStrW参数类型不匹配"));
        return NULL;
    }

    mstring str = paramSet[0]->mStrValue;
    size_t pos1 = (size_t)paramSet[1]->mIntValue;
    size_t count = (size_t)paramSet[2]->mIntValue;
    mstring sub = str.substr(pos1, count);
    return GetInst()->GetUnicodeDesc(sub);
}

VariateDesc *CScriptExpReader::ProcStrCatA(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 2) {
        throw(new CScriptParserException("调用StrCatA参数数量不匹配"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_gbk) || (paramSet[1]->mVarType != em_var_str_gbk))
    {
        throw(new CScriptParserException("调用StrCatA参数类型不匹配"));
        return NULL;
    }

    mstring sub = paramSet[0]->mStrValue + paramSet[1]->mStrValue;
    return GetInst()->GetGbkDesc(sub);
}

VariateDesc *CScriptExpReader::ProcStrCatW(vector<VariateDesc *> &paramSet) {
    if (paramSet.size() != 3) {
        throw(new CScriptParserException("调用StrCatW参数数量不匹配"));
        return NULL;
    }

    if ((paramSet[0]->mVarType != em_var_str_unicode) || (paramSet[1]->mVarType != em_var_str_unicode))
    {
        throw(new CScriptParserException("调用StrCatW参数类型不匹配"));
        return NULL;
    }
    mstring sub = paramSet[0]->mStrValue + paramSet[1]->mStrValue;
    return GetInst()->GetUnicodeDesc(sub);
}

VariateDesc *CScriptExpReader::ProcRunCommand(vector<VariateDesc *> &paramSet) {
    VariateDesc *ret = new VariateDesc();
    ret->mVarType = em_var_int;
    return ret;
}

//默认10进制，16进制数字必须以0x打头
BOOL CScriptExpReader::GetNumFromStr(const mstring &strNumber, DWORD64 &dwResult) const
{
    mstring str(strNumber);
    if (str.startwith("0n"))
    {
        str.erase(0, 2);
    }
    return StrToInt64ExA(str.c_str(), STIF_SUPPORT_HEX, (LONGLONG *)&dwResult);
}

vector<CScriptExpReader::ScriptProc> CScriptExpReader::GetProcSet(const mstring &express) const {
    const char *rule = "\\w+\\(.+\\)";

    vector<ScriptProc> result;
    CRegexpT<char> regex;
    regex.Compile(rule);
    size_t lastPos = 0;

    while (true) {
        MatchResult ret = regex.Match(express.c_str(), lastPos);
        if (1 != ret.IsMatched()) {
            break;
        }

        ScriptProc tmp;
        tmp.mStartPos = ret.GetStart();
        tmp.mEndPos = ret.GetEnd();
        tmp.mProcStr = express.substr(tmp.mStartPos, tmp.mEndPos - tmp.mStartPos);
        result.push_back(tmp);
        lastPos = ret.GetEnd();
    }
    return result;
}

VariateDesc *CScriptExpReader::GetDataDesc(const mstring &str) {
    size_t pos1 = 0;
    size_t pos2 = 0;
    mstring tmp;

    if (IsNumber(str))
    {
        DWORD64 dd = 0;
        GetNumFromStr(str, dd);

        return GetIntDesc(dd);
    } else if (IsVariable(str))
    {
        return GetVarByName(str);
    } else if (IsProc(str))
    {
        //proc handle
        int d = 1234;
    } else if (IsGbkStr(str))
    {
        pos1 = str.find("\"");
        pos2 = str.rfind("\"");

        return GetGbkDesc(str.substr(pos1 + 1, pos2 - pos1 - 1));
    } else if (IsUnicodeStr(str))
    {
        pos1 = str.find("\"");
        pos2 = str.rfind("\"");

        return GetUnicodeDesc(str.substr(pos1 + 1, pos2 - pos1 - 1));
    }
    else
    {
        throw(new CScriptParserException("! operator type error2"));
    }
    return NULL;
}

void CScriptExpReader::ReplaceNode(vector<ScriptData> &nodeSet, size_t pos1, size_t pos2, const ScriptData &newNode) const {
    nodeSet[pos1] = newNode;

    if (pos2 > pos1 + 1)
    {
        nodeSet.erase(nodeSet.begin() + pos1 + 1, nodeSet.begin() + pos2);
    }
}

size_t CScriptExpReader::GetParamResult(vector<ScriptData> &nodeSet, size_t index) {
    ScriptData d = nodeSet[index];
    mstring opt = d.mContent;

    size_t retIndex = 0;
    ScriptData d1;
    ScriptData d2;
    ScriptData newData;
    VariateDesc *desc = NULL;
    VariateDesc *desc1 = NULL;
    VariateDesc *desc2 = NULL;
    //因为运算顺序的问题一元操作符反向遍历，二元操作符正向遍历
    if (opt == "!")
    {
        d2 = nodeSet[index + 1];
        if (d2.mType != SCRIPT_DATA_DATA)
        {
            throw (new CScriptParserException("! operator type error"));
            return 0;
        }

        desc = GetDataDesc(d2.mContent);
        if (desc->mIntValue == 0)
        {
            desc->mIntValue = 1;
        } else {
            desc->mIntValue = 0;
        }

        retIndex = index - 1;
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index, index + 2, newData);
    } else if (opt == "~")
    {
        d2 = nodeSet[index + 1];
        desc = GetDataDesc(d2.mContent);
        if (desc->mVarType != em_var_int)
        {
            throw (new CScriptParserException("~ operator type error"));
            return 0;
        }

        desc->mIntValue = ~desc->mIntValue;
        retIndex = index - 1;
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;

        ReplaceNode(nodeSet, index, index + 2, newData);
    } else if (opt == "*")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = desc1->mIntValue * desc2->mIntValue;
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    }
    else if (opt == "/")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = desc1->mIntValue / desc2->mIntValue;
        nodeSet.erase(nodeSet.begin() + index - 1, nodeSet.begin() + index + 2);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        nodeSet.insert(nodeSet.begin() + index - 1, newData);
        retIndex = index - 1;
    }
    else if (opt == "%")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = desc1->mIntValue % desc2->mIntValue;
        nodeSet.erase(nodeSet.begin() + index - 1, nodeSet.begin() + index + 2);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        nodeSet.insert(nodeSet.begin() + index - 1, newData);
        retIndex = index - 1;
    } else if (opt == "+")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = desc1->mIntValue + desc2->mIntValue;
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "-")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = desc1->mIntValue - desc2->mIntValue;
        nodeSet.erase(nodeSet.begin() + index - 1, nodeSet.begin() + index + 2);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        nodeSet.insert(nodeSet.begin() + index - 1, newData);
        retIndex = index - 1;
    } else if (opt == "<<")
    {
    } else if (opt == ">>")
    {
    } else if (opt == ">")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue > desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;

        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == ">=")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue >= desc2->mIntValue);
        nodeSet.erase(nodeSet.begin() + index - 1, nodeSet.begin() + index + 2);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        nodeSet.insert(nodeSet.begin() + index - 1 , newData);
        retIndex = index - 1;
    } else if (opt == "<")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue < desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "<=")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue <= desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "==")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue == desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "!=")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue != desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "&")
    {
    } else if (opt == "^")
    {
    } else if (opt == "|")
    {
    } else if (opt == "&&")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue && desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "||")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        if (desc1->mVarType != em_var_int || desc2->mVarType != em_var_int)
        {
            throw (new CScriptParserException("* operator type error"));
            return 0;
        }

        DWORD64 dd = (desc1->mIntValue || desc2->mIntValue);
        desc = GetIntDesc(dd);
        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    } else if (opt == "=")
    {
        d1 = nodeSet[index - 1];
        d2 = nodeSet[index + 1];

        if (!IsVariable(d1.mContent))
        {
            throw (new CScriptParserException("= operator param error"));
            return 0;
        }
        desc1 = GetDataDesc(d1.mContent);
        desc2 = GetDataDesc(d2.mContent);
        *desc1 = *desc2;

        newData.mType = SCRIPT_DATA_DATA;
        newData.mContent = desc1->mVarName;
        ReplaceNode(nodeSet, index - 1, index + 2, newData);
        retIndex = index - 1;
    }
    return retIndex;
}

VariateDesc *CScriptExpReader::ProcessScriptNode(vector<ScriptData> &nodeSet) {
    //0:!, ~ 1:/, *, % 2:+, -, <<, >> 3:>, >=, <, <=, ==, != 4:&, ^, | 5:&&, ||
    static map<int, vector<mstring>> sOptLevel;
    if (sOptLevel.empty())
    {
        sOptLevel[0].push_back("!"), sOptLevel[0].push_back("~");
        sOptLevel[1].push_back("*"), sOptLevel[1].push_back("/"), sOptLevel[1].push_back("%");
        sOptLevel[2].push_back("+"), sOptLevel[2].push_back("-"), sOptLevel[2].push_back("<<"), sOptLevel[2].push_back(">>");
        sOptLevel[3].push_back(">"), sOptLevel[3].push_back(">="), sOptLevel[3].push_back("<"), sOptLevel[3].push_back("<=");
        sOptLevel[3].push_back("=="), sOptLevel[3].push_back("!=");
        sOptLevel[4].push_back("&"), sOptLevel[4].push_back("^"), sOptLevel[4].push_back("|");
        sOptLevel[5].push_back("&&"), sOptLevel[5].push_back("||");
        sOptLevel[9].push_back("=");
    }

    map<int, vector<mstring>> optLevel = sOptLevel;
    while (true) {
        vector<mstring> curOptSet = optLevel.begin()->second;
        //因为可能出现!!!abc类似的嵌套调用，所以从后往前计算
        size_t i = 0;
        if (optLevel.begin()->first == 0)
        {
            for (i = nodeSet.size() - 1 ; i != 0 ;) 
            {
                ScriptData tmp = nodeSet[i];
                if (tmp.mType == SCRIPT_DATA_OPERATOR)
                {
                    size_t j = 0;
                    for (j = 0 ; j < curOptSet.size() ; j++)
                    {
                        mstring opt1 = curOptSet[j];
                        if (tmp.mContent == opt1)
                        {
                            //logic
                            i = GetParamResult(nodeSet, i);
                            break;
                        }
                    }

                    if (j != curOptSet.size())
                    {
                        continue;
                    }
                }
                i--;
            }
        } else {
            for (i = 0 ; i != nodeSet.size() ;) 
            {
                ScriptData tmp = nodeSet[i];
                if (tmp.mType == SCRIPT_DATA_OPERATOR)
                {
                    size_t j = 0;
                    for (j = 0 ; j < curOptSet.size() ; j++)
                    {
                        mstring opt1 = curOptSet[j];
                        if (tmp.mContent == opt1)
                        {
                            //logic
                            i = GetParamResult(nodeSet, i);
                            break;
                        }
                    }

                    if (j != curOptSet.size())
                    {
                        continue;
                    }
                }
                i++;
            }
        }

        optLevel.erase(optLevel.begin());
        if (optLevel.empty())
        {
            break;
        }
    }

    if (nodeSet.size() == 1)
    {
        return GetDataDesc(nodeSet.begin()->mContent);
    }
    return NULL;
}

VariateDesc *CScriptExpReader::ProcessSimpleStr(const mstring &expression) {
    size_t i = 0;
    vector<ScriptData> nodeSet;
    size_t lastPos = 0;

    for (i = 0 ; i < expression.size() ;)
    {
        mstring opt;
        if (IsOperator(expression, i, opt)) {
            if (i > lastPos) {
                ScriptData d1;
                d1.mType = 0;
                d1.mContent = expression.substr(lastPos, i - lastPos);
                nodeSet.push_back(d1);

                ScriptData d2;
                d2.mType = 1;
                d2.mContent = opt;
                nodeSet.push_back(d2);
                lastPos = i + opt.size();
                i = lastPos;
            } else {
                throw (new CScriptParserException("ProcessSimpleStr error1"));
                return NULL;
            }
        } else {
            i++;
        }
    }

    if (expression.size() > lastPos)
    {
        ScriptData d;
        d.mType = 0;
        d.mContent = expression.substr(lastPos, i - lastPos);
        nodeSet.push_back(d);
    }
    return ProcessScriptNode(nodeSet);
}

void CScriptExpReader::ParserProc(mstring &script) {
    vector<ScriptProc> procSet = GetProcSet(script);
    vector<VariateDesc *> resultSet;

    if (procSet.empty())
    {
        return;
    }

    for (vector<ScriptProc>::const_iterator it = procSet.begin() ; it != procSet.end() ; it++)
    {
        mstring funName, paramStr;
        size_t pos1 = 0, pos2 = 0;
        pos1 = it->mProcStr.find('(');
        pos2 = CScriptHlpr::FindNextBracket('(', ')', it->mProcStr, pos1);
        funName = it->mProcStr.substr(0, pos1);

        paramStr = it->mProcStr.substr(pos1 + 1, pos2 - pos1 - 1);
        list<mstring> paramStrList = SplitStrA(paramStr, ",");

        vector<VariateDesc *> paramSet;
        for (list<mstring>::const_iterator ij = paramStrList.begin() ; ij != paramStrList.end() ; ij++)
        {
            paramSet.push_back(GetDataDesc(*ij));
        }
        VariateDesc *result = CallInternalProc(funName, paramSet);
        resultSet.push_back(result);
    }

    for (int i = (int)procSet.size() - 1 ; i >= 0 ; i--)
    {
        script.replace(procSet[i].mStartPos, procSet[i].mEndPos - procSet[i].mStartPos, resultSet[i]->mVarName);
    }
}

VariateDesc *CScriptExpReader::ParserStr(const mstring &str) {
    mstring strCopy = str;
    size_t pos1 = 0, pos2 = 0;
    VariateDesc *desc = NULL;
    if (strCopy.empty())
    {
        return GetPendingDesc();
    }
    mstring lastStr;
    if (strCopy.startwith("var ")) {
        pos1 = strlen("var ");
        pos2 = strCopy.find("=", pos1);

        mstring name;
        mstring expression;
        if (mstring::npos == pos2) {
            name = strCopy.substr(pos1, expression.size() - pos1);
        } else {
            name = strCopy.substr(pos1, pos2 - pos1);
            expression = strCopy.substr(pos2 + 1, str.size() - pos2 - 1);
        }

        desc = ParserStr(expression);
        desc->mVarName = name;
        AddVar(desc);
    } else {
        //先计算表达式中的函数,可能和后面的解析冲突
        ParserProc(strCopy);

        //消除括号
        size_t lastPos = 0;
        while (true) {
            pos1 = strCopy.find('(', lastPos);
            if (mstring::npos == pos1)
            {
                break;
            }

            pos2 = CScriptHlpr::FindNextBracket('(', ')', strCopy, pos1);
            lastStr = strCopy.substr(pos1 + 1, pos2 - pos1 - 1);
            VariateDesc *desc = ParserStr(lastStr);
            desc->mVarName = CScriptHlpr::GetTempVarName();
            mTempVarSet[desc->mVarName] = desc;
            strCopy.replace(pos1, pos2 - pos1 + 1, desc->mVarName);
        }

        //消除取值操作符[]
        lastPos = 0;
        while (true) {
            pos1 = strCopy.find('[', lastPos);
            if (mstring::npos == pos1)
            {
                break;
            }

            pos2 = CScriptHlpr::FindNextBracket('[', ']', strCopy, pos1);
            if (pos2 == mstring::npos)
            {
                throw (new CScriptParserException("[]匹配失败"));
            }
            lastStr = strCopy.substr(pos1 + 1, pos2 - pos1 - 1);
            VariateDesc *desc = ParserStr(lastStr);
            desc->mVarName = CScriptHlpr::GetTempVarName();
            mTempVarSet[desc->mVarName] = desc;
            strCopy.replace(pos1, pos2 - pos1 + 1, desc->mVarName);
        }
        //express eg: a + b + 1234, StrStartWithW, a + StrStrlenA();
        desc = ProcessSimpleStr(strCopy);
    }
    return desc;
}

CScriptExpReader::CScriptExpReader() {
    mCache = NULL;
}

CScriptExpReader::~CScriptExpReader() {}

bool CScriptExpReader::IsOperator(const mstring &script, size_t pos, mstring &opt) const {
    if (pos >= script.size())
    {
        return false;
    }

    char c1 = script.c_str()[pos];
    if ((c1 >= 'a' && c1 <= 'z') || (c1 >= 'A' && c1 <= 'Z') || (c1 >= '0' && c1 <='9'))
    {
        return false;
    }

    char str[3] = {0};
    if (pos >= (script.size() - 1)) {
        str[0] = c1;
        if (mOperatorSet.end() != mOperatorSet.find(str))
        {
            opt = str;
            return true;
        }
    } else {
        str[0] = c1;
        str[1] = script.c_str()[pos + 1];

        if (mOperatorSet.end() != mOperatorSet.find(str))
        {
            opt = str;
            return true;
        } else {
            str[1] = 0x00;
            if (mOperatorSet.end() != mOperatorSet.find(str))
            {
                opt = str;
                return true;
            }
        }
    }
    return false;
}

void CScriptExpReader::InitReader() {
    msVarSerial = 0xff12;
    mOperatorSet.insert("+"), mOperatorSet.insert("-"), mOperatorSet.insert("*");
    mOperatorSet.insert("/"), mOperatorSet.insert("%"), mOperatorSet.insert("^");
    mOperatorSet.insert("|"), mOperatorSet.insert("&&"), mOperatorSet.insert("||");
    mOperatorSet.insert(">"), mOperatorSet.insert("<"), mOperatorSet.insert(">=");
    mOperatorSet.insert("<="), mOperatorSet.insert("=="), mOperatorSet.insert("!=");
    mOperatorSet.insert("=");

    mCmdInternal.insert("bp");
    mCmdInternal.insert("g");
    mCmdInternal.insert("gu");

    vector<VariateType> paramType;
    paramType.push_back(em_var_str_gbk);
    paramType.push_back(em_var_str_gbk);
    RegisterProc("StrStartWithA", em_var_int, paramType, ProcStrStartWithA);

    paramType.clear();
    paramType.push_back(em_var_str_unicode);
    paramType.push_back(em_var_str_unicode);
    RegisterProc("StrStartWithW", em_var_int, paramType, ProcStrStartWithW);

    paramType.clear();
    paramType.push_back(em_var_str_gbk);
    paramType.push_back(em_var_int);
    paramType.push_back(em_var_int);
    RegisterProc("StrSubStrA", em_var_int, paramType, ProcStrSubStrA);

    paramType.clear();
    paramType.push_back(em_var_str_unicode);
    paramType.push_back(em_var_int);
    paramType.push_back(em_var_int);
    RegisterProc("StrSubStrW", em_var_int, paramType, ProcStrSubStrW);

    paramType.clear();
    paramType.push_back(em_var_str_gbk);
    paramType.push_back(em_var_str_gbk);
    RegisterProc("StrCatA", em_var_str_gbk, paramType, ProcStrCatA);

    paramType.clear();
    paramType.push_back(em_var_str_unicode);
    paramType.push_back(em_var_str_unicode);
    RegisterProc("StrCatW", em_var_str_unicode, paramType, ProcStrCatW);

    paramType.clear();
    paramType.push_back(em_var_str_gbk);
    RegisterProc("RunCommand", em_var_int, paramType, ProcRunCommand);
}