#pragma once
#include <Windows.h>
#include <list>
#include <map>
#include <vector>
#include <string>
#include "SyntaxTextView.h"
#include "export.h"

class CSyntaxCache : public SyntaxTextView {
public:
    CSyntaxCache();
    virtual ~CSyntaxCache();

    bool InitCache(int interval);
    void PushToCache(const std::string &label, const std::string &content);

    //clear all cache
    void ClearCache();
private:
    void Lock();
    void UnLock();
    static void CALLBACK TimerCache(HWND hwnd,
        UINT msg,
        UINT_PTR id,
        DWORD time
        );

    struct DataCacheDesc {
        std::string mLabel;
        void *mParam;
        size_t mStartPos;
        size_t mLength;

        DataCacheDesc() {
            mStartPos = 0;
            mLength = 0;
            mParam = NULL;
        }
    };
private:
    int mInterval;
    //cache desc
    std::list<DataCacheDesc> mCacheDesc;
    //cache content
    std::string mCacheContent;
    //cache locker
    CRITICAL_SECTION mCacheLocker;
    static std::map<HWND, CSyntaxCache *> msTimerCache;
};
