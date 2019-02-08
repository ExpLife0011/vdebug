#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include "MemoryBase.h"

using namespace std;

std::ustring CMemoryBase::MemoryReadStrUnicode(DWORD64 dwAddr, DWORD dwMaxSize) const {
    if (!dwAddr || !dwMaxSize)
    {
        return L"";
    }

    WCHAR cBuffer = 0;
    DWORD dwOffset = 0;
    ustring wstrBuffer;
    while (TRUE)
    {
        DWORD dwRead = 0;
        if (!MemoryReadSafe(dwAddr + dwOffset, (char *)&cBuffer, sizeof(WCHAR), &dwRead))
        {
            break;
        }

        dwOffset += sizeof(WCHAR);
        if (0 == cBuffer)
        {
            return wstrBuffer;
        }

        wstrBuffer += cBuffer;
    }

    return L"";
}

std::mstring CMemoryBase::MemoryReadStrGbk(DWORD64 dwAddr, DWORD dwMaxSize) const {
    if (!dwAddr || !dwMaxSize)
    {
        return "";
    }

    char cBuffer = 0;
    DWORD dwOffset = 0;
    mstring strBuffer;
    while (TRUE)
    {
        DWORD dwRead = 0;
        if (!MemoryReadSafe(dwAddr + dwOffset, (char *)&cBuffer, sizeof(char), &dwRead))
        {
            break;
        }

        dwOffset += sizeof(char);
        if (0 == cBuffer)
        {
            return strBuffer;
        }

        strBuffer += cBuffer;
    }

    return "";
}

std::mstring CMemoryBase::MemoryReadStrUtf8(DWORD64 dwAddr, DWORD dwMaxSize) const {
    return "";
}