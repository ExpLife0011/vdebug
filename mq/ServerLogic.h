#ifndef SERVERLOGIC_DPMSG_H_H_
#define SERVERLOGIC_DPMSG_H_H_
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <list>
#include <map>
#include <set>
#include <ComLib/LockBase.h>
#include "msgserv.h"

using namespace std;

struct ClientRegiserCache {
    string mClientUnique;
    SOCKET mClientSocket;

    set<string> mChannels;
};

class CServerLogic : public ServEvent {
private:
    CServerLogic();
public:
    static CServerLogic *GetInstance();
    bool InitServ(unsigned short port);

private:
    list<string> ParsePackage(string &strPackage) const;
    ClientRegiserCache *FindClientInCache(const string &clientUnique) const;
    bool DeleteClientBySocket(SOCKET socket);

private:
    /**
    服务端事件接口
    */
    virtual void OnServAccept(SOCKET client);
    virtual void OnServRecvData(SOCKET client, const string &strRecved, string &strResp);
    virtual void OnServSocketErr(SOCKET client);
    virtual void OnServSocketClose(SOCKET client);
    void OnServRecvComplete(SOCKET client, const string &strData);

private:
    bool m_bServInit;
    unsigned short m_port;

    /**
    server缓存相关数据
    */
    CMsgServ m_msgServ;
    CCriticalSectionLockable m_servLock;//消息服务端读写锁
    map<SOCKET, string> m_servCache;    //服务端封包缓存
    map<string, SOCKET> m_routeCache;   //封包路由缓存
    list<ClientRegiserCache *> m_RegisterCache;//注册缓存
};
#endif //SERVERLOGIC_DPMSG_H_H_