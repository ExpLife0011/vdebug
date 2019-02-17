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
                    tmp2.mNode->mName = FormatA("0x%04x  %hs", tmp1.mDesc->mMemberOffset[i], tmp1.mDesc->mMemberName[i].c_str());
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
                tmp3.mNode->mName = FormatA("0x0000  %hs", tmp3.mDesc->mNameSet.front().c_str());
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

mstring CProcPrinter::GetStructStrInternal(const StructDesc *desc, LPVOID baseAddr) const {
    if (desc->mType != STRUCT_TYPE_STRUCT)
    {
        return "";
    }

    PrinterNode *root = GetNodeStruct(desc, baseAddr);
    vector<PrinterNode *> lineSet;
    int line = 0;

    FillLineAndRow(root, lineSet, line);

    for (vector<PrinterNode *>::const_iterator it = lineSet.begin() ; it != lineSet.end() ; it++)
    {
        dp(L"%hs", (*it)->mName.c_str());
    }
    return "";
}