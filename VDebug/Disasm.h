#ifndef DISASM_VDEBUG_H_H_
#define DISASM_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include "mstring.h"

using namespace std;

struct DisasmInfo
{
    ustring m_wstrAddr;
    DWORD64 m_dwAddr;
    ustring m_wstrOpt;
    ustring m_wstrContent;
    BYTE m_vData[16];

    DisasmInfo()
    {
        m_dwAddr = 0;
        ZeroMemory(m_vData, sizeof(m_vData));
    }
};

class CDisasmParser
{
public:
    CDisasmParser(HANDLE hProcess);

    virtual ~CDisasmParser();

    bool Disasm(DWORD64 dwAddr, DWORD dwMaxSize, vector<DisasmInfo> &vInfo) const;

protected:
    HANDLE m_hProcess;
};

#endif