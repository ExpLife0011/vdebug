#include <Windows.h>
#include <ComLib/ComLib.h>
#include "ProcPrinter.h"
#include "ProcParser.h"

CProcPrinter::CProcPrinter() {
}

CProcPrinter::~CProcPrinter() {
}

CProcPrinter *CProcPrinter::GetInst() {
    static CProcPrinter *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CProcPrinter();
    }
    return s_ptr;
}

mstring CProcPrinter::GetProcStrByAddr(LPVOID startAddr, const ProcDesc &desc) const {
    return "";
}

mstring CProcPrinter::GetProcStr(const ProcDesc &desc) const {
    return "";
}

mstring CProcPrinter::GetStructStrByAddr(LPVOID startAddr, const StructDesc *desc) const {
    return "";
}

mstring CProcPrinter::GetStructStr(const StructDesc *desc) const {
    return GetStructStrInternal(desc, NULL);
}

//生成节点层次结构
PrinterNode *CProcPrinter::GetNodeStruct(const StructDesc *desc, LPVOID baseAddr) const {
    PrinterNode *root = new PrinterNode();
    struct PrintEnumInfo {
        PrinterNode *mNode;
        const StructDesc *mDesc;

        PrintEnumInfo() {
            mNode = NULL;
            mDesc = NULL;
        }
    };

    list<PrintEnumInfo> enumSet;
    PrintEnumInfo tmp;
    tmp.mDesc = desc;
    tmp.mNode = root;
    root->mName = desc->mNameSet.front();
    bool withOffset = (baseAddr == NULL);
    enumSet.push_back(tmp);

    int i = 0;
    while (!enumSet.empty()) {
        PrintEnumInfo tmp1 = enumSet.front();
        enumSet.pop_front();

        if (tmp1.mDesc->mType == STRUCT_TYPE_STRUCT)
        {
            tmp1.mNode->mSubSize = tmp1.mDesc->mMemberSet.size();

            PrinterNode *lastNode = NULL;
            for (i = 0 ; i != tmp1.mDesc->mMemberSet.size() ; i++)
            {
                PrintEnumInfo tmp2;
                tmp2.mDesc = tmp1.mDesc->mMemberSet[i];
                tmp2.mNode = new PrinterNode();
                tmp2.mNode->mParent = tmp1.mNode;
                tmp1.mNode->mSubNodes.push_back(tmp2.mNode);

                if (withOffset)
                {
                    tmp2.mNode->mName = FormatA("0x%04x %hs", tmp1.mDesc->mMemberOffset[i], tmp1.mDesc->mMemberName[i].c_str());
                }

                if (lastNode != NULL)
                {
                    lastNode->mNextBrotherNode = tmp2.mNode;
                    tmp2.mNode->mLastBrotherNode = lastNode;
                }
                lastNode = tmp2.mNode;
                enumSet.push_back(tmp2);
            }
        } else if ((tmp1.mDesc->mType == STRUCT_TYPE_PTR) && (tmp1.mDesc->mUnknownType == false))
        {
            PrintEnumInfo tmp3;
            tmp3.mDesc = tmp1.mDesc->mPtr;
            tmp3.mNode = new PrinterNode();
            tmp3.mNode->mParent = tmp1.mNode;
            tmp1.mNode->mSubNodes.push_back(tmp3.mNode);

            if (withOffset)
            {
                tmp3.mNode->mName = FormatA("0x0000 %hs", tmp3.mDesc->mNameSet.front().c_str());
            }
            enumSet.push_back(tmp3);
        }
    }
    return root;
}

void CProcPrinter::FillLineAndRow(PrinterNode *ptr, vector<PrinterNode *> &result, int &line) const {
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

void CProcPrinter::LinkDetachedNode(const vector<PrinterNode *> &nodeSet, vector<mstring> &strSet) const {
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

/*
kernel32!CreateFileW:
0x1122aabb  lpFilePath(LPCWSTR) = "c:\\abcdef\\2.txt"
0x12343434  lpSecurityAttributes(LPSECURITY_ATTRIBUTES) = 0x1122abcd
              └---0x12343410  SECURITY_ATTRIBUTES0x12343410
                                └---0x12343410  nLength(DWORD) = 0x12341234
                                └---0x12343414  lpSecurityDescriptor(LPVOID) = 0xaabb1234
                                └---0x12343418  bInheritHandle(BOOL) = FALSE
0x1323aabb  dwShareMode(DWORD) = 0x1122aabb
*/
mstring CProcPrinter::GetStructStrInternal(const StructDesc *desc, LPVOID baseAddr) const {
    if (desc->mType != STRUCT_TYPE_STRUCT)
    {
        return "";
    }

    PrinterNode *root = GetNodeStruct(desc, baseAddr);
    vector<PrinterNode *> lineSet;
    int line = 0;

    FillLineAndRow(root, lineSet, line);

    mstring result;
    int offset = 0;
    bool withOffset = (baseAddr == NULL);

    if (withOffset)
    {
        offset = lstrlenA("└─0x1122");
    }
    
    bool first = true;
    vector<mstring> strSet;
    for (vector<PrinterNode *>::const_iterator it = lineSet.begin() ; it != lineSet.end() ; it++)
    {
        PrinterNode *ptr = *it;
        if (first)
        {
            first = false;
            continue;
        }

        int curOffset = (ptr->mRow - 1) * offset;
        mstring dd;
        for (int i = 0 ; i < curOffset ; i++)
        {
            dd += " ";
        }

        if (ptr->mNextBrotherNode)
        {
            dd += "├─";
        } else {
            dd += "└─";
        }

        dd += ptr->mName;
        strSet.push_back(dd);
    }

    LinkDetachedNode(lineSet, strSet);
    result = (lineSet.front()->mName + ":\n");
    for (vector<mstring>::const_iterator ij = strSet.begin() ; ij != strSet.end() ; ij++)
    {
        result += (*ij + "\n");
    }
    return result;
}