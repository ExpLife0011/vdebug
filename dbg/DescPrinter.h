#ifndef PROCPRINTER_PARSER_H_H_
#define PROCPRINTER_PARSER_H_H_
#include <Windows.h>
#include "DescParser.h"

struct PrinterNode {
    mstring mOffset;
    mstring mAddr;
    mstring mType;
    mstring mName;
    mstring mContent;

    int mSubSize;
    PrinterNode *mParent;
    PrinterNode *mLastBrotherNode;
    PrinterNode *mNextBrotherNode;

    list<PrinterNode *> mSubNodes;
    int mLine;
    int mRow;

    PrinterNode() {
        mSubSize = 0;
        mParent = NULL;
        mLastBrotherNode = NULL;
        mNextBrotherNode = NULL;
        mLine = 0;
        mRow = 0;
    }
};

struct PrintEnumInfo {
    //����mParent�ֶ���Ϊ�˴���ṹ��ָ��.
    //�ṹ���Աֱ��ָ��ṹ��ָ�����������һ������ڵ�
    PrinterNode *mParent;
    PrinterNode *mNode;
    const StructDesc *mDesc;
    LPVOID mBaseAddr;

    PrintEnumInfo() {
        mParent = NULL;
        mNode = NULL;
        mDesc = NULL;
        mBaseAddr = NULL;
    }
};

/*
print with addr
Test0:
����0x0113f358 UINT cbSize 0xffffffff(4294967295)
����0x0113f35c PTest1 test1 0x0113F3B4
��             ����0x0113f3b4 int test1a 0x00000002(2)
��             ����0x0113f3b8 PTest5 pTest5 0x0113F3C8
��             ��             ����0x0113f3c8 int b1 0x00000009(9)
��             ��             ����0x0113f3cc int b2 0x0000000b(11)
��             ����0x0113f3bc LPCSTR test1b aaaa
����0x0113f360 UINT style 0x00000000(0)

print desc only
Test0:
����0x00 UINT cbSize
����0x04 PTest1 test1
��       ����0x00 int test1a
��       ����0x04 PTest5 pTest5
��       ��       ����0x00 int b1
��       ��       ����0x04 int b2
��       ����0x08 LPCSTR test1b
����0x08 UINT style
*/
class CDescPrinter {
private:
    CDescPrinter();
    virtual ~CDescPrinter();

public:
    static CDescPrinter *GetInst();
    mstring GetProcStrByAddr(const mstring &name, LPVOID stackAddr) const;
    mstring GetProcStr(const mstring &name) const;

    mstring GetStructStrByAddr(const mstring &name, LPVOID startAddr) const;
    mstring GetStructStr(const mstring &name) const;
private:
    void StructHandler(PrintEnumInfo &tmp1, list<PrintEnumInfo> &enumSet, bool withOffset) const;
    PrinterNode *GetNodeStruct(const mstring &name, LPVOID baseAddr) const;
    void FillLineAndRow(PrinterNode *root, vector<PrinterNode *> &result, int &line) const;
    mstring GetStructStrInternal(const mstring &name, LPVOID baseAddr) const;
    void LinkDetachedNode(const vector<PrinterNode *> &nodeSet, vector<mstring> &strSet) const;
    bool IsValidAddr(LPVOID addr) const;
};
#endif //PROCPRINTER_PARSER_H_H_