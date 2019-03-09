#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include "DescPrinter.h"
#include "DescParser.h"
#include "DescCache.h"

CDescPrinter::CDescPrinter() {
}

CDescPrinter::~CDescPrinter() {
}

CDescPrinter *CDescPrinter::GetInst() {
    static CDescPrinter *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CDescPrinter();
    }
    return s_ptr;
}

mstring CDescPrinter::GetProcStrByName(const mstring &module, const mstring &procName, LPVOID stackAddr) const {
    list<FunDesc *>funDesc = CDescCache::GetInst()->GetFunByName(module, procName);

    if (funDesc.empty())
    {
        return "";
    }

    mstring result;
    for (list<FunDesc *>::const_iterator it = funDesc.begin() ; it != funDesc.end() ; it++)
    {
        result += GetFunctionStrInternal(*it, stackAddr);
    }
    return result;
}

mstring CDescPrinter::GetProcStrByDesc(const FunDesc *desc, LPVOID stackAddr) const {
    return GetFunctionStrInternal(desc, stackAddr);
}

mstring CDescPrinter::GetStructStrByName(const mstring &name, LPVOID startAddr, int startOffset) const {
    StructDesc *desc = CDescCache::GetInst()->GetStructByName(name);
    if (NULL == desc)
    {
        return "";
    }

    return GetStructStrInternal(desc, startAddr, startOffset);
}

mstring CDescPrinter::GetStructStrByDesc(const StructDesc *desc, LPVOID startAddr, int startOffset) const {
    return GetStructStrInternal(desc, startAddr, startOffset);
}

bool CDescPrinter::IsValidAddr(LPVOID addr) const {
    return (ULONGLONG)addr > 0xffff;
}

void CDescPrinter::StructHandler(PrintEnumInfo &tmp1, list<PrintEnumInfo> &enumSet, bool withOffset) const {
    tmp1.mNode->mSubSize = tmp1.mDesc->mMemberSet.size();

    PrinterNode *lastNode = NULL;
    size_t i = 0;
    for (i = 0 ; i != tmp1.mDesc->mMemberSet.size() ; i++)
    {
        PrintEnumInfo tmp2;
        tmp2.mDesc = tmp1.mDesc->mMemberSet[i];
        tmp2.mNode = new PrinterNode();
        tmp2.mParent = tmp2.mNode;
        tmp2.mNode->mParent = tmp1.mParent;
        tmp1.mParent->mSubNodes.push_back(tmp2.mNode);

        tmp2.mNode->mOffset = FormatA("0x%02x", tmp1.mDesc->mMemberOffset[i]);
        tmp2.mNode->mType = tmp1.mDesc->mMemberType[i];
        tmp2.mNode->mName = tmp1.mDesc->mMemberName[i];

        bool next = true;
        if (!withOffset)
        {
            tmp2.mBaseAddr = (LPVOID)((const char *)tmp1.mBaseAddr + tmp1.mDesc->mMemberOffset[i]);
            tmp2.mNode->mAddr = FormatA("0x%08x", tmp2.mBaseAddr);

            bool isUnicode = false;
            if (tmp2.mDesc->mType == STRUCT_TYPE_PTR && tmp2.mDesc->IsStr(isUnicode))
            {
                char *pStr = *((char **)tmp2.mBaseAddr);
                if (!pStr)
                {
                    tmp2.mNode->mContent = "读取字符串内容错误";
                } else {
                    if (isUnicode)
                    {
                        tmp2.mNode->mContent = FormatA("%ls", pStr);
                    } else {
                        tmp2.mNode->mContent = FormatA("%hs", pStr);
                    }
                }
                next = false;
            } else {
                if (tmp2.mDesc->mType != STRUCT_TYPE_STRUCT)
                {
                    if (IsValidAddr(tmp2.mBaseAddr))
                    {
                         tmp2.mNode->mContent = CDescCache::GetInst()->GetFormatStr(tmp2.mDesc->mFormat, (const char *)tmp2.mBaseAddr, tmp2.mDesc->mLength);
                    } else {
                        tmp2.mNode->mContent = "读取地址内容错误";
                    }
                }
            }
        }

        if (lastNode != NULL)
        {
            lastNode->mNextBrotherNode = tmp2.mNode;
            tmp2.mNode->mLastBrotherNode = lastNode;
        }
        lastNode = tmp2.mNode;

        if (next)
        {
            enumSet.push_back(tmp2);
        }
    }
}

//生成节点层次结构
PrinterNode *CDescPrinter::GetNodeStruct(const StructDesc *desc, LPVOID baseAddr) const {
    if (NULL == desc)
    {
        return NULL;
    }

    PrinterNode *root = new PrinterNode();
    list<PrintEnumInfo> enumSet;
    PrintEnumInfo tmp;
    tmp.mParent = root;
    tmp.mDesc = desc;
    tmp.mNode = root;
    tmp.mBaseAddr = baseAddr;

    root->mName = desc->mTypeName;
    bool withOffset = (baseAddr == NULL);
    enumSet.push_back(tmp);

    int i = 0;
    while (!enumSet.empty()) {
        PrintEnumInfo tmp1 = enumSet.front();
        enumSet.pop_front();

        if (tmp1.mDesc->mType == STRUCT_TYPE_STRUCT)
        {
            StructHandler(tmp1, enumSet, withOffset);
        } else if ((tmp1.mDesc->mType == STRUCT_TYPE_PTR) && (tmp1.mDesc->mUnknownType == false))
        {
            if (tmp1.mDesc->mPtr->mType != STRUCT_TYPE_STRUCT) {
                continue;
            }

            PrintEnumInfo tmp3;
            tmp3.mDesc = tmp1.mDesc->mPtr;
            tmp3.mNode = new PrinterNode();
            tmp3.mParent = tmp1.mParent;

            if (withOffset) {
                StructHandler(tmp3, enumSet, withOffset);
            } else {
                if (IsValidAddr(tmp1.mBaseAddr))
                {
                    char *pStr = *((char **)tmp1.mBaseAddr);
                    tmp3.mBaseAddr = pStr;
                    StructHandler(tmp3, enumSet, withOffset);
                }
            }
        }
    }
    return root;
}

void CDescPrinter::FillLineAndRow(PrinterNode *ptr, vector<PrinterNode *> &result, int &line) const {
    if (ptr->mLastBrotherNode != NULL)
    {
        ptr->mRow = ptr->mLastBrotherNode->mRow;
    } else {
        if (ptr->mParent != NULL)
        {
            ptr->mRow = ptr->mParent->mRow + 1;
        }
    }

    ptr->mLine = line++;
    result.push_back(ptr);

    for (list<PrinterNode *>::iterator it = ptr->mSubNodes.begin() ; it != ptr->mSubNodes.end() ; it++)
    {
        PrinterNode *ptr2 = *it;
        FillLineAndRow(ptr2, result, line);
    }
}

void CDescPrinter::LinkDetachedNode(const vector<PrinterNode *> &nodeSet, vector<mstring> &strSet) const {
    for (size_t i = 0 ; i < strSet.size() ; i++)
    {
        PrinterNode *ptr = nodeSet[i + 1];

        if (ptr->mLastBrotherNode && (ptr->mLastBrotherNode->mLine + 1) < ptr->mLine)
        {
            for (int j = ptr->mLastBrotherNode->mLine ; j < ptr->mLine - 1 ; j++)
            {
                mstring last = strSet[i];
                size_t pos1 = last.find("└");
                if (mstring::npos == pos1)
                {
                    pos1 = last.find("├");
                }

                mstring dd = strSet[j];
                dd.replace(pos1, 1, "│");
                strSet[j] = dd;
            }
        }
    }
}

mstring CDescPrinter::GetStructStrInternal(const StructDesc *desc, LPVOID baseAddr, int startOffset) const {
    PrinterNode *root = GetNodeStruct(desc, baseAddr);
    vector<PrinterNode *> lineSet;
    int line = 0;

    FillLineAndRow(root, lineSet, line);

    mstring result;
    int offset = 0;
    bool withOffset = (baseAddr == NULL);

    if (withOffset)
    {
        offset = lstrlenA("└─0x12");
    } else {
        offset = lstrlenA("└─0x1122abcd");
    }

    size_t i = 0;
    vector<mstring> fmtSet;
    for (i = 1 ; i < lineSet.size() ; i++) {
        PrinterNode *ptr = lineSet[i];

        if (!withOffset)
        {
            fmtSet.push_back(FormatA("%hs %hs %hs %hs", ptr->mAddr.c_str(), ptr->mType.c_str(), ptr->mName.c_str(), ptr->mContent.c_str()));
        } else {
            fmtSet.push_back(FormatA("%hs %hs %hs", ptr->mOffset.c_str(), ptr->mType.c_str(), ptr->mName.c_str()));
        }
    }

    PrinterNode *ptr = NULL;
    for (i = 0 ; i < fmtSet.size() ; i++)
    {
        ptr = lineSet[i + 1];

        int curOffset = (ptr->mRow - 1) * offset;
        mstring dd;
        for (int j = 0 ; j < curOffset + startOffset ; j++)
        {
            dd += " ";
        }

        if (ptr->mNextBrotherNode)
        {
            dd += "├─";
        } else {
            dd += "└─";
        }
        dd += fmtSet[i];
        fmtSet[i] = dd;
    }

    LinkDetachedNode(lineSet, fmtSet);

    //result = (lineSet.front()->mName + ":\n");
    for (vector<mstring>::const_iterator ij = fmtSet.begin() ; ij != fmtSet.end() ; ij++)
    {
        result += (*ij + "\n");
    }
    return result;
}

/*
KernelBase!CreateProcessW:
参数列表
param0 LPCTSTR lpApplicationName
param1 LPCTSTR lpProcName
param2 LPCTSTR Test3
        └----0x1234aa00 nLength(INT) = 1234
        └----0x1234aa10 lpSecurityDescriptor(LPVOID) = 0x11ff1234
返回类型
DWORD
*/
mstring CDescPrinter::GetFunctionStrInternal(const FunDesc *procDesc, LPVOID stackAddr) const {
    bool structOnly = (NULL == stackAddr);

    mstring result = FormatA("%hs!%hs:\n", procDesc->mDllName.c_str(), procDesc->mProcName.c_str());
    result += "返回类型\n";
    result += (procDesc->mReturn.mReturnType + "\n");

    if (procDesc->mParam.size() > 0)
    {
        result += "参数列表\n";
        for (size_t i = 0 ; i != procDesc->mParam.size() ; i++)
        {
            ParamDesc param = procDesc->mParam[i];
            if (structOnly)
            {

                result += FormatA("param%d %hs %hs\n", i, param.mParamType.c_str(), param.mParamName.c_str());
                if (param.mStruct->mType == STRUCT_TYPE_STRUCT || param.mStruct->IsStructPtr())
                {
                    result += GetStructStrByDesc(param.mStruct, 0, lstrlenA("param0 ") + 1);
                }
            }
        }
    }
    return result;
}