#ifndef COMUTIL_COMLIB_H_H_
#define COMUTIL_COMLIB_H_H_
#include <Windows.h>

class ComUtil {
    //char conver
    virtual const char *UtoA(const char *) = 0;
    virtual const char *AtoU(const char *) = 0;
    virtual const wchar_t *AtoW(const char *) = 0;
    virtual const char *WtoA(const wchar_t *) = 0;
    virtual const wchar_t *UtoW(const char *) = 0;
    virtual const char *WtoU(const wchar_t *) = 0;

    //×Ö·û´®·Ö¸î
    struct StrListA {
        char *sub;
        StrListA *next;
    };

    struct StrListW {
        wchar_t *sub;
        StrListW *next;
    };

    virtual StrListA *StrSplitA(const char *src, const char *split) = 0;
    virtual StrListW *StrSplitW(const wchar_t *src, const wchar_t *split) = 0;
    virtual void StrFreeA(StrListA *) = 0;
    virtual void StrFreeW(StrListW *) = 0;

    //ÄÚ´æÊÍ·Å
    virtual void MemoryFree(void *) = 0;
};

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

static HMODULE _GetComLib() {
#if _WIN64 || WIN64
    #define COMLIB_NAME "ComLib64.dll"
#else
    #define COMLIB_NAME "ComLib32.dll"
#endif
    return GetModuleHandleA(COMLIB_NAME);
}

static ComUtil *_GetComUtil() {
    typedef ComUtil *(__stdcall *pfnGetComUtil)();
    pfnGetComUtil pfn = (pfnGetComUtil)GetProcAddress(_GetComLib(), "GetComUtil");
    return pfn();
}

static void _FreeComUtil(ComUtil *p) {
    typedef void (__stdcall *pFreeComUtil)(ComUtil *p);
    pFreeComUtil pfn = (pFreeComUtil)GetProcAddress(_GetComLib(), "FreeComUtil");
    pfn(p);
}

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
#endif //COMUTIL_COMLIB_H_H_