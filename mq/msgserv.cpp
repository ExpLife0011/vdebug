#include "msgserv.h"
#include <MSTcpIP.h>
#include <vector>
#include <ComLib/logger.h>

using namespace std;

CMsgServ::CMsgServ() {
    m_pListener = NULL;
    m_bInit = FALSE;
    m_bStop = FALSE;
    m_hServThread = NULL;
    m_servSocket = INVALID_SOCKET;
    m_uLocalPort = 0;
}

bool CMsgServ::SetKeepAlive()
{
    tcp_keepalive live = {0};
    tcp_keepalive liveout = {0};
    live.keepaliveinterval = 1000 * 10;
    live.keepalivetime = 1000 * 10;
    live.onoff = TRUE;
    int iKeepLive = 1;
    int iRet = setsockopt(m_servSocket, SOL_SOCKET, SO_KEEPALIVE,(char *)&iKeepLive, sizeof(iKeepLive));
    if(iRet == 0){
        DWORD dw;
        if(WSAIoctl(m_servSocket, SIO_KEEPALIVE_VALS, &live, sizeof(live), &liveout, sizeof(liveout), &dw, NULL, NULL)== SOCKET_ERROR)
        {
            return false;
        }
    }
    return true;
}

bool CMsgServ::InitServ(unsigned short uLocalPort, ServEvent *pListener) {
    m_servSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == m_servSocket)
    {
        return false;
    }

    bool bResult = false;
    do
    {
        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(uLocalPort);
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
        if(bind(m_servSocket,(LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
        {
            break;
        }

        //监听
        if(listen(m_servSocket, 5) == SOCKET_ERROR)
        {
            break;
        }

        /**
        ULONG NonBlock = 1;
        if(ioctlsocket(m_servSock, FIONBIO, &NonBlock) == SOCKET_ERROR)
        {
            break;
        }
        */
        SetKeepAlive();
        m_pListener = pListener;
        m_hServThread = CreateThread(NULL, 0, ServThread, this, 0, NULL);
        bResult = true;
        m_bInit = true;
    } while (false);

    if (!bResult && INVALID_SOCKET != m_servSocket)
    {
        closesocket(m_servSocket);
        m_servSocket = INVALID_SOCKET;
    }
    return bResult;
}

void CMsgServ::Close() {
    return;
}

DWORD CMsgServ::ServThread(LPVOID pParam)
{
    CMsgServ *pThis = (CMsgServ *)pParam;

    FD_SET writeSet;
    FD_SET readSet;
    FD_SET errSet;
    vector<SOCKET>::const_iterator it;
    int iBufSize = (1024 * 1024 * 4);
    char *buffer = new char[iBufSize];

    while (true)
    {
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&errSet);

        FD_SET(pThis->m_servSocket, &readSet);
        FD_SET(pThis->m_servSocket, &errSet);
        for (it = pThis->m_clientSet.begin() ; it != pThis->m_clientSet.end() ; it++)
        {
            FD_SET(*it, &readSet);
            FD_SET(*it, &writeSet);
            FD_SET(*it, &errSet);
        }

        int res = 0;
        if ((res = select(0, &readSet, NULL, &errSet, NULL)) != SOCKET_ERROR)
        {
            if (SOCKET_ERROR == res)
            {
                LOGGER_ERROR(L"select err:%d", WSAGetLastError());
                break;
            }

            if (FD_ISSET(pThis->m_servSocket, &errSet))
            {
                LOGGER_ERROR(L"server socket err:%d", WSAGetLastError());
                break;
            }

            if (FD_ISSET(pThis->m_servSocket, &readSet))
            {
                //新连接
                SOCKET client = accept(pThis->m_servSocket, NULL, NULL);
                if (client != INVALID_SOCKET)
                {
                    pThis->m_clientSet.push_back(client);
                    pThis->m_pListener->OnServAccept(client);
                }
            }

            for (it = pThis->m_clientSet.begin() ; it != pThis->m_clientSet.end() ;)
            {
                bool bDelete = false;
                SOCKET sock = *it;
                if (FD_ISSET(sock, &readSet))
                {
                    {
                        //接收数据
                        int iSize = recv(sock, buffer, iBufSize, 0);
                        if (iSize > 0)
                        {
                            string strResp;
                            pThis->m_pListener->OnServRecvData(sock, string(buffer, iSize), strResp);
                            if (strResp.size() > 0)
                            {
                                ::send(sock, strResp.c_str(), static_cast<int>(strResp.size()), 0);
                            }
                        }
                        //接收出错
                        else
                        {
                            LOGGER_ERROR(L"recv data err:%d", WSAGetLastError());
                            pThis->m_pListener->OnServSocketClose(sock);
                            closesocket(sock);
                            bDelete = true;
                        }
                    }
                }

                if (FD_ISSET(sock, &errSet))
                {
                    LOGGER_ERROR(L"client socket err:%d", WSAGetLastError());
                    pThis->m_pListener->OnServSocketClose(sock);
                    closesocket(sock);
                    bDelete = true;
                }

                if (bDelete)
                {
                    it = pThis->m_clientSet.erase(it);
                    continue;
                }
                it++;
            }
        }
    }

    if (pThis->m_servSocket != INVALID_SOCKET)
    {
        LOGGER_PRINT(L"serv socket close");
        closesocket(pThis->m_servSocket);
        pThis->m_servSocket = INVALID_SOCKET;
    }
    delete []buffer;
    return 0;
}