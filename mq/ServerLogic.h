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
    ������¼��ӿ�
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
    server�����������
    */
    CMsgServ m_msgServ;
    CCriticalSectionLockable m_servLock;//��Ϣ����˶�д��
    map<SOCKET, string> m_servCache;    //����˷������
    map<string, SOCKET> m_routeCache;   //���·�ɻ���
    list<ClientRegiserCache *> m_RegisterCache;//ע�Ỻ��
};
#endif //SERVERLOGIC_DPMSG_H_H_