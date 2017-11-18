#include "Disasm.h"
#include "memory.h"
#include "capstone/capstone.h"
#include "Debugger.h"

CDisasmParser::CDisasmParser(HANDLE hProcess)
{}

CDisasmParser::~CDisasmParser()
{}

bool CDisasmParser::Disasm(DWORD64 dwAddr, DWORD dwMaxSize, vector<DisasmInfo> &vInfo) const
{
    static char *s_szBuffer = new char[1024 * 8];
    static const DWORD s_dwDefaultSize = 1024;

    DWORD dwReadSize = 0;
    CMemoryOperator memReader(m_hProcess);
    memReader.MemoryReadSafe(dwAddr, s_szBuffer, s_dwDefaultSize, &dwReadSize);

    if (!dwReadSize)
    {
        return false;
    }

    csh mHandle = NULL;
    //if (GetDebugger()->GetDebuggerInfo().m_bIsx64Proc)
    //{
    //    cs_open(CS_ARCH_X86, CS_MODE_64, &mHandle);
    //}
    //else
    //{
    //    cs_open(CS_ARCH_X86, CS_MODE_32, &mHandle);
    //}
    cs_option(mHandle, CS_OPT_DETAIL, CS_OPT_ON);

    size_t codeSize = dwReadSize;
    uint64_t addr64 = (uint64_t)s_szBuffer;
    cs_insn *asmabcd = cs_malloc(mHandle);

    while (cs_disasm_iter(mHandle, (const uint8_t **)&s_szBuffer, &codeSize, (uint64_t *)&addr64, asmabcd))
    {
        DisasmInfo info;
        info.m_dwAddr = asmabcd->address;
        info.m_wstrOpt = asmabcd->mnemonic;
        info.m_wstrContent = asmabcd->op_str;

        WCHAR wszBuf[64] = {0};
        _ui64tow(info.m_dwAddr, wszBuf, 16);
        info.m_wstrAddr.format(L"%016ls", wszBuf);
        vInfo.push_back(info);
    }
    cs_close(&mHandle);
    return !vInfo.empty();
}