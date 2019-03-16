#pragma once
#include <Windows.h>
#include <set>
#include <map>
#include <vector>
#include "mstring.h"
#include "ScriptDef.h"
#include "ScriptParser.h"

using namespace std;

typedef VariateDesc *(* pfnProcInternal)(vector<VariateDesc *> &paramSet);

struct ProcRegisterInfo {
    mstring mProcName;
    VariateType mReturnType;
    vector<VariateType> mParamType;
    pfnProcInternal mProcInternal;

    ProcRegisterInfo() {
        mReturnType = em_var_pending;
        mProcInternal = NULL;
    }
};

class CScriptExpReader {
    #define SCRIPT_DATA_DATA        0
    #define SCRIPT_DATA_OPERATOR    1
    struct ScriptData {
        int mType;          //0: data, 1: operator
        mstring mContent;   //content
    };

    struct ScriptProc 
    {
        mstring mProcStr;   //ģʽƥ�䵽�ĺ�������
        size_t mStartPos;   //��Դ���е���ʼλ��
        size_t mEndPos;     //��Դ���еĽ���λ��
    };

    friend class CScriptParser;
public:
    static CScriptExpReader *GetInst();
    void SetCache(ScriptCache *cache);
    VariateDesc *ParserExpression(const mstring &expression);

private:
    CScriptExpReader();
    virtual ~CScriptExpReader();

private:
    void InitReader();
    void AddVar(VariateDesc *desc);
    VariateDesc *GetVarByName(const mstring &name) const;
    VariateDesc *ParserStr(const mstring &str);
    size_t GetParamResult(vector<ScriptData> &nodeSet, size_t index);
    VariateDesc *GetDataDesc(const mstring &str);
    //������ʽ�еĺ���
    void ParserProc(mstring &script);
    //��ȡ�ַ����еĺ�������
    vector<ScriptProc> GetProcSet(const mstring &express) const;

    //ģʽʶ��
    bool IsOperator(const mstring &script, size_t pos, mstring &opt) const;
    bool IsVariable(const mstring &str) const;
    bool IsProc(const mstring &str) const;
    bool IsNumber(const mstring &str) const;
    //gbk�ַ����ж�
    bool IsGbkStr(const mstring &str) const;
    //unicode�ַ����ж�
    bool IsUnicodeStr(const mstring &wstr) const;

    VariateDesc *ProcessScriptNode(vector<ScriptData> &nodeSet);
    VariateDesc *ProcessSimpleStr(const mstring &expression);

    VariateDesc *GetGbkDesc(const mstring &str);
    VariateDesc *GetUnicodeDesc(const mstring &str);
    VariateDesc *GetIntDesc(DWORD64 d);
    VariateDesc *GetPendingDesc();
    BOOL GetNumFromStr(const mstring &strNumber, DWORD64 &dwResult) const;
    void ReplaceNode(vector<ScriptData> &nodeSet, size_t pos1, size_t pos2, const ScriptData &newNode) const;
private:
    bool RegisterProc(
        const mstring &procName,
        VariateType returnType,
        const vector<VariateType> &paramSet,
        pfnProcInternal proc
        );
    VariateDesc *CallInternalProc(const mstring &procName, vector<VariateDesc *> &param);

    //internal proc
    static VariateDesc *ProcStrStartWithA(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcStrStartWithW(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcStrSubStrA(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcStrSubStrW(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcStrCatA(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcStrCatW(vector<VariateDesc *> &paramSet);
    static VariateDesc *ProcRunCommand(vector<VariateDesc *> &paramSet);

private:
    map<mstring, ProcRegisterInfo *> mProcSet;
    set<mstring> mOperatorSet;
    set<mstring> mCmdInternal;
    ScriptCache *mCache;

    static DWORD msVarSerial;
    map<mstring, VariateDesc *> mTempVarSet;    //��ʱ�������棬������
};