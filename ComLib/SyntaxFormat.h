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

static PrintFormater *_GetPrintFormater() {
    typedef PrintFormater *(__stdcall *pfnGetPrintFormater)();
    pfnGetPrintFormater pfn = (pfnGetPrintFormater)GetProcAddress(_GetComLib(), "GetPrintFormater");
    return pfn();
}

static void _FreePrintFormater(PrintFormater *p) {
    typedef void (__stdcall *pFreePrintFormater)(PrintFormater *p);
    pFreePrintFormater pfn = (pFreePrintFormater)GetProcAddress(_GetComLib(), "FreePrintFormater");
    pfn(p);
}
#endif