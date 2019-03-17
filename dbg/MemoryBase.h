#ifndef MEMORYBASE_DBG_H_H_
#define MEMORYBASE_DBG_H_H_
#include <Windows.h>
#include <ComLib/mstring.h>

class CMemoryBase {
public:
    virtual bool MemoryReadSafe(DWORD64 dwAddr, char *szBuffer, DWORD dwBufferSize, IN OUT DWORD *pReadSize) const = 0;
    std::ustring MemoryReadStrUnicode(DWORD64 dwAddr, DWORD dwMaxSize = 1024) const;
    std::mstring MemoryReadStrGbk(DWORD64 dwAddr, DWORD dwMaxSize = 1024) const;
    std::mstring MemoryReadStrUtf8(DWORD64 dwAddr, DWORD dwMaxSize = 1024) const;
};
#endif //MEMORYBASE_DBG_H_H_