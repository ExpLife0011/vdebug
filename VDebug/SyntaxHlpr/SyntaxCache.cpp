#include "SyntaxCache.h"
#include <assert.h>
#include <Shlwapi.h>

using namespace std;

#define TIMER_CACHE     (WM_USER + 7001)
map<HWND, CSyntaxCache *> CSyntaxCache::msTimerCache;

CSyntaxCache::CSyntaxCache() {
    mInterval = 0;

    InitializeCriticalSection(&mCacheLocker);
}

CSyntaxCache::~CSyntaxCache() {
}

void CSyntaxCache::Lock() {
    EnterCriticalSection(&mCacheLocker);
}
void CSyntaxCache::UnLock() {
    LeaveCriticalSection(&mCacheLocker);
}

bool CSyntaxCache::InitCache(int interval) {
    mInterval = interval;

    HWND hwnd = SyntaxTextView::GetWindow();
    assert(IsWindow(hwnd));

    msTimerCache[hwnd] = this;
    SetTimer(hwnd, TIMER_CACHE, interval, TimerCache);
    return true;
}

void CSyntaxCache::TimerCache(HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
{
    if (TIMER_CACHE == id)
    {
        map<HWND, CSyntaxCache *>::iterator it = msTimerCache.find(hwnd);

        if (it == msTimerCache.end())
        {
            return;
        }

        CSyntaxCache *ptr = it->second;
        ptr->Lock();
        if (ptr->mCacheContent.empty())
        {
            ptr->UnLock();
            return;
        }

        for (list<DataCacheDesc>::const_iterator ij = ptr->mCacheDesc.begin() ; ij != ptr->mCacheDesc.end() ; ij++)
        {
            const DataCacheDesc &tmp = *ij;
            ptr->AppendText(tmp.mLabel, ptr->mCacheContent.substr(tmp.mStartPos, tmp.mLength));
        }
        ptr->mCacheDesc.clear();
        ptr->mCacheContent.clear();
        ptr->UnLock();
    }
}

void CSyntaxCache::ClearCache() {
    Lock();
    mCacheDesc.clear();
    mCacheContent.clear();
    UnLock();
}

void CSyntaxCache::PushToCache(const std::string &label, const std::string &content) {
    if (content.empty())
    {
        return;
    }

    Lock();
    bool newItem = false;
    size_t lastPos = 0;
    if (!mCacheDesc.empty())
    {
        DataCacheDesc &lastDesc = *mCacheDesc.rbegin();

        //Merge content
        if (lastDesc.mLabel == label)
        {
            lastDesc.mLength += content.size();
        } else {
            newItem = true;
            lastPos = lastDesc.mStartPos + lastDesc.mLength;
        }
    } else {
        newItem = true;
    }

    if (newItem)
    {
        DataCacheDesc desc;
        desc.mLabel = label;
        desc.mStartPos = lastPos;
        desc.mLength = content.size();
        mCacheDesc.push_back(desc);
    }
    mCacheContent += content;
    UnLock();
}
