#include <Windows.h>
#include "DbgCommon.h"
#include "symbol.h"

using namespace std;

std::mstring CDbgCommon::GetSystemStr(DWORD majver, DWORD minver, DWORD product) {
    mstring show;
    if (majver <= 4)
    {
        show += "Microsoft Windows NT";
    } else if (majver == 5)
    {
        if (minver == 0)
        {
            show += "Microsoft Windows 2000";
        } else if (minver == 1)
        {
            show += "Microsoft Windows XP";
        } else if (minver == 2)
        {
            show += "Microsoft Server 2003";
        }
    } else if (majver == 6)
    {
        if (minver == 0)
        {
            if (product == VER_NT_WORKSTATION)
            {
                show += "Microsoft Windows Vista";
            } else {
                show += "Microsoft Server 2008";
            }
        } else if (minver == 1)
        {
            if (product == VER_NT_WORKSTATION)
            {
                show += "Microsoft Windows 7";
            } else {
                show += "Microsoft Windows Server 2008 R2";
            }
        } else if (minver == 2)
        {
            if (product == VER_NT_WORKSTATION)
            {
                show += "Microsoft Windows 8";
            } else {
                show += "Microsoft Windows Server 2012";
            }
        } else if (minver == 3)
        {
            if (product == VER_NT_WORKSTATION)
            {
                show += "Microsoft Windows 8.1";
            } else {
                show += "Windows Server 2012 R2";
            }
        }
    } else if (majver == 10)
    {
        if (product == VER_NT_WORKSTATION)
        {
            show += "Microsoft Windows 10";
        } else {
            show += "Windows Server 2016";
        }
    }
    return show;
}

std::mstring CDbgCommon::GetExceptionDesc(DWORD code) {
    mstring exceptionDesc;
    switch (code) {
        case  EXCEPTION_ACCESS_VIOLATION:
            exceptionDesc = "�Ƿ���ַ�����쳣";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            exceptionDesc = "�������Խ���쳣";
        case EXCEPTION_BREAKPOINT:
            exceptionDesc = "�û��ϵ�Int3�쳣";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            exceptionDesc = "���ݶ����쳣";
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:
            exceptionDesc = "���������쳣(�Ƿ���ֵ̫С���޷���ʾΪ��׼����ֵ)";
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            exceptionDesc = "�߳���ͼ��һ��0�ĸ����������һ������ֵ";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            exceptionDesc = "�����쳣";
            break;
        case EXCEPTION_INT_OVERFLOW:
            exceptionDesc = "�������";
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            exceptionDesc = "�����������ִ���쳣";
            break;
        case EXCEPTION_SINGLE_STEP:
            exceptionDesc = "����ִ���쳣";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            exceptionDesc = "ջ����쳣";
            break;
        default:
            exceptionDesc = "�����쳣";
            break;
    }
    return exceptionDesc;
}

std::mstring CDbgCommon::GetSymFromAddr(DWORD64 dwAddr, const mstring &dllName, DWORD64 moduleBase) {
    CTaskSymbolFromAddr task;
    task.mAddr = (DWORD64)dwAddr;
    CSymbolTaskHeader header;
    header.m_dwSize = sizeof(CTaskSymbolFromAddr) + sizeof(CSymbolTaskHeader);
    header.m_eTaskType = em_task_strfromaddr;

    task.mAddr = dwAddr;
    header.m_pParam = &task;
    CSymbolHlpr::GetInst()->SendTask(&header);

    if (header.m_bSucc != TRUE)
    {
        mstring tmp = dllName;
        size_t pos = tmp.rfind('.');

        if (mstring::npos != pos)
        {
            tmp = tmp.substr(0, pos);
        }
        return FormatA("%hs!0x%x", tmp.c_str(), (DWORD)(dwAddr - moduleBase));
    }

    mstring str = task.mDllName;
    size_t pos = str.rfind('.');
    if (mstring::npos != pos)
    {
        str.erase(pos, str.size() - pos);
    }

    mstring symbol = FormatA("%hs!%hs", str.c_str(), task.mSymbol.c_str());
    if (!task.mFilePath.empty())
    {
        symbol += FormatA("  %hs %d", task.mFilePath.c_str(), task.mLineNumber);
    }
    return symbol;
}