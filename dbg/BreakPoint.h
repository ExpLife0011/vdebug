#ifndef BREAKPOINT_VDEBUG_H_H_
#define BREAKPOINT_VDEBUG_H_H_
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include "CmdBase.h"

using namespace std;

enum ProcDbgBreakPointStat
{
    em_bp_enable,       //断点启用
    em_bp_disable,      //断点禁用
    em_bp_uneffect      //断点尚未生效
};

enum BreakPointType
{
    em_breakpoint_int3,
    em_breakpoint_hard
};

struct BreakPointInfo
{
    DWORD mSerial;
    BreakPointType mBpType;
    DWORD64 mBpAddr;
    mstring mSymbol;
    HUserCtx mUserContext;
    ProcDbgBreakPointStat mBpStat;

    BreakPointInfo() : mBpAddr(0), mBpType(em_breakpoint_int3) {
        mSerial = 0;
        mBpStat = em_bp_enable;
    }

    bool operator==(const BreakPointInfo &info) const
    {
        return (
            mBpType == info.mBpType &&
            mBpAddr == info.mBpAddr &&
            mUserContext == info.mUserContext
            );
    }
};

class CBreakPointMgr
{
public:
    CBreakPointMgr();
    virtual ~CBreakPointMgr();

    BOOL SetBreakPoint(DWORD64 dwAddr, HUserCtx ctx);
    BOOL DeleteBpByIndex(int index);
    BOOL DeleteAllBp();
    BOOL EnableBpByIndex(int index);
    BOOL DisableBpByIndex(int index);
    vector<BreakPointInfo> GetBpSet() const;
    std::mstring GetLastErr() const;
    BOOL GetBpByAddr(DWORD64 addr, BreakPointInfo &info) const;

    BOOL SetBreakPointWithLogic(DWORD64 dwAddr, const ustring &wstrName, const ustring &wstrLogic);
    BOOL SetBreakPointWithFile(DWORD64 dwAddr, const ustring &wstrName, const ustring &wstrFile);
protected:
    BOOL OnBreakPoint(DWORD64 dwAddr);
    bool IsBpInCache(DWORD64 dwAddr) const;
    bool PushBreakPoint(const BreakPointInfo &info);
    bool DeleteInCache(DWORD64 dwAddr);
protected:
    static void WINAPI Int3BpCallback();

protected:
    static DWORD msSerial;
    vector<BreakPointInfo> mBreakPoints;
    std::mstring mLastErr;
};

CBreakPointMgr *GetBreakPointMgr();
#endif