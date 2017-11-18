#include "memory.h"
#include "common.h"
#include "TitanEngine/TitanEngine.h"

#define PAGE_SIZE 0x1000

CMemoryOperator::CMemoryOperator(HANDLE hProcess)
{
    m_hProcess = hProcess;
}

CMemoryOperator::~CMemoryOperator()
{}

bool CMemoryOperator::MemoryReadPageSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const
{
    if (dwBufferSize > (PAGE_SIZE - (dwAddr & (PAGE_SIZE - 1))))
    {
        ErrMessage(L"Read Page Memory Error");
        return false;
    }

    return ::MemoryReadSafe(m_hProcess, (LPVOID)dwAddr, szBuffer, dwBufferSize, pReadSize);
}

bool CMemoryOperator::MemoryReadSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const
{
    if (!szBuffer || !dwBufferSize)
    {
        return false;
    }

    DWORD dwOffset = 0;
    DWORD dwRequest = dwBufferSize;
    DWORD dwLeftInFirstPage = (PAGE_SIZE - (dwAddr & (PAGE_SIZE - 1)));
    DWORD dwReadSize = min(dwLeftInFirstPage, dwRequest);

    pReadSize[0] = 0;
    while (dwReadSize)
    {
        DWORD dw = 0;
        MemoryReadPageSafe(dwAddr + dwOffset, szBuffer + dwOffset, dwReadSize, &dw);

        pReadSize[0] += dw;
        if (dw != dwReadSize)
        {
            break;
        }

        dwOffset += dw;
        dwRequest -= dw;
        dwReadSize = min(PAGE_SIZE, dwRequest);
    }
    return true;
}

ustring CMemoryOperator::MemoryReadStrUnicode(DWORD64 dwAddr, DWORD dwMaxSize) const
{
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
        if (dwMaxSize <= (dwOffset / sizeof(WCHAR)))
        {
            return L"";
        }
    }

    return L"";
}