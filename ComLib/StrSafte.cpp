#include "StrSafe.h"
#include "LockBase.h"
#include <map>
#include <string>

using namespace std;

static CCriticalSectionLockable gs_lock;
static map<DWORD, string> gs_cache1;
static map<DWORD, wstring> gs_cache2;

const char *__stdcall SafeStrCopyA(const char *str) {
    CScopedLocker lock(&gs_lock);
    string &sp = gs_cache1[GetCurrentThreadId()];
    sp = str;
    return sp.c_str();
}

const wchar_t *__stdcall SafeStrCopyW(const wchar_t *str) {
    CScopedLocker lock(&gs_lock);
    wstring &sp = gs_cache2[GetCurrentThreadId()];
    sp = str;
    return sp.c_str();
}