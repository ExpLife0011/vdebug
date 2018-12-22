#include "DbgCtrlTool.h"
#include <ComLib/ComLib.h>

using namespace std;

#define REG_VDEBUG_CACHE    L"SoftWare\\vdebug\\config\\dbgport"

unsigned short CalPortFormUnique(const wstring &unique) {
    DWORD port = RegGetDWORDFromRegW(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str(), 0);
    if (port)
    {
        return (unsigned short)port;
    }

    const char *p = WtoA(unique.c_str());
    unsigned long crc = crc32(p, lstrlenA(p), 0xffffffff);
    port = (6300 + crc % 500);
    return (unsigned short)port;
}