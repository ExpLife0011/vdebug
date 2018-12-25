#ifndef PRINTFORMAT_H_H_
#define PRINTFORMAT_H_H_
#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include "ComUtil.h"

using namespace std;

enum PrintFormatStat {
    line_start = 0,
    line_end,
    space
};

class PrintFormater {
public:
    virtual ~PrintFormater(){}
    virtual bool InitRule(const char *type, const char *rule) = 0;
    virtual bool Reset() = 0;
    virtual bool StartSession(const char *type) = 0;
    virtual PrintFormater &operator << (const char*) = 0;
    virtual PrintFormater &operator << (PrintFormatStat stat) = 0;
    virtual bool EndSession() = 0;
    virtual const char *GetResult() = 0;
};

PrintFormater *__stdcall GetPrintFormater();
void __stdcall FreePrintFormater(PrintFormater *p);
#endif