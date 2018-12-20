#include "ComUtil.h"
#include "SyntaxFormat.h"

ComUtil *__stdcall GetComUtil() {
    return NULL;
}

void __stdcall FreeComUtil(ComUtil *p) {
    return;
}

PrintFormater *__stdcall GetPrintFormater() {
    return new SyntaxFormater();
}

void __stdcall FreePrintFormater(PrintFormater *p) {
    delete p;
}