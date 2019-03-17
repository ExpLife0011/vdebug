#ifndef DISASM_VDEBUG_H_H_
#define DISASM_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include "capstone/capstone.h"
#include <ComLib/ComLib.h>

using namespace std;

struct DisasmInfo
{
    mstring mAddrStr;
    DWORD64 mAddr;
    mstring mOpt;
    mstring mContent;
    mstring mByteCode;
    int mByteCount;
    BYTE mByteData[16];

    DisasmInfo()
    {
        mAddr = 0;
        mByteCount = 0;
        ZeroMemory(mByteData, sizeof(mByteData));
    }
};

typedef BOOL (WINAPI *pfnDisasmProc)(const cs_insn *pAsmInfo, LPVOID pParam);

class CDisasmParser
{
public:
    CDisasmParser(HANDLE hProcess);
    virtual ~CDisasmParser();

    bool DisasmUntilReturn(DWORD64 dwAddr, vector<DisasmInfo> &vInfo);
    bool DisasmWithSize(DWORD64 dwAddr, DWORD dwMaxSize, vector<DisasmInfo> &vInfo);

protected:
    bool DisasmInternal(DWORD64 dwAddr, pfnDisasmProc pfn, LPVOID pParam);
    static BOOL WINAPI DisasmCallback(const cs_insn *pAsmInfo, LPVOID pParam);

protected:
    HANDLE m_hProcess;
    BOOL m_bx64;
};

#endif