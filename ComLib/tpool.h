//thread pool lougd 2018 11-26
#ifndef TPOOL_DPMSG_H_H_
#define TPOOL_DPMSG_H_H_
#include <Windows.h>
#include <list>
#include <vector>
#include <string>
#include "ComUtil.h"

using namespace std;

class ThreadRunable {
public:
    virtual ~ThreadRunable() {}
    virtual void run() = 0;
};

class ThreadPoolBase {
public:
    virtual ~ThreadPoolBase() {}
    virtual bool exec(ThreadRunable *runable) = 0;
};

ThreadPoolBase * __stdcall GetThreadPool(int initCount, int maxCount);

void __stdcall DestroyThreadPool();
#endif //TPOOL_DPMSG_H_H_