#ifndef PROCPRINTER_PARSER_H_H_
#define PROCPRINTER_PARSER_H_H_
#include <Windows.h>
#include "ProcParser.h"

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
        mParent = mLastBrotherNode = mNextBrotherNode = NULL;
        mLine = 0;
        mRow = 0;
    }
};

struct PrintEnumInfo {
    //加入mParent字段是为了处理结构体指针,结构体成员直接指向结构体指针而非再增加一个对象节点
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
kernel32!CreateFileW:
0x1122aabb  lpFilePath(LPCWSTR) = "c:\\abcdef\\2.txt"
0x12343434  lpSecurityAttributes(LPSECURITY_ATTRIBUTES) = 0x1122abcd
              └---0x12343410  SECURITY_ATTRIBUTES0x12343410
                                └---0x12343410  nLength(DWORD) = 0x12341234
                                └---0x12343414  lpSecurityDescriptor(LPVOID) = 0xaabb1234
                                └---0x12343418  bInheritHandle(BOOL) = FALSE
0x1323aabb  dwShareMode(DWORD) = 0x1122aabb
*/
class CProcPrinter {
private:
    CProcPrinter();
    virtual ~CProcPrinter();

public:
    static CProcPrinter *GetInst();
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
};
#endif //PROCPRINTER_PARSER_H_H_