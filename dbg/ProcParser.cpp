#include <Windows.h>
#include <list>
#include "ProcParser.h"

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

CProcParser *CProcParser::GetInst() {
    static CProcParser *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CProcParser();
    }
    return s_ptr;
}

void CProcParser::InsertBaseType(int type, const mstring &nameSet, int length, const mstring &fmt) {
    StructDesc *newPtr = new StructDesc();
    newPtr->mType = type;
    newPtr->mLength = length;
    newPtr->mFormat = fmt;

    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        newPtr->mNameSet.push_back(*it);
        mStructMap[*it] = newPtr;
    }
}

bool CProcParser::InsertVoidPtr(const mstring &nameSet) {
    StructDesc *newPtr = new StructDesc();
    newPtr->mType = STRUCT_TYPE_PTR;
    newPtr->mUnknownType = true;
    newPtr->mFormat = "%p";
    newPtr->mLength = sizeof(void *);

    list<mstring> nameArray = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArray.begin() ; it != nameArray.end() ; it++)
    {
        newPtr->mNameSet.push_back(*it);
        mStructMap[*it] = newPtr;
    }
    return true;
}

bool CProcParser::LinkPtr(const mstring &nameSet, const mstring &linked) {
    map<mstring, StructDesc *>::const_iterator it = mStructMap.find(linked);

    if (it == mStructMap.end())
    {
        return false;
    }

    StructDesc *newPtr = new StructDesc();
    newPtr->mType = STRUCT_TYPE_PTR;
    StructDesc *ptr = it->second;
    list<mstring> nameArry = SplitStrA(nameSet, ";");
    for (list<mstring>::const_iterator it = nameArry.begin() ; it != nameArry.end() ; it++)
    {
        newPtr->mNameSet.push_back(*it);
        newPtr->mPtr = ptr;
        mStructMap[*it] = newPtr;
    }
    return true;
}

void CProcParser::InitParser() {
    //1 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "bool;boolean;byte;unsigned char;UCHAR;INT8;BYTE;__int8", 1, "%d");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "char;CHAR", 1, "%c");

    //2 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WORD;ATOM;unsigned short;USHORT;UINT16;uint16_t;short;int16_t;INT16;__int16;BOOL", 2, "%d");
    InsertBaseType(STRUCT_TYPE_BASETYPE, "WCHAR;wchar_t", 2, "%c");

    //4 byte
    InsertBaseType(STRUCT_TYPE_BASETYPE, "unsigned int;UINT;uint32_t;UINT32;int;INT32;__int32;DWORD", 4, "%d");

    //8 bytes
    InsertBaseType(STRUCT_TYPE_BASETYPE, "__int64;INT64;ULONGLONG;LONGLONG;UINT64;int64_t", 8, "%I64d");

    //str
    InsertBaseType(STRUCT_TYPE_STR, "const char*;char*;LPCSTR;PSTR;LPSTR", sizeof(void*), "%hs");
    InsertBaseType(STRUCT_TYPE_STR, "const wchar_t*;wchar_t*;LPCWSTR;PWSTR;LPWSTR", sizeof(void*), "%ls");

    //void ptr
    InsertVoidPtr("void*;LPVOID;PVOID;HANDLE;HKEY;HWINSTA;HWND;HMENU;HINSTANCE;WNDPROC;HICON;HCURSOR;HBRUSH;HOOKPROC;LPTHREAD_START_ROUTINE");

    //ptr
    LinkPtr("LPBYTE;PBYTE;LPCBYTE;PINT8;LPINT8", "byte");
    LinkPtr("LPDWORD;PDWORD;PINT;PINT32;SIZE_T;size_t;ULONG_PTR;LONG_PTR;LSTATUS", "DWORD");
    LinkPtr("PWORD;LPWORD;PINT16;LPINT16", "WORD");
}

CProcParser::CProcParser() {
}

CProcParser::~CProcParser() {
}

NodeStr CProcParser::ParserStructNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const {
    const int len1 = lstrlenA("struct");
    list<char> charStack;
    bool stat = false;
    bool endFlag = false;
    size_t i = 0;
    size_t j = 0;
    for (j = curPos + len1 ; j < procStr.size() ; j++)
    {
        char curChar = procStr[j];
        if (curChar == '{')
        {
            charStack.push_back('{');
            stat = true;
        }

        if (curChar == '}')
        {
            if (charStack.empty())
            {
                throw (new CParserException("error1"));
            }

            char lastChar = *charStack.rbegin();
            if (lastChar != '{')
            {
                throw (new CParserException("error2"));
            }
            charStack.pop_back();

            if (charStack.empty())
            {
                endFlag = true;
                break;
            }
        }
    }

    if (!endFlag)
    {
        throw (new CParserException("error3"));
    }

    i = j + 1;
    for (j = i  ; j < procStr.size() ; j++)
    {
        if (procStr[j] == ';')
        {
            break;
        }
    }
    NodeStr node;
    node.mType = TYPE_STRUCT;
    node.mContent = procStr.substr(startPos, j - startPos);
    endPos = j + 1;
    return node;
}

bool CProcParser::IsPartOpt(char c) const {
    if (c == '\n' || c == '\t' || c == ' ')
    {
        return true;
    }
    return false;
}

NodeStr CProcParser::ParserProcNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const {
    size_t i = 0;
    size_t j = 0;
    list<char> charStack;
    bool stat = false;
    bool endFlag = false;
    for (i = curPos ; i < procStr.size() ; i++)
    {
        char c = procStr[i];
        if (c == '(')
        {
            charStack.push_back(c);
        }

        if (c == ')')
        {
            if (charStack.empty() || *charStack.rbegin() != '(')
            {
                throw(new CParserException("error4"));
            }

            charStack.pop_back();
            if (charStack.empty())
            {
                size_t tmp = procStr.find(";", i);
                if (tmp == mstring::npos)
                {
                    tmp = procStr.size();
                    endPos = tmp;
                } else {
                    endPos = tmp;
                }

                NodeStr newNode;
                newNode.mType = TYPE_FUNCTION;
                newNode.mContent = procStr.substr(startPos, endPos - startPos);
                return newNode;
            }
        }
    }
    throw(new CParserException("error5"));
}

list<NodeStr> CProcParser::SplitNodeStr(const mstring &procStr) const {
    size_t startPos = 0;
    int type = -1;
    list<NodeStr> nodeSet;

    for (size_t i = 0 ; i < procStr.size() ;)
    {
        size_t endPos = 0;
        if (0 == procStr.compare(i, lstrlenA("struct"), "struct"))
        {
            NodeStr tmp = ParserStructNode(procStr, startPos, i, endPos);
            nodeSet.push_back(tmp);

            i = endPos;
            startPos = i;
        } else if (procStr[i] == '(')
        {
            NodeStr tmp = ParserProcNode(procStr, startPos, i, endPos);
            nodeSet.push_back(tmp);

            i = endPos;
            startPos = i;
        } else {
            i++;
        }
    }
    return nodeSet;
}

void CProcParser::ClearParamStr(mstring &str) const {
    const char *clearStr[] = {
        " const", "const ", " CONST", "CONST ",
        "__in_opt", "__inout_opt", "__out_opt", "__out_bcount_opt", "_bcount", "_opt",
        "__in", "__out", "IN ", " IN", " OUT", "OUT ",
        "near ", " near", "NEAR ", " NEAR", "FAR ", " FAR", "far ", " far"
    };

    for (int i = 0 ; i < RTL_NUMBER_OF(clearStr) ; i++)
    {
        str.delsub(clearStr[i]);
    }
    str.trim();
}

StructDesc *CProcParser::CreatePtrStruct() const {
    StructDesc *ptr = new StructDesc();
    ptr->mType = STRUCT_TYPE_PTR;
    ptr->mFormat = "%p";
    ptr->mLength = sizeof(void *);

    return ptr;
}

StructDesc *CProcParser::ParserParamStr(const mstring &str, mstring &name) const {
    mstring content = str;
    ClearParamStr(content);

    content.repsub("\r", " ");
    content.repsub("\n", " ");
    content.repsub("\t", " ");

    while (mstring::npos != content.find(" *")) {
        content.repsub(" *", "*");
    }

    while (mstring::npos != content.find("* ")) {
        content.repsub("* ", "*");
    }

    content.repsub("void*", "PVOID ");
    content.repsub("VOID*", "PVOID ");
    content.trim();

    mstring tmpType, tmpName;
    size_t i = 0;
    StructDesc *pStruct = NULL;
    //一级或者多级指针
    if (mstring::npos != content.find('*')) {
        size_t count = 0;
        size_t pos1 = mstring::npos;
        size_t pos2 = mstring::npos;
        for (i = 0 ; i < content.size() ; i++) {
            if (content[i] == '*') {
                if (mstring::npos == pos1)
                {
                    pos1 = i;
                }

                pos2 = i;
                count++;
            }
        }

        tmpType = content.substr(0, pos1);
        tmpName = content.substr(pos2 + 1, content.size() - pos2 - 1);
        tmpType.trim();
        tmpName.trim();

        StructDesc *pStructCache = FindStructFromName(tmpType);
        if (pStructCache == NULL)
        {
            throw(new CParserException(FormatA("未识别的参数类型 %hs", content.c_str())));
        }

        name = tmpName;
        StructDesc *root = CreatePtrStruct();
        StructDesc *lastPtr = root;
        for (i = 0 ; i < count - 1 ; i++) {
            StructDesc *tmp = CreatePtrStruct();
            lastPtr->mPtr = tmp;
            lastPtr = tmp;
        }
        lastPtr->mPtr = pStructCache;
        pStruct = root;
    } else {
        size_t pos1 = content.find(' ');

        if (mstring::npos == pos1)
        {
            tmpType = content;
        } else {
            tmpType = content.substr(0, pos1);
            tmpName = content.substr(pos1, content.size() - pos1);
            tmpType.trim();
            tmpName.trim();
        }

        pStruct = FindStructFromName(tmpType);
        if (pStruct == NULL)
        {
            throw(new CParserException(FormatA("未识别的参数类型 %hs", content.c_str())));
        }
        name = tmpName;
    }
    return pStruct;
}

ProcDesc CProcParser::ParserSingleProc(const mstring &dllName, const NodeStr &node) const {
    ProcDesc proc;
    mstring procStr = node.mContent;
    procStr.trim();

    size_t i = 0;
    size_t lastPos = 0;
    int mode = 0; //0:retrun 1:proc name 2:param list
    for (i = 0 ; i < procStr.size() ; i++)
    {
        char c = procStr[i];

        if (0 == mode)
        {
            if (!IsPartOpt(c)) {
                continue;
            }

            mstring str1 = procStr.substr(lastPos, i - lastPos);
            str1.trim();

            StructDesc *pDesc = FindStructFromName(str1);
            if (!pDesc)
            {
                throw (new CParserException(FormatA("未识别的返回类型 %hs", str1.c_str())));
            }
            proc.mReturn.mStruct = pDesc;
            mode++;
            lastPos = i;
        } else if (1 == mode)
        {
            if (c == '(')
            {
                size_t j = i;
                for (j = i ; j > 0 ; j--)
                {
                    if (IsPartOpt(procStr[j]))
                    {
                        break;
                    }
                }

                if (j == 0)
                {
                    throw (new CParserException(FormatA("函数格式错误 %hs", procStr.c_str())));
                }
                mstring procName = procStr.substr(j, i - j);
                procName.trim();
                proc.mProcName = procName;
                mode++;
                lastPos = i;
            }
        } else if (2 == mode)
        {
            size_t endPos = procStr.rfind(')');
            mstring paramStr = procStr.substr(lastPos + 1, endPos - lastPos - 1);

            list<mstring> paramList = SplitStrA(paramStr, ",");
            for (list<mstring>::const_iterator it = paramList.begin() ; it != paramList.end() ; it++)
            {
                mstring tmp = *it;
                ClearParamStr(tmp);

                mstring showName;
                StructDesc *pDesc = ParserParamStr(tmp, showName);

                ParamDesc param;
                param.mParamName = showName;
                param.mStruct = pDesc;
                proc.mParam.push_back(param);
            }
            break;
        }
    }

    if (2 != mode)
    {
        throw (new CParserException(FormatA("无法解析的函数体 %hs", procStr.c_str())));
    }
    return proc;
}

StructDesc *CProcParser::ParserStructName(const mstring &content, map<mstring, StructDesc *> &out) const {
    size_t curPos = -1;
    curPos = content.find("struct");
    if (mstring::npos == curPos)
    {
        throw(new CParserException("struct parser err1"));
    }

    curPos += lstrlenA("struct");
    size_t startPos = content.find('{', curPos);

    mstring name = content.substr(curPos, startPos - curPos);
    name.trim();
    StructDesc *pStruct = new StructDesc();
    StructDesc *pStructPtr = new StructDesc();
    out[name] = pStruct;
    pStruct->mNameSet.push_back(name);

    pStruct->mType = STRUCT_TYPE_STRUCT;
    pStructPtr = CreatePtrStruct();
    pStructPtr->mPtr = pStruct;

    size_t pos1 = content.rfind('}');
    if (content.size() > pos1 + 1)
    {
        mstring endStr = content.substr(pos1 + 1, content.size() - pos1 - 1);
        list<mstring> nameArry = SplitStrA(endStr, ",");

        for (list<mstring>::const_iterator it = nameArry.begin() ; it != nameArry.end() ; it++)
        {
            mstring tmp = *it;
            ClearParamStr(tmp);
            tmp.trim();

            if (tmp.empty())
            {
                continue;
            }

            //暂不考虑多级指针
            if (tmp[0] == '*')
            {
                tmp.erase(0, 1);
                tmp.trim();
                out[tmp] = pStructPtr;
                pStructPtr->mNameSet.push_back(tmp);
            } else {
                out[tmp] = pStruct;
                pStruct->mNameSet.push_back(tmp);
            }
        }
    }
    return pStruct;
}

StructDesc *CProcParser::FindStructFromName(const mstring &name) const {
    map<mstring, StructDesc *>::const_iterator it = mStructMap.find(name);

    if (it == mStructMap.end())
    {
        return NULL;
    }
    return it->second;
}

bool CProcParser::ParserStructParam(const mstring &content, StructDesc *ptr) const {
    size_t pos1 = content.find('{');
    size_t pos2 = content.rfind('}');

    mstring paramStr = content.substr(pos1 + 1, pos2 - pos1 - 1);
    list<mstring> paramSet = SplitStrA(paramStr, ";");
    int offset = 0;
    for (list<mstring>::const_iterator it = paramSet.begin() ; it != paramSet.end() ; it++)
    {
        mstring tmp = *it;
        tmp.trim();

        if (tmp.empty())
        {
            continue;
        }

        mstring name;
        StructDesc *pDesc = ParserParamStr(tmp, name);

        ptr->mMemberSet.push_back(pDesc);
        ptr->mMemberName.push_back(name);
        ptr->mMemberOffset.push_back(offset);
        ptr->mLength += pDesc->mLength;
        offset += pDesc->mLength;
    }
    return true;
}

map<mstring, StructDesc *> CProcParser::ParserSingleStruct(const mstring &dllName, const NodeStr &node) const {
    size_t curPos = -1;
    mstring content = node.mContent;
    content.trim();

    map<mstring, StructDesc *> structSet;
    //parser struct name
    StructDesc *newPtr = ParserStructName(content, structSet);

    //parser struct param
    ParserStructParam(content, newPtr);
    return structSet;
}

bool CProcParser::ParserModuleProc(
    const mstring &dllName,
    const mstring &procStr,
    vector<ProcDesc> &procSet
    )
{
    list<NodeStr> nodeSet;

    try {
        nodeSet = SplitNodeStr(procStr);

        for (list<NodeStr>::const_iterator it = nodeSet.begin() ; it != nodeSet.end() ; it++)
        {
            if (it->mType == TYPE_STRUCT)
            {
                map<mstring, StructDesc *> tmp = ParserSingleStruct(dllName, *it);
                mStructMap.insert(tmp.begin(), tmp.end());
            } else if (it->mType == TYPE_FUNCTION)
            {
                ProcDesc proc = ParserSingleProc(dllName, *it);
                procSet.push_back(proc);
            }
        }
    } catch (const CParserException &e) {
        MessageBoxA(0, 0, e.GetErrorMsg(), 0);
    }
    return true;
}

void CProcParser::ParserPreProcess(mstring &str) const {
    str.trim();
    mstring tmp;
    bool spaceFlag = false;
    for (size_t i = 0 ; i < str.size() ; i++)
    {
        char cur = str[i];
        if (str[i] == '\t')
        {
            cur = ' ';
        }

        if (cur == ' ')
        {
            if (spaceFlag)
            {
                continue;
            } else {
                spaceFlag = true;
            }
        } else {
            spaceFlag = false;
        }
        tmp += cur;
    }
}

bool CProcParser::IsStructStr(const mstring &str) const {
    return true;
}

bool CProcParser::IsProcStr(const mstring &str) const {
    return true;
}

#include <Shlwapi.h>
#include <ComLib/ComLib.h>
#include "ProcPrinter.h"

void TestProc() {
    CProcParser *ptr = CProcParser::GetInst();
    ptr->InitParser();

    char path[256] = {0};
    GetModuleFileNameA(NULL, path, 256);
    PathAppendA(path, "..\\test.txt");

    PFILE_MAPPING_STRUCT pMapping = MappingFileA(path, FALSE, 1024 * 1024 * 8);

    vector<ProcDesc> set1;
    ptr->ParserModuleProc("kernel32.dll", (const char *)pMapping->lpView, set1);
    StructDesc *pDesc = ptr->FindStructFromName("WNDCLASSEXW");

    mstring strStruct = CProcPrinter::GetInst()->GetStructStr(pDesc);
    OutputDebugStringA("\n");
    OutputDebugStringA(strStruct.c_str());
    CloseFileMapping(pMapping);
}