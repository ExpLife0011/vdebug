#ifndef MSGCLIENT_DPMSG_H_H_
#define MSGCLIENT_DPMSG_H_H_
#include <WinSock2.h>
#include <Windows.h>
#include <string>

using namespace std;

class CMsgClient;

class ClientEvent {
public:
    virtual void OnClientConnect(CMsgClient &client) {}
    virtual void OnClientRecvData(CMsgClient &client, const string &strRecved, string &strResp) = 0;
    virtual void OnClientSocketErr(CMsgClient &client) = 0;
};

class CMsgClient {
public:
    CMsgClient();
    bool InitClient(const string &strIp, unsigned short uPort, ClientEvent *pListener, int iTimeOut);
    bool Send(const string &strMsg) const;
    void Close();

private:
    bool TestConnect();
    bool Connect(const string &strIp, unsigned short uPort, int iTimeOut);
    static DWORD WINAPI RecvThread(LPVOID pParam);

private:
    ClientEvent *m_pListener;
    bool m_bInit;
    bool m_bStop;
    bool m_bTestConnent;
    HANDLE m_hRecvThread;
    SOCKET m_clientSock;
    string m_strSerIp;
    unsigned short m_uServPort;
};
#endif