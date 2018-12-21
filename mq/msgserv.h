#ifndef MSGSERV_DPMSG_H_H_
#define MSGSERV_DPMSG_H_H_
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>

using namespace std;

class CMsgClient;

class ServEvent {
public:
    virtual void OnServAccept(SOCKET client) = 0;
    virtual void OnServRecvData(SOCKET client, const string &strRecved, string &strResp) = 0;
    virtual void OnServSocketErr(SOCKET client) = 0;
    virtual void OnServSocketClose(SOCKET client) = 0;
};

class CMsgServ {
public:
    CMsgServ();
    bool InitServ(unsigned short uLocalPort, ServEvent *pListener);
    void Close();

private:
    bool SetKeepAlive();
    bool Bind(const string &strIp, unsigned short uPort, int iTimeOut);
    static DWORD WINAPI ServThread(LPVOID pParam);

private:
    ServEvent *m_pListener;
    bool m_bInit;
    bool m_bStop;
    HANDLE m_hServThread;
    SOCKET m_servSocket;
    unsigned short m_uLocalPort;
    vector<SOCKET> m_clientSet;
};
#endif