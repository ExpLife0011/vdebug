#ifndef BREAKPOINT_VDEBUG_H_H_
#define BREAKPOINT_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include <map>
#include "mstring.h"
#include "Command.h"
#include "CmdBase.h"

using namespace std;

enum BreakPointType
{
    em_breakpoint_int3,
    em_breakpoint_hard
};

struct BreakPointInfo
{
    BreakPointType m_eBpType;
    DWORD64 m_dwBpAddr;
    ustring m_wstrName;
    CmdUserParam m_vUserContext;

    BreakPointInfo() : m_dwBpAddr(0), m_eBpType(em_breakpoint_int3)
    {}

    bool operator==(const BreakPointInfo &info) const
    {
        return (
            m_eBpType == info.m_eBpType &&
            m_dwBpAddr == info.m_dwBpAddr &&
            m_vUserContext == info.m_vUserContext
            );
    }
};

class CBreakPointMgr
{
public:
    CBreakPointMgr();
    virtual ~CBreakPointMgr();

    BOOL SetBreakPoint(DWORD64 dwAddr, const CmdUserParam *pUserContext = NULL);
    BOOL DeleteBp(DWORD64 dwAddr);
    BOOL EnableBp(DWORD64 dwAddr);
    BOOL DisableBp(DWORD64 dwAddr);

    BOOL SetBreakPointWithLogic(DWORD64 dwAddr, const ustring &wstrName, const ustring &wstrLogic);
    BOOL SetBreakPointWithFile(DWORD64 dwAddr, const ustring &wstrName, const ustring &wstrFile);
    BOOL OnBreakPoint(DWORD64 dwAddr);

protected:
    bool IsBpInCache(DWORD64 dwAddr) const;
    bool PushBreakPoint(const BreakPointInfo &info);
    bool DeleteInCache(DWORD64 dwAddr);
protected:
    static void WINAPI Int3BpCallback();

protected:
    vector<BreakPointInfo> m_vBreakPoints;
};

CBreakPointMgr *GetBreakPointMgr();
#endif