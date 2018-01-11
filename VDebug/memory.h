#ifndef MEMORY_VDEBUG_H_H_
#define MEMORY_VDEBUG_H_H_
#include <Windows.h>
#include "mstring.h"

using namespace std;

class CMemoryOperator
{
public:
    CMemoryOperator(HANDLE hProcess);

    virtual ~CMemoryOperator();

    bool MemoryReadPageSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT SIZE_T *pReadSize) const;

    bool MemoryReadSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const;

    bool MemoryReadUnSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const;

    ustring MemoryReadStrUnicode(DWORD64 dwAddr, DWORD dwMaxSize) const;

    mstring MemoryReadStrGbk(DWORD64 dwAddr, DWORD dwMaxSize) const;

    mstring MemoryReadStrUtf8(DWORD64 dwAddr, DWORD dwMaxSize) const;

protected:
    HANDLE m_hProcess;
};

#endif