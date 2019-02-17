#ifndef PROCPARSER_DPSERV_H_H_
#define PROCPARSER_DPSERV_H_H_
#include <Windows.h>
#include <vector>
#include <list>
#include <map>
#include <ComStatic/ComStatic.h>

using namespace std;

#define STRUCT_TYPE_BASETYPE    0   //base type eg:int32 int64 char ...
#define STRUCT_TYPE_STRUCT      1   //struct contains some members.
#define STRUCT_TYPE_PTR         2   //struct ptr
#define STRUCT_TYPE_STR         3   //str array type

struct StructDesc {
    DWORD mType;                    //type 0:single var 1:struct 2:ptr
    list<mstring> mNameSet;         //class name set
    mstring mFormat;                //format rule eg: %d, %hs, %ls
    int mLength;                    //var length

    //single var
    mstring mContent;               //var content

    //struct members
    vector<StructDesc *> mMemberSet;//member array
    vector<mstring> mMemberType;    //member type
    vector<mstring> mMemberName;    //member name
    vector<int> mMemberOffset;      //membet offset

    //ptr or array
    StructDesc *mPtr;               //ptr
    int mSize;                      //array size
    bool mUnknownType;              //unknow type LPVOID void *;

    //str array
    StructDesc *mStrPtr;            //str
    int mStrType;                   //0:multibyte str 1:unicode str

    StructDesc() {
        mType = STRUCT_TYPE_BASETYPE;
        mLength = 0;
        mPtr = NULL;
        mStrPtr = NULL;
        mStrType = 0;
        mUnknownType = false;
        mSize = 0;
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
    StructDesc *mStruct;    //return content
};

struct ProcDesc {
    mstring mDllName;           //module name
    mstring mProcName;          //proc name
    ProcCallType mCallType;     //call type
    vector<ParamDesc> mParam;   //param array
    ReturnDesc mReturn;         //return desc
};

#define TYPE_STRUCT         0
#define TYPE_FUNCTION       1
struct NodeStr {
    int mType;
    mstring mContent;
};

struct ModuleDesc {
    mstring mModuleName;
    vector<StructDesc> mStructSet;
    vector<ProcDesc> mProcSet;
};

class CProcParser {
public:
    static CProcParser *GetInst();
    void InitParser();
private:
    CProcParser();
    virtual ~CProcParser();

public:
    bool ParserModuleProc(
        const mstring &dllName,
        const mstring &procStr,
        vector<ProcDesc> &procSet
        );

    StructDesc *FindStructFromName(const mstring &name) const;
private:
    list<NodeStr> SplitNodeStr(const mstring &procStr) const;
    NodeStr ParserProcNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const;
    NodeStr ParserStructNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const;
    void ParserPreProcess(mstring &str) const;
    bool IsStructStr(const mstring &str) const;
    bool IsProcStr(const mstring &str) const;
    bool IsPartOpt(char c) const;
    ProcDesc ParserSingleProc(const mstring &dllName, const NodeStr &node) const;
    StructDesc GetStructFormName(const mstring &dllName, const mstring &structName) const;
    map<mstring, StructDesc *> ParserSingleStruct(const mstring &dllName, const NodeStr &node) const;
    StructDesc *ParserStructName(const mstring &content, map<mstring, StructDesc *> &out) const;
    bool ParserStructParam(const mstring &content, StructDesc *ptr) const;
    void InsertBaseType(int type, const mstring &nameSet, int length, const mstring &fmt);
    bool LinkPtr(const mstring &nameSet, const mstring &linked);
    bool InsertVoidPtr(const mstring &nameSet);
    void ClearParamStr(mstring &str) const;
    StructDesc *ParserParamStr(const mstring &str, mstring &type, mstring &name) const;
    StructDesc *CreatePtrStruct() const;

private:
    list<ModuleDesc> mModuleSet;
    map<mstring, StructDesc *> mStructMap;
};
#endif //PROCPARSER_DPSERV_H_H_