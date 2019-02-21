#ifndef PROCPARSER_DPSERV_H_H_
#define PROCPARSER_DPSERV_H_H_
#include <Windows.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include "mstring.h"

using namespace std;

class CParserException
{
public:
    CParserException(const mstring &errMsg)
    {
        mErrMsg = errMsg;
    }

    virtual ~CParserException()
    {}

    const char *GetErrorMsg() const
    {
        return mErrMsg.c_str();
    }

private:
    mstring mErrMsg;
};

#define STRUCT_TYPE_BASETYPE    0   //base type eg:int32 int64 char ...
#define STRUCT_TYPE_STRUCT      1   //struct contains some members.
#define STRUCT_TYPE_PTR         2   //struct ptr

typedef mstring (__stdcall *pfnFormatProc)(LPVOID ptr, int length);

struct StructDesc {
    DWORD mType;                    //type 0:single var 1:struct 2:ptr
    mstring mTypeName;              //type name
    DWORD mCheckSum;                //check sum

    //base variable
    int mLength;                    //var length
    mstring mFormat;                //format rule eg: %d, %hs, %ls

    //struct members
    vector<StructDesc *> mMemberSet;//member array
    vector<mstring> mMemberType;    //member type
    vector<mstring> mMemberName;    //member name
    vector<int> mMemberOffset;      //membet offset

    //ptr or array
    StructDesc *mPtr;               //ptr
    int mLinkCount;                 //ptr count 多级指针计数
    StructDesc *mPtrEnd;            //ptr end   多级指针的最后一级类型，用于加载展开
    int mSize;                      //array size
    bool mUnknownType;              //unknow type LPVOID void *
    mstring mEndType;               //end type, cache used

    bool IsStr(bool &isUnicode) const {
        if (mType == STRUCT_TYPE_PTR)
        {
            static set<mstring> sCharSet;
            static set<mstring> sWCharSet;

            if (sCharSet.empty()){
                sCharSet.insert("char"), sCharSet.insert("CHAR");
                sWCharSet.insert("WCHAR"), sWCharSet.insert("wchar_t");
            }

            if (mUnknownType == true)
            {
                return false;
            }

            if (sCharSet.end() != sCharSet.find(mPtr->mTypeName))
            {
                isUnicode = false;
                return true;
            } else if (sWCharSet.end() != sWCharSet.find(mPtr->mTypeName)){
                isUnicode = true;
                return true;
            }
        }
        return false;
    }

    StructDesc() {
        mType = STRUCT_TYPE_BASETYPE;
        mLength = 0;
        mPtr = NULL;
        mUnknownType = false;
        mSize = 0;
        mLinkCount = 0;
        mPtrEnd = NULL;
        mCheckSum = 0;
    }
};

enum ProcCallType {
    em_call_std,
    em_call_cdecl,
    em_call_fast,
    em_call_this
};

struct ParamDesc {
    mstring mParamType;     //param type
    mstring mParamName;     //param name
    StructDesc *mStruct;    //param struct desc
};

struct ReturnDesc {
    StructDesc *mStruct;        //return content
};

struct FunDesc {
    mstring mDllName;           //module name
    mstring mProcName;          //proc name
    ProcCallType mCallType;     //call type
    vector<ParamDesc> mParam;   //param array
    ReturnDesc mReturn;         //return desc
};

#define STR_TYPE_STRUCT         0
#define STR_TYPE_FUNCTION       1
struct NodeStr {
    int mType;
    mstring mContent;
};

struct ModuleDesc {
    mstring mModuleName;
    vector<StructDesc *> mStructSet;
    vector<FunDesc *> mProcSet;
};

class CDescParser {
public:
    static CDescParser *GetInst();
    void InitParser();
private:
    CDescParser();
    virtual ~CDescParser();

public:
    bool ParserModuleProc(
        const mstring &dllName,
        const mstring &procStr,
        vector<FunDesc> &procSet
        );
private:
    list<NodeStr> SplitNodeStr(const mstring &procStr) const;
    NodeStr ParserProcNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const;
    NodeStr ParserStructNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const;
    void ParserPreProcess(mstring &str) const;
    bool IsStructStr(const mstring &str) const;
    bool IsProcStr(const mstring &str) const;
    bool IsPartOpt(char c) const;
    FunDesc ParserSingleProc(const mstring &dllName, const NodeStr &node) const;
    StructDesc GetStructFormName(const mstring &dllName, const mstring &structName) const;
    map<mstring, StructDesc *> ParserSingleStruct(const mstring &dllName, const NodeStr &node) const;
    StructDesc *ParserStructName(const mstring &content, map<mstring, StructDesc *> &out) const;
    bool ParserStructParam(const mstring &content, StructDesc *ptr) const;
    void ClearParamStr(mstring &str) const;
    StructDesc *ParserParamStr(const mstring &str, mstring &type, mstring &name) const;
private:
};
#endif //PROCPARSER_DPSERV_H_H_