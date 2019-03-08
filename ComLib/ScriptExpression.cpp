#include "ScriptExpression.h"
#include <gdlib/deelx.h>
#include "ScriptHlpr.h"
#include "StrUtil.h"

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
    mCache->mVarSet.push_back(desc);
}

VariateDesc *CScriptExpReader::ParserExpression(const mstring &expression) {
    mTempVarSet.clear();
    return NULL;
}

mstring CScriptExpReader::GetTempVarName() {
    return FormatA("@var_%d", mVarSerial++);
}

VariateDesc *CScriptExpReader::ParserStr(const mstring &str) {
    mstring expression = str;
    size_t pos1 = 0, pos2 = 0;
    VariateDesc *desc = NULL;
    if (str.empty())
    {
        return desc;
    }

    mstring lastStr;
    if (expression.startwith("var ")) {
        pos1 = strlen("var ");
        pos2 = expression.find("=", pos1);

        mstring name;
        mstring expression;
        if (mstring::npos == pos2) {
            name = expression.substr(pos1, expression.size() - pos1);
        } else {
            name = expression.substr(pos1, pos2 - pos1);
            expression = expression.substr(pos2 + 1, str.size() - pos2 - 1);
        }

        desc = ParserStr(expression);
        desc->mVarName = name;
        AddVar(desc);
    } else {
        //消除括号
        size_t lastPos = 0;
        while (true) {
            pos1 = expression.find('(', lastPos);
            if (mstring::npos == pos1)
            {
                break;
            }

            pos2 = CScriptHlpr::FindNextBracket('(', ')', expression, pos1);
            lastStr = expression.substr(pos1 + 1, pos2 - pos1 - 1);
            VariateDesc *desc = ParserStr(lastStr);
            desc->mVarName = GetTempVarName();
            mTempVarSet[desc->mVarName] = desc;
            expression.replace(pos1, pos2 - pos1 + 1, desc->mVarName);
        }

        //消除取值操作符[]
    }
    return desc;
}

CScriptExpReader::CScriptExpReader() {}

CScriptExpReader::~CScriptExpReader() {}

void CScriptExpReader::InitReader() {
    mVarSerial = 0xff12;

    mOperatorSet.insert("+"), mOperatorSet.insert("-"), mOperatorSet.insert("*");
    mOperatorSet.insert("/"), mOperatorSet.insert("%"), mOperatorSet.insert("^");
    mOperatorSet.insert("|"), mOperatorSet.insert("&&"), mOperatorSet.insert("||");
    mOperatorSet.insert(">"), mOperatorSet.insert("<"), mOperatorSet.insert(">=");
    mOperatorSet.insert("<="), mOperatorSet.insert("=="), mOperatorSet.insert("!=");

    mCmdInternal.insert("bp");
    mCmdInternal.insert("g");
    mCmdInternal.insert("gu");
}