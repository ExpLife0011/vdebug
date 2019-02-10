#include <Windows.h>
#include "DbgStat.h"

using namespace std;

CDbgStatMgr *CDbgStatMgr::GetInst() {
    static CDbgStatMgr *s_ptr = NULL;

    if (NULL == s_ptr)
    {
        s_ptr = new CDbgStatMgr();
    }
    return s_ptr;
}

CDbgStatMgr::CDbgStatMgr() {
    mNotifyThread = NULL;
    mSerial = 0xff11;
}

BOOL CDbgStatMgr::InitStatMgr(const std::mstring &unique) {
    mUnique = unique;
    mCachePath = FormatA("%hs\\%hs", REG_VDEBUG_STATUS, unique.c_str());
    mstring eventName = FormatA("Global\\%hs_StatusNotify", unique.c_str());
    mNotifyEvent = CreateLowsdEvent(FALSE, FALSE, eventName.c_str());

    return TRUE;
}

CDbgStatMgr::~CDbgStatMgr() {
}

BOOL CDbgStatMgr::ReportDbgStatus(const DbgStat &status) {
    Value root;
    root["dbggerType"] = status.mDbggerType;
    root["dbggerStatus"] = status.mDbggerStatus;
    root["curThread"] = (int)status.mCurTid;

    mstring desc = FastWriter().write(root);
    mstring time = GetCurTimeStr1("%04d-%02d-%02d %02d:%02d:%02d %03d");

    RegSetStrValueA(
        HKEY_LOCAL_MACHINE,
        mCachePath.c_str(),
        "desc",
        desc.c_str()
        );
    RegSetStrValueA(
        HKEY_LOCAL_MACHINE,
        mCachePath.c_str(),
        "stamp",
        desc.c_str()
        );
    SetEvent(mNotifyEvent);
    return TRUE;
}

DbgStat CDbgStatMgr::ParserStatDesc(const mstring &desc) const {
    DbgStat stat;

    if (desc.empty())
    {
        return stat;
    }

    Value root;
    Reader().parse(desc, root);
    stat.mDbggerType = (DbggerType)root["dbggerType"].asInt();
    stat.mDbggerStatus = (DbggerStatus)root["dbggerStatus"].asInt();
    stat.mCurTid = root["curThread"].asInt();
    return stat;
}

HDbgStatus CDbgStatMgr::RegisterStatusNotify(pfnDbgStatusNotifyProc pfn, void *param) {
    DbgStatRegisterInfo newInfo;
    newInfo.mIndex = mSerial++;
    newInfo.mNotifyProc = pfn;
    newInfo.mParam = param;
    mRegisterCache.push_back(newInfo);

    if (!mNotifyThread)
    {
        mNotifyThread = CreateThread(NULL, 0, NotifyThread, NULL, 0, NULL);
    }
    return newInfo.mIndex;
}

void CDbgStatMgr::UnRegisterStatusNotify(HDbgStatus index) {
    for (list<DbgStatRegisterInfo>::const_iterator it = mRegisterCache.begin() ; it != mRegisterCache.end() ; it++)
    {
        if (it->mIndex == index)
        {
            mRegisterCache.erase(it);
            return;
        }
    }
}

void CDbgStatMgr::OnDispatchStat() {
    mstring desc = RegGetStrValueExA(HKEY_LOCAL_MACHINE, mCachePath.c_str(), "desc");

    if (!desc.empty() && desc != mStatDesc)
    {
        DbgStat stat = ParserStatDesc(desc);
        for (list<DbgStatRegisterInfo>::const_iterator it = mRegisterCache.begin() ; it != mRegisterCache.end() ; it++)
        {
            it->mNotifyProc(stat, it->mParam);
        }
        mStatDesc = desc;
    }
}

DWORD CDbgStatMgr::NotifyThread(LPVOID pParam) {
    while (TRUE) {
        GetInst()->OnDispatchStat();

        WaitForSingleObject(GetInst()->mNotifyEvent, 5000);
    }
}