#ifndef DBGBASE_VDEBUG_H_H_
#define DBGBASE_VDEBUG_H_H_
#include <Windows.h>
#include <DbgHelp.h>
#include <map>
#include <list>
#include <ComLib/ComLib.h>
#include <DbgCtrl/DbgCtrl.h>
#include "TitanEngine/TitanEngine.h"

using namespace std;

typedef DWORD (__stdcall *pfnReadMemoryProc)(IN DWORD64 dwAddr, IN DWORD dwReadLength, OUT char *pBuffer);
typedef DWORD (__stdcall *pfnWriteMemoryProc)(IN DWORD64 dwAddr, IN DWORD dwWriteLength, IN const char *pBuffer);

struct DbgProcModuleProcInfo
{
    ustring m_wstrName;     //名称
    DWORD64 m_dwBaseAddr;   //模块基址
    DWORD64 m_dwProcAddr;   //函数名称
    DWORD64 m_dwType;       //类型

    DbgProcModuleProcInfo()
    {
        m_dwBaseAddr = 0;
        m_dwProcAddr = 0;
        m_dwType = 0;
    }
};

struct DbgModuleInfo
{
    mstring m_strDllPath;      //dll路径
    mstring m_strDllName;      //dll名称
    DWORD64 m_dwBaseOfImage;    //dll基址
    DWORD64 m_dwEndAddr;        //模块结束地址
    DWORD64 m_dwModuleSize;     //模块大小
    HMODULE m_hModule;          //模块句柄
    map<DWORD64, DbgProcModuleProcInfo> m_vProcInfo;   //函数信息

    DbgModuleInfo()
    {
        m_dwBaseOfImage = 0;
        m_dwEndAddr = 0;
        m_dwModuleSize = 0;
        m_hModule = NULL;
    }
};

class CDbgBase
{
public:
    CDbgBase() : m_bX64(FALSE)
    {}

    virtual ~CDbgBase()
    {}

    virtual BOOL Connect(LPCWSTR wszTarget, LPVOID pParam)
    {
        return FALSE;
    }

    virtual BOOL Connect(DWORD dwPid)
    {
        return FALSE;
    }

    virtual BOOL DisConnect()
    {
        return FALSE;
    }

    virtual BOOL IsConnect()
    {
        return m_bConnected;
    }

    DbggerStatus GetStatus()
    {
        return m_eStatus;
    }

    void SetStatus(DbggerStatus eStatus)
    {
        m_eStatus = eStatus;
    }

    BOOL IsDbgProcx64()
    {
        return m_bX64;
    }

    //是否是寄存器
    bool IsRegister(const ustring &wstr) const;
    mstring GetPrintStr(const char *szBuffer, int iSize)
    {
        mstring strOut;
        for (int i = 0 ; i < iSize ;)
        {
            byte letter = szBuffer[i];
            //字符
            if (letter >= 0x20 && letter <= 0x7e)
            {
                strOut += (char)letter;
                i++;
                continue;
            }
            //汉字
            else if (letter >= 0xb0 && letter <= 0xf7)
            {
                if (i < iSize)
                {
                    byte next = szBuffer[i + 1];
                    if (next >= 0xa1 && next <= 0xfe)
                    {
                        strOut += (char)letter;
                        strOut += (char)next;
                        i += 2;
                        continue;
                    }
                }
            }
            //不可打印
            strOut += '.';
            i++;
        }
        return strOut;
    }

    virtual TITAN_ENGINE_CONTEXT_t GetCurrentContext() {
        return TITAN_ENGINE_CONTEXT_t();
    }

protected:
    BOOL ReadMemory(DWORD64 dwAddr, IN OUT DWORD dwLength, LPSTR pBuffer);
    BOOL WriteMemory(DWORD64 dwAddr, LPCSTR pBuffer, DWORD dwLength);
    virtual INT_PTR OnDebugEvent(LPDEBUG_EVENT pDbgEvent)
    {
        return 0;
    }

protected:
    BOOL m_bX64;
    BOOL m_bConnected;
    DbggerStatus m_eStatus;
};
#endif