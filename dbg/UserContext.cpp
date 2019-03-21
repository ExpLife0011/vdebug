#include "UserContext.h"

CUserContextMgr::CUserContextMgr() {
    mSerial = 0x1f;
}

CUserContextMgr::~CUserContextMgr() {
}

CUserContextMgr *CUserContextMgr::GetInst() {
    static CUserContextMgr *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CUserContextMgr();
    }
    return sPtr;
}

HUserCtx CUserContextMgr::GetUserCtx(pfnUserNotifyProc pfn, LPVOID p) {
    DWORD dwCur = mSerial++;

    UserContextInfo ctx;
    ctx.mNotifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    ctx.mPfnCallback = pfn;
    ctx.mParam = p;
    mCtxMap[dwCur] = ctx;
    return dwCur;
}

bool CUserContextMgr::GetInfoByIndex(HUserCtx ctx, UserContextInfo &info) {
    CScopedLocker locker(this);
    map<DWORD, UserContextInfo>::const_iterator it = mCtxMap.find(ctx);
    if (it == mCtxMap.end())
    {
        return false;
    }

    info = it->second;
    return true;
}

bool CUserContextMgr::DeleteByIndex(HUserCtx ctx) {
    CScopedLocker locker(this);
    map<DWORD, UserContextInfo>::const_iterator it = mCtxMap.find(ctx);
    if (it == mCtxMap.end())
    {
        return false;
    }

    mCtxMap.erase(it);
    return true;
}

void CUserContextMgr::SetUserCtx(HUserCtx ctx) {
    UserContextInfo info;
    if (!GetInfoByIndex(ctx, info))
    {
        return;
    }

    if (info.mNotifyEvent)
    {
        SetEvent(info.mNotifyEvent);
    }

    if (info.mPfnCallback)
    {
        info.mPfnCallback(info.mParam);
    }
}

void CUserContextMgr::Close(HUserCtx ctx) {
    CScopedLocker locker(this);
    UserContextInfo info;
    if (!GetInfoByIndex(ctx, info))
    {
        return;
    }

    DeleteByIndex(ctx);
}

void CUserContextMgr::WaitNotify(HUserCtx ctx, DWORD timeOut) {
    UserContextInfo info;
    if (!GetInfoByIndex(ctx, info))
    {
        return;
    }

    WaitForSingleObject(info.mNotifyEvent, timeOut);
}