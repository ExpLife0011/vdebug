#include "msgclient.h"
#include <ComLib/logger.h>

#pragma comment(lib, "Ws2_32.lib")

CMsgClient::CMsgClient() :
m_pListener(NULL),
m_bInit(false),
m_bStop(false),
m_hRecvThread(NULL),
m_clientSock(INVALID_SOCKET),
m_uServPort(0),
m_bTestConnent(false) {
}

bool CMsgClient::Connect(const string &strIp, unsigned short uPort, int iTimeOut) {
    unsigned long ul = 1;
    ioctlsocket(m_clientSock, FIONBIO, (unsigned long*)&ul);
    SOCKADDR_IN servAddr ;
    servAddr.sin_family = AF_INET ;
    servAddr.sin_port = htons(uPort);
    servAddr.sin_addr.S_un.S_addr = inet_addr(strIp.c_str());

    connect(m_clientSock, (sockaddr *)&servAddr, sizeof(servAddr));

    struct timeval timeout;
    fd_set r;
    FD_ZERO(&r);
    FD_SET(m_clientSock, &r);
    timeout.tv_sec = iTimeOut / 1000;
    timeout.tv_usec = 0;
    int ret = 0;
    select(0, 0, &r, 0, &timeout);

    bool bStat = false;
    if (FD_ISSET(m_clientSock, &r))
    {
        bStat = true;
    } else {
        bStat = false;
    }
    unsigned long ul1= 0 ;
    ioctlsocket(m_clientSock, FIONBIO, (unsigned long*)&ul1);
    if (false == bStat)
    {
        LOGGER_ERROR(L"ioctlsocket err:%d", WSAGetLastError());
        closesocket(m_clientSock);
        m_clientSock = INVALID_SOCKET;
        return false;
    }
    return true;
}

bool CMsgClient::InitClient(const string &strIp, unsigned short uPort, ClientEvent *pListener, int iTimeOut) {
    if (m_bInit)
    {
        return true;
    }

    m_bStop = false;
    m_clientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == m_clientSock)
    {
        return false;
    }

    m_strSerIp = strIp;
    m_uServPort = uPort;
    m_pListener = pListener;
    if (Connect(strIp, uPort, iTimeOut)) {
        m_bInit = true;
        m_hRecvThread = CreateThread(NULL, 0, RecvThread, this, 0, NULL);
        m_pListener->OnClientConnect(*this);
        return true;
    }
    return false;
}

bool CMsgClient::Send(const string &strMsg) const {
    if (!m_bInit) {
        return false;
    }
    ::send(m_clientSock, strMsg.c_str(), static_cast<int>(strMsg.size()), 0);
    return true;
}

void CMsgClient::Close() {
    if (m_bInit && INVALID_SOCKET != m_clientSock)
    {
        m_bStop = true;
        ::closesocket(m_clientSock);

        if (WAIT_TIMEOUT == WaitForSingleObject(m_hRecvThread, 3000))
        {
            TerminateThread(m_hRecvThread, 0);
        }
        CloseHandle(m_hRecvThread);
        m_hRecvThread = NULL;
        m_clientSock = INVALID_SOCKET;
        m_bInit = false;
    }
}

bool CMsgClient::TestConnect() {
    m_clientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    LOGGER_PRINT(L"test connect ip:%hs, port:%d", m_strSerIp.c_str(), m_uServPort);
    if (Connect(m_strSerIp, m_uServPort, 3000))
    {
        m_bTestConnent = false;
        m_pListener->OnClientConnect(*this);
        return true;
    } else {
        m_bTestConnent = true;
        return false;
    }
}

DWORD CMsgClient::RecvThread(LPVOID pParam) {
    CMsgClient *pThis = (CMsgClient *)pParam;

    char buffer[2048];
    int iRecv = 0;
    while (true) {
        if (pThis->m_bTestConnent)
        {
            if (!pThis->TestConnect())
            {
                LOGGER_ERROR(L"test connect err:%d", WSAGetLastError());
                Sleep(1000);
                continue;
            }
        }

        if ((iRecv = ::recv(pThis->m_clientSock, buffer, sizeof(buffer), 0)) > 0) {
            string strResp;
            pThis->m_pListener->OnClientRecvData(*pThis, string(buffer, iRecv), strResp);

            if (!strResp.empty())
            {
                pThis->Send(strResp);
            }
        } else {
            LOGGER_ERROR(L"recv data err:%d", WSAGetLastError());
            pThis->m_pListener->OnClientSocketErr(*pThis);
            //close for socket err and test connect again
            closesocket(pThis->m_clientSock);
            pThis->m_clientSock = INVALID_SOCKET;
            pThis->m_bTestConnent = true;

            if (pThis->m_bStop) {
                LOGGER_PRINT(L"stop socket");
                break;
            }
        }
    }
    return 0;
}