#ifndef MEMORY_VDEBUG_H_H_
#define MEMORY_VDEBUG_H_H_
#include <Windows.h>
#include <ComLib/ComLib.h>
#include "MemoryBase.h"

using namespace std;

class CMemoryProc : public CMemoryBase
{
public:
    CMemoryProc(HANDLE hProcess);
    virtual ~CMemoryProc();
    virtual bool MemoryReadSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const;
private:
    bool MemoryReadPageSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT SIZE_T *pReadSize) const;
protected:
    HANDLE m_hProcess;
};

#endif