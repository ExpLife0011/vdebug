#include "tpool.h"
#include "LockBase.h"

class ThreadPool : public CCriticalSectionLockable, public ThreadPoolBase {
public:
    ThreadPool(int initCount = 1, int maxCount = 4);
    bool exec(ThreadRunable *runable);
    virtual ~ThreadPool();
    string toString();

private:
    bool CreateNewThread();

private:
    HANDLE m_hWorkNotify;
    HANDLE m_hExitNotify;
    int m_initCount;
    int m_curCount;
    int m_freeCount;
    int m_maxCount;
    list<ThreadRunable *> m_runable;
    vector<HANDLE> m_threadSet;
    static DWORD WINAPI ThreadProc(LPVOID pParam);
};

ThreadPool::ThreadPool(int initCount, int maxCount) {
    m_hWorkNotify = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hExitNotify = CreateEventW(NULL, TRUE, FALSE, NULL);

    m_initCount = initCount;
    m_maxCount = maxCount;
    m_freeCount = initCount;
    m_curCount = initCount;
    for (int i = 0 ; i < m_initCount ; i++)
    {
        HANDLE h = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
        m_threadSet.push_back(h);
    }
}

ThreadPool::~ThreadPool() {
    SetEvent(m_hExitNotify);
    WaitForMultipleObjects((DWORD)m_threadSet.size(), &(m_threadSet[0]), TRUE, 3000);

    for (int i = 0 ; i < (int)m_threadSet.size() ; i++)
    {
        DWORD dw;
        GetExitCodeThread(m_threadSet[i], &dw);

        if (dw == STILL_ACTIVE)
        {
            TerminateThread(m_threadSet[i], 0);
        }
    }
    CloseHandle(m_hWorkNotify);
    CloseHandle(m_hExitNotify);
}

bool ThreadPool::CreateNewThread() {
    CScopedLocker lock(this);
    if ((m_freeCount == 0) && (m_curCount < m_maxCount))
    {
        HANDLE h = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
        m_threadSet.push_back(h);
        m_freeCount++;
        m_curCount++;
        return true;
    }
    return false;
}

bool ThreadPool::exec(ThreadRunable *runable) {
    CScopedLocker lock(this);
    m_runable.push_back(runable);

    if (m_freeCount == 0)
    {
        CreateNewThread();
    }
    SetEvent(m_hWorkNotify);
    return true;
}

string ThreadPool::toString() {
    return "";
}

DWORD ThreadPool::ThreadProc(LPVOID pParam) {
    ThreadPool *pThis = (ThreadPool *)pParam;
    HANDLE arry[] = {pThis->m_hWorkNotify, pThis->m_hExitNotify};
    while (true) {
        DWORD dw = WaitForMultipleObjects(2, arry, FALSE, INFINITE);

        if (dw == WAIT_OBJECT_0)
        {
            while (true) {
                ThreadRunable *pRunable = NULL;
                {
                    CScopedLocker lock(pThis);
                    if (pThis->m_runable.empty())
                    {
                        break;
                    }
                    pThis->m_freeCount--;
                    pRunable = *(pThis->m_runable.begin());
                    pThis->m_runable.pop_front();
                }

                pRunable->run();
                {
                    CScopedLocker lock(pThis);
                    delete pRunable;
                    pThis->m_freeCount++;
                }
            }
        } else {
            break;
        }
    }
    return 0;
}

ThreadPoolBase *__stdcall GetThreadPool() {
    return new ThreadPool();
}

void __stdcall DestroyThreadPool(ThreadPoolBase *p) {
    delete p;
}