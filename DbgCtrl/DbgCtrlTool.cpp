#include "DbgCtrlTool.h"
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

using namespace std;

#if WIN64 || _WIN64
#define REG_VDEBUG_CACHE    L"SoftWare\WOW6432Node\\vdebug\\config\\dbgport"
#else
#define REG_VDEBUG_CACHE    L"SoftWare\\vdebug\\config\\dbgport"
#endif

unsigned short CalPortFormUnique(const wstring &unique) {
    DWORD port = RegGetDWORDFromRegW(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str(), 0);
    if (port)
    {
        return (unsigned short)port;
    }

    string p = WtoA(unique);
    unsigned long crc = crc32(p.c_str(), p.size(), 0xffffffff);
    port = (6300 + crc % 500);
    return (unsigned short)port;
}