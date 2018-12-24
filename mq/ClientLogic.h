#ifndef CLIENTLOGIC_DPMSG_H_H_
#define CLIENTLOGIC_DPMSG_H_H_
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <list>
#include <map>
#include <Comlib/LockBase.h>
#include <Comlib/tpool.h>
#include "msgclient.h"
#include "mq.h"

using namespace std;

struct NotifyProcInfo {
    void *pParam;               //需要传入回调的参数
    PMsgNotify pNotifyProc;     //回调函数地址
    HANDLE_REGISTER index;      //注册索引，用于反注册
};

struct MsgRecvCache {
    wstring m_wstrRoute;      //消息路由标识
    HANDLE m_hNotify;         //消息通知事件
    wstring m_wstrResult;     //消息处理端
};

class CClientLogic : public ClientEvent {
private:
    CClientLogic();
public:
    static CClientLogic *GetInstance();
    bool InitClient(unsigned short port);

    HANDLE_REGISTER Register(const wstring &wstrKey, PMsgNotify pfn, void *pParam);
    bool UnRegister(HANDLE_REGISTER hRegister);
    bool Dispatch(const wstring &wstrKey, const wstring &wstrValue);
    bool DispatchForResult(const wstring &wstrKey, const wstring &wstrValue, wstring &wstrResult, int iTimeOut);

private:
    static DWORD WINAPI ConnectThread(LPVOID pParam);
    list<string> ParsePackage(string &strPackage) const;
    bool DispatchInternal(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const;
    bool DispatchInCache(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const;

private:
    /**
    客户端事件接口
    */
    virtual void OnClientConnect(CMsgClient &client);
    virtual void OnClientRecvData(CMsgClient &client, const string &strRecved, string &strResp);
    virtual void OnClientSocketErr(CMsgClient &client);
    void OnClientRecvComplete(const string &strData);

private:
    bool m_bClientInit;
    bool m_bConnectSucc;
    unsigned short m_port;
    /**
    client缓存相关数据
    */
    CMsgClient m_msgClient;
    CCriticalSectionLockable m_clientLock;      //消息接收端端读写锁
    string m_clientCache;                       //消息接收端封包缓存
    map<wstring, list<NotifyProcInfo>> m_notifyMap;//频道数据接收缓存
    map<wstring, MsgRecvCache *> m_recvCache;   //数据回执缓存
    HANDLE_REGISTER m_curIndex;                 //当前分配的最大索引
    ThreadPoolBase *m_tpool;                    //任务处理线程池
    string m_clientUnique;                      //客户端标识
};
#endif //CLIENTLOGIC_DPMSG_H_H_