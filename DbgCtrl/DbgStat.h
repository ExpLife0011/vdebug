#ifndef DBGSTATUS_DBGCTRL_H_H_
#define DBGSTATUS_DBGCTRL_H_H_
#include <Windows.h>
#include <list>
#include <ComStatic/ComStatic.h>
#include "DbgCtrlCom.h"

struct DbgStat {
    DbggerType mDbggerType;
    DbggerStatus mDbggerStatus;
    DWORD mCurTid;

    DbgStat() {
        mDbggerType = em_dbg_proc86;
        mDbggerStatus = em_dbg_status_init;
        mCurTid = 0;
    }
};

typedef DWORD HDbgStatus;
typedef void (__stdcall *pfnDbgStatusNotifyProc)(const DbgStat &status, void *param);

struct DbgStatRegisterInfo {
    DWORD mIndex;
    void *mParam;
    pfnDbgStatusNotifyProc mNotifyProc;

    DbgStatRegisterInfo() {
        mIndex = 0;
        mParam = NULL;
        mNotifyProc = NULL;
    }
};

class DbgCtrlApi CDbgStatMgr {
public:
    static CDbgStatMgr *GetInst();
    BOOL InitStatMgr(const std::mstring &unique);
    BOOL ReportDbgStatus(const DbgStat &status);
    HDbgStatus RegisterStatusNotify(pfnDbgStatusNotifyProc pfn, void *param);
    void UnRegisterStatusNotify(HDbgStatus);

private:
    CDbgStatMgr();
    virtual ~CDbgStatMgr();
    static DWORD WINAPI NotifyThread(LPVOID pParam);
    DbgStat ParserStatDesc(const mstring &desc) const;
    void OnDispatchStat();

private:
    HANDLE mNotifyThread;
    DWORD mSerial;
    std::mstring mUnique;
    std::mstring mCachePath;
    HANDLE mNotifyEvent;
    std::list<DbgStatRegisterInfo> mRegisterCache;

    std::mstring mTimeStamp;
    std::mstring mStatDesc;
};
#endif //DBGSTATUS_DBGCTRL_H_H_