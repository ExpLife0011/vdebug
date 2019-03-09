#include <Windows.h>
#include <list>
#include <ComLib/ComLib.h>
#include "DescParser.h"
#include "DescCache.h"

CDescParser *CDescParser::GetInst() {
    static CDescParser *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CDescParser();
    }
    return s_ptr;
}

void CDescParser::InitParser() {
}

CDescParser::CDescParser() {
}

CDescParser::~CDescParser() {
}

NodeStr CDescParser::ParserStructNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const {
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
    node.mType = STR_TYPE_STRUCT;
    node.mContent = procStr.substr(startPos, j - startPos);
    endPos = j + 1;
    return node;
}

bool CDescParser::IsPartOpt(char c) const {
    if (c == '\n' || c == '\t' || c == ' ')
    {
        return true;
    }
    return false;
}

NodeStr CDescParser::ParserProcNode(const mstring &procStr, size_t startPos, size_t curPos, size_t &endPos) const {
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
                newNode.mType = STR_TYPE_FUNCTION;
                newNode.mContent = procStr.substr(startPos, endPos - startPos);
                return newNode;
            }
        }
    }
    throw(new CParserException("函数格式错误"));
    return NodeStr();
}

list<NodeStr> CDescParser::SplitNodeStr(const mstring &procStr) const {
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

void CDescParser::ClearParamStr(mstring &str) const {
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

StructDesc *CDescParser::ParserParamStr(const mstring &str, mstring &type, mstring &name) const {
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
        name = tmpName;
        
        pStruct = CDescCache::GetInst()->GetLinkDescByType(count, tmpType);
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

        pStruct = CDescCache::GetInst()->GetStructByName(tmpType);
        if (!pStruct)
        {
            throw(new CParserException(FormatA("未识别的参数类型:%hs", tmpType.c_str())));
        }

        name = tmpName;
    }

    name = tmpName;
    if (name.empty())
    {
        type = str;
        type.trim();
    } else {
        size_t pos3 = str.rfind(name);
        type = str.substr(0, pos3);
        type.trim();
    }
    return pStruct;
}

FunDesc *CDescParser::ParserSingleProc(const mstring &dllName, const NodeStr &node) const {
    FunDesc *proc = new FunDesc();
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

            StructDesc *pDesc = CDescCache::GetInst()->GetStructByName(str1);
            if (!pDesc)
            {
                throw (new CParserException(FormatA("未识别的返回类型 %hs", str1.c_str())));
            }

            proc->mReturn.mReturnType = str1;
            proc->mReturn.mStruct = pDesc;
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
                proc->mProcName = procName;
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

                ParamDesc param;

                if (mstring::npos != tmp.find("LPSECURITY_ATTRIBUTES"))
                {
                    int ee = 123;
                }

                param.mStruct = ParserParamStr(tmp, param.mParamType, param.mParamName);
                proc->mParam.push_back(param);
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

StructDesc *CDescParser::ParserStructName(const mstring &content, map<mstring, StructDesc *> &out) const {
    size_t curPos = -1;
    curPos = content.find("struct");
    if (mstring::npos == curPos)
    {
        throw(new CParserException("struct parser err1"));
    }

    //struct名称不一定存在，先根据内容生成一个临时的
    //这样如果录入相同的struct，临时名也也一样，不会重复录入
    ULONG dd = crc32(content.c_str(), content.size(), 0xffffffff);
    mstring tmpName = FormatA("struct_%u", dd);
    StructDesc *pNewStruct = new StructDesc();
    pNewStruct->mTypeName = tmpName;
    pNewStruct->mType = STRUCT_TYPE_STRUCT;
    out[tmpName] = pNewStruct;

    curPos += lstrlenA("struct");
    size_t startPos = content.find('{', curPos);

    mstring name = content.substr(curPos, startPos - curPos);
    name.trim();

    if (!name.empty())
    {
        StructDesc *pNewStruct = new StructDesc();
        pNewStruct->mTypeName = name;
        pNewStruct->mType = STRUCT_TYPE_STRUCT;
        out[name] = pNewStruct;
    }

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
                int j = 0;
                while (tmp[j] == '*') {
                    j++;
                }

                tmp.erase(0, j);
                tmp.trim();

                StructDesc *ppDesc = CDescCache::GetInst()->GetLinkDescByDesc(j, pNewStruct);
                ppDesc->mTypeName = tmp;
                out[tmp] = ppDesc;
            } else {
                tmp.trim();
                pNewStruct = new StructDesc();
                pNewStruct->mType = STRUCT_TYPE_STRUCT;
                pNewStruct->mTypeName = tmp;

                out[tmp] = pNewStruct;
            }
        }
    }
    return pNewStruct;
}

bool CDescParser::ParserStructParam(const mstring &content, StructDesc *ptr) const {
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

        mstring type, name;
        StructDesc *pDesc = ParserParamStr(tmp, type, name);

        ptr->mMemberSet.push_back(pDesc);
        ptr->mMemberType.push_back(type);
        ptr->mMemberName.push_back(name);
        ptr->mMemberOffset.push_back(offset);
        ptr->mLength += pDesc->mLength;
        offset += pDesc->mLength;
    }
    return true;
}

map<mstring, StructDesc *> CDescParser::ParserSingleStruct(const mstring &dllName, const NodeStr &node) const {
    size_t curPos = -1;
    mstring content = node.mContent;
    content.trim();

    map<mstring, StructDesc *> structSet;
    //parser struct name
    ParserStructName(content, structSet);

    //parser struct param
    StructDesc tmpStruct;
    ParserStructParam(content, &tmpStruct);

    for (map<mstring, StructDesc *>::iterator it = structSet.begin() ; it != structSet.end() ; it++)
    {
        StructDesc *ptr = it->second;
        if (ptr->mType != STRUCT_TYPE_STRUCT)
        {
            continue;
        }

        ptr->mMemberSet = tmpStruct.mMemberSet;
        ptr->mMemberName = tmpStruct.mMemberName;
        ptr->mMemberOffset = tmpStruct.mMemberOffset;
        ptr->mMemberType = tmpStruct.mMemberType;
        ptr->mLength = tmpStruct.mLength;
    }
    return structSet;
}

bool CDescParser::ParserModuleProc(
    const mstring &dllName,
    const mstring &content,
    list<StructDesc *> &structSet,
    list<FunDesc *> &procSet
    )
{
    mstring procStr = content;
    ParserPreProcess(procStr);
    list<NodeStr> nodeSet;

    try {
        CDescCache::GetInst()->ClearTempCache();
        nodeSet = SplitNodeStr(procStr);

        for (list<NodeStr>::const_iterator it = nodeSet.begin() ; it != nodeSet.end() ; it++)
        {
            if (it->mType == STR_TYPE_STRUCT)
            {
                map<mstring, StructDesc *> tmp = ParserSingleStruct(dllName, *it);
                for (map<mstring, StructDesc *>::const_iterator it = tmp.begin() ; it != tmp.end() ; it++)
                {
                    CDescCache::GetInst()->InsertStructToCache(it->second);
                    structSet.push_back(it->second);
                }
            } else if (it->mType == STR_TYPE_FUNCTION)
            {
                FunDesc *proc = ParserSingleProc(dllName, *it);
                proc->mDllName = dllName;
                procSet.push_back(proc);
            }
        }
    } catch (const CParserException *e) {
        mError = e->GetErrorMsg();
        return false;
    }

    if (structSet.empty() && procSet.empty())
    {
        mError = "描述信息格式错误";
        return false;
    }
    return true;
}

mstring CDescParser::GetErrorStr() const {
    return mError;
}

void CDescParser::ParserPreProcess(mstring &str) const {
    str.trim();
    mstring tmp;
    bool spaceFlag = false;
    for (size_t i = 0 ; i < str.size() ; i++) {
        char cur = str[i];
        if (str[i] == '\t')
        {
            cur = ' ';
        }

        if (cur == ' ') {
            if (spaceFlag) {
                continue;
            } else {
                spaceFlag = true;
            }
        } else {
            spaceFlag = false;
        }
        tmp += cur;
    }

    //delete node block
    while (true) {
        size_t pos1 = tmp.find("/*");
        size_t pos2 = tmp.find("//");

        size_t pos3 = min(pos1, pos2);
        if (pos3 == mstring::npos)
        {
            break;
        }

        size_t pos4  = -1;
        if (pos3 == pos1) {
            pos4 = tmp.find("*/", pos3 + 2);

            if (mstring::npos != pos4)
            {
                tmp.erase(pos1, pos4 + 2 - pos1);
            }
        } else if (pos3 == pos2) {
            pos4 = tmp.find("\n", pos2 + 2);
            if (mstring::npos != pos4)
            {
                tmp.erase(pos2, pos4 + 1 - pos2);
            } else {
                tmp.erase(pos2, tmp.size() - pos2);
            }
        }
    }
    str = tmp.trim();
}

bool CDescParser::IsStructStr(const mstring &str) const {
    return true;
}

bool CDescParser::IsProcStr(const mstring &str) const {
    return true;
}

#include <Shlwapi.h>
#include "DescPrinter.h"
#include "DescCache.h"

typedef struct Test5 {
    int b1;
    int b2;
}*PTest5;

typedef struct {
    int test1a;
    PTest5 pTest5;
    LPCSTR test1b;
}*PTest1, Test1;

struct Test0 {
    UINT        cbSize;
    PTest1      test1;
    UINT        style;
    PTest1      test4;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    Test5       mTest5;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCWSTR     lpszMenuName;
    LPCWSTR     lpszClassName;
    HICON       hIconSm;
};

void TestProc() {
    CDescCache::GetInst()->InitDescCache();
    CDescParser *ptr = CDescParser::GetInst();
    ptr->InitParser();

    int tt1 = 123;
    mstring uuu = CDescCache::GetInst()->GetFormatStr("%x(%d)[%d]", (const char *)&tt1, 4);

    char path[256] = {0};
    GetModuleFileNameA(NULL, path, 256);
    PathAppendA(path, "..\\test.txt");

    PFILE_MAPPING_STRUCT pMapping = MappingFileA(path, FALSE, 1024 * 1024 * 8);

    list<StructDesc *> set1;
    list<FunDesc *> set2;
    ptr->ParserModuleProc("kernel32.dll", (const char *)pMapping->lpView, set1, set2);

    Test5 test5 = {0};
    test5.b1 = 9;
    test5.b2 = 11;

    Test1 test1 = {0};
    test1.test1a = 2;
    test1.test1b = "aaaa";
    test1.pTest5 = &test5;

    Test1 test3 = {0};
    test3.test1a = 7;
    test3.test1b = "cccc";

    Test0 test2 = {0};
    test2.cbSize = -1;
    test2.test1 = &test1;
    test2.test4 = &test3;
    test2.mTest5.b1 = 1;
    test2.mTest5.b2 = 5;
    test2.lpszClassName = L"111";
    test2.lpszMenuName = L"bbb";
    mstring strStruct = "Test0\n";
    strStruct += CDescPrinter::GetInst()->GetStructStrByName("Test0", &test2, 0);
    mstring strFunction = CDescPrinter::GetInst()->GetProcStrByName("", "TestFunction", 0);
    OutputDebugStringA("\n");
    OutputDebugStringA(strFunction.c_str());
    CloseFileMapping(pMapping);
}