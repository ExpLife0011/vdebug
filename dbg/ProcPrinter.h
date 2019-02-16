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

class CProcPrinter {
private:
    CProcPrinter();
    virtual ~CProcPrinter();

public:
    static CProcPrinter *GetInst();
    mstring GetProcStrByAddr(LPVOID startAddr, const ProcDesc &desc) const;
    mstring GetProcStr(const ProcDesc &desc) const;

    mstring GetStructStrByAddr(LPVOID startAddr, const StructDesc *desc) const;
    mstring GetStructStr(const StructDesc *desc) const;

private:
    PrinterNode *GetNodeStruct(const StructDesc *desc) const;
    void FillLineAndRow(PrinterNode *root) const;
    mstring GetStructStrInternal(const StructDesc *desc) const;
};
#endif //PROCPRINTER_PARSER_H_H_