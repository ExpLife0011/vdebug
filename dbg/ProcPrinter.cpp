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
    return "";
}

//生成节点层次结构
PrinterNode *CProcPrinter::GetNodeStruct(const StructDesc *desc) const {
    PrinterNode *root = new PrinterNode();
    struct PrintEnumInfo {
        PrinterNode *mNode;
        const StructDesc *mDesc;

        PrintEnumInfo() {
            mNode = NULL;
            mDesc = NULL;
        }
    };

    int lineCount = 0;
    list<PrintEnumInfo> enumSet;
    PrintEnumInfo tmp;
    tmp.mDesc = desc;
    tmp.mNode = root;
    root->mLine = lineCount;
    enumSet.push_back(tmp);

    int i = 0;
    while (!enumSet.empty()) {
        PrintEnumInfo tmp1 = enumSet.front();
        enumSet.pop_front();
        tmp1.mNode->mSubSize = tmp1.mDesc->mMemberSet.size();

        int i = 0;
        PrinterNode *lastNode = NULL;
        for (; i != tmp1.mDesc->mMemberSet.size() ; i++)
        {
            PrintEnumInfo tmp2;
            tmp2.mDesc = tmp1.mDesc->mMemberSet[i];
            tmp2.mNode = new PrinterNode();
            tmp2.mNode->mParent = tmp1.mNode;
            tmp1.mNode->mSubNodes.push_back(tmp2.mNode);
            tmp2.mNode->mName = tmp1.mDesc->mMemberName[i];

            if (lastNode != NULL)
            {
                lastNode->mNextBrotherNode = tmp2.mNode;
                tmp2.mNode->mLastBrotherNode = lastNode;
            }
            lastNode = tmp2.mNode;
            enumSet.push_back(tmp2);
        }
    }
    return root;
}

void CProcPrinter::FillLineAndRow(PrinterNode *root) const {
    list<PrinterNode *> enumSet;
    root->mLine = 0;
    root->mRow = 0;
    enumSet.push_back(root);

    while (!enumSet.empty()) {
        PrinterNode *ptr = enumSet.front();
        enumSet.pop_front();

        if (ptr->mLastBrotherNode == NULL)
        {
            if (ptr->mParent != NULL)
            {
                ptr->mLine = ptr->mParent->mLine + 1;
                ptr->mRow = ptr->mParent->mRow + 1;
            }
        } else {
            ptr->mLine = ptr->mLastBrotherNode->mLine + 1;
            ptr->mRow = ptr->mLastBrotherNode->mRow;
        }

        for (list<PrinterNode *>::const_iterator it = ptr->mSubNodes.begin() ; it != ptr->mSubNodes.end() ; it++)
        {
            enumSet.push_back(*it);
        }
    }
}

mstring CProcPrinter::GetStructStrInternal(const StructDesc *desc) const {
    if (desc->mType != TYPE_STRUCT)
    {
        return "";
    }

    PrinterNode *root = GetNodeStruct(desc);
    return "";
}