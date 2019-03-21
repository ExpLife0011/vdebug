#pragma once
#include <Windows.h>
#include <map>
#include <ComLib/mstring.h>
#include <ComLib/LockBase.h>

using namespace std;

typedef DWORD HUserCtx;
typedef void (WINAPI *pfnUserNotifyProc)(LPVOID pParam);

class CUserContextMgr : public CCriticalSectionLockable {
    struct UserContextInfo
    {
        HANDLE mNotifyEvent;
        pfnUserNotifyProc mPfnCallback;
        LPVOID mParam;

        UserContextInfo() : mParam(NULL), mPfnCallback(NULL)
        {
            mNotifyEvent = NULL;
        }
    };

private:
    CUserContextMgr();
    virtual ~CUserContextMgr();

public:
    static CUserContextMgr *GetInst();

    HUserCtx GetUserCtx(pfnUserNotifyProc pfn = NULL, LPVOID p = NULL);
    void SetUserCtx(HUserCtx);
    void Close(HUserCtx);
    void WaitNotify(HUserCtx ctx, DWORD timeOut = -1);

private:
    void InitCtxMgr();
    bool GetInfoByIndex(HUserCtx ctx, UserContextInfo &info);
    bool DeleteByIndex(HUserCtx ctx);

private:
    DWORD mSerial;
    map<DWORD, UserContextInfo> mCtxMap;
};