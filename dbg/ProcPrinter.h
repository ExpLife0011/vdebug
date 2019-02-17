#ifndef PROCPRINTER_PARSER_H_H_
#define PROCPRINTER_PARSER_H_H_
#include <Windows.h>
#include "ProcParser.h"

struct PrinterNode {
    mstring mName;
    int mSubSize;
    PrinterNode *mParent;
    PrinterNode *mLastBrotherNode;
    PrinterNode *mNextBrotherNode;

    list<PrinterNode *> mSubNodes;
    int mLine;
    int mRow;
    mstring mShow;

    PrinterNode() {
        mSubSize = 0;
        mParent = mLastBrotherNode = mNextBrotherNode = NULL;
        mLine = 0;
        mRow = 0;
    }
};

/*
kernel32!CreateFileW:
0x1122aabb  lpFilePath(LPCWSTR) = "c:\\abcdef\\2.txt"
0x12343434  lpSecurityAttributes(LPSECURITY_ATTRIBUTES) = 0x1122abcd
              ©¸---0x12343410  SECURITY_ATTRIBUTES0x12343410
                                ©¸---0x12343410  nLength(DWORD) = 0x12341234
                                ©¸---0x12343414  lpSecurityDescriptor(LPVOID) = 0xaabb1234
                                ©¸---0x12343418  bInheritHandle(BOOL) = FALSE
0x1323aabb  dwShareMode(DWORD) = 0x1122aabb
*/
class CProcPrinter {
private:
    CProcPrinter();
    virtual ~CProcPrinter();

public:
    static CProcPrinter *GetInst();
    mstring GetProcStrByAddr(LPVOID startAddr, const ProcDesc &desc) const;
    mstring GetProcStr(const ProcDesc &desc) const;

    mstring GetStructStrByAddr(LPVOID startAddr, const StructDesc *desc) const;
    mstring GetStructStr(const mstring &name) const;

private:
    PrinterNode *GetNodeStruct(const mstring &name, LPVOID baseAddr) const;
    void FillLineAndRow(PrinterNode *root, vector<PrinterNode *> &result, int &line) const;
    mstring GetStructStrInternal(const mstring &name, LPVOID baseAddr) const;
    void LinkDetachedNode(const vector<PrinterNode *> &nodeSet, vector<mstring> &strSet) const;
};
#endif //PROCPRINTER_PARSER_H_H_