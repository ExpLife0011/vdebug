#include "Disasm.h"
#include "memory.h"
//#include "MainView.h"
//#include "common.h"
#include <ComLib/ComLib.h>

#define MAX_DISASM_SIZE     (1024 * 1024)
#define MAX_ASMOPT_SIZE     16

enum DisasmTypeInternal
{
    em_disasm_bysize,   //�����ָ������
    em_dissam_byaddr,   //����ൽָ����ַ
    em_disasm_byret     //����ൽ���÷���
};

struct DisasmInfoInternal
{
    DisasmTypeInternal m_eType;     //�Ƿ񷴻�ൽ���÷���
    DWORD m_dwMaxSize;              //��󷴻����ֽ���
    DWORD64 m_dwStartAddr;          //��ʼλ��
    DWORD m_dwCurSize;              //��ǰ�������ֽ���
    BOOL m_bx64;
    vector<DisasmInfo> *m_pOutSet;  //����

    DisasmInfoInternal()
    {
        m_bx64 = FALSE;
        ZeroMemory(this, sizeof(DisasmInfoInternal));
    }
};

CDisasmParser::CDisasmParser(HANDLE hProcess)
{
    m_hProcess = hProcess;
}

CDisasmParser::~CDisasmParser()
{}

BOOL CDisasmParser::DisasmCallback(const cs_insn *pAsmInfo, LPVOID pParam)
{
    DisasmInfoInternal *pInfo = (DisasmInfoInternal *)pParam;
    DisasmInfo info;
    info.mAddr = pAsmInfo->address;
    info.mOpt = pAsmInfo->mnemonic;
    info.mContent = pAsmInfo->op_str;

    info.mAddrStr = FormatA("0x%08x", info.mAddr);

    for (int i = 0 ; i < pAsmInfo->size ; i++)
    {
        info.mByteCode += FormatA("%02x", pAsmInfo->bytes[i]);
    }
    pInfo->m_dwCurSize += pAsmInfo->size;

    if (em_disasm_byret == pInfo->m_eType)
    {
        if (info.mOpt.startwith("nop") || info.mOpt.startwith("int3"))
        {
            return FALSE;
        }
    }
    pInfo->m_pOutSet->push_back(info);

    if (pInfo->m_dwCurSize > MAX_DISASM_SIZE)
    {
        return FALSE;
    }

    if (em_disasm_bysize == pInfo->m_eType)
    {
        if (pInfo->m_dwCurSize >= pInfo->m_dwMaxSize)
        {
            return FALSE;
        }
    }
    else if (em_disasm_byret == pInfo->m_eType)
    {
        if (info.mOpt.startwith("ret"))
        {
            return FALSE;
        }
    }
    return TRUE;
}

bool CDisasmParser::DisasmInternal(DWORD64 dwAddr, pfnDisasmProc pfn, LPVOID pParam)
{
    static char *s_szBuffer = new char[4096];
    static const DWORD dwDisasmSize = 4096;

    DWORD dwReadSize = 0;
    CMemoryOperator memReader(m_hProcess);
    memReader.MemoryReadSafe(dwAddr, s_szBuffer, dwDisasmSize, &dwReadSize);

    if (!dwReadSize)
    {
        return false;
    }
    csh mHandle = NULL;

#if _WIN64 || WIN64
    cs_open(CS_ARCH_X86, CS_MODE_64, &mHandle);
#else
    cs_open(CS_ARCH_X86, CS_MODE_32, &mHandle);
#endif
    cs_option(mHandle, CS_OPT_DETAIL, CS_OPT_ON);

    cs_insn *asmabcd = cs_malloc(mHandle);
    bool bGotoEnd = false;
    uint64_t addr64 = (uint64_t)dwAddr;
    while (TRUE)
    {
        char *pCurPos = s_szBuffer;
        size_t codeSize = dwReadSize;
        while (cs_disasm_iter(mHandle, (const uint8_t **)&pCurPos, &codeSize, (uint64_t *)&addr64, asmabcd))
        {
            if (!pfn(asmabcd, (LPVOID)pParam))
            {
                bGotoEnd = true;
                break;
            }
        }

        if (bGotoEnd)
        {
            break;
        }

        if (codeSize < MAX_ASMOPT_SIZE && CS_ERR_OK == cs_errno(mHandle))
        {
            dwReadSize = 0;
            memReader.MemoryReadSafe(addr64, s_szBuffer, dwDisasmSize, &dwReadSize);

            if (!dwReadSize)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    cs_close(&mHandle);
    return true;
}

bool CDisasmParser::DisasmUntilReturn(DWORD64 dwAddr, vector<DisasmInfo> &vInfo)
{
    DisasmInfoInternal st;
    st.m_eType = em_disasm_byret;
    st.m_dwStartAddr = dwAddr;
    st.m_pOutSet = &vInfo;
    st.m_bx64 = m_bx64;
    DisasmInternal(dwAddr, DisasmCallback, &st);
    return !vInfo.empty();
}

bool CDisasmParser::DisasmWithSize(DWORD64 dwAddr, DWORD dwDisasmSize, vector<DisasmInfo> &vInfo)
{
    DisasmInfoInternal st;
    st.m_eType = em_disasm_bysize;
    st.m_dwMaxSize = dwDisasmSize;
    st.m_dwStartAddr = dwAddr;
    st.m_bx64 = m_bx64;
    st.m_pOutSet = &vInfo;
    DisasmInternal(dwAddr, DisasmCallback, &st);
    return !vInfo.empty();
}