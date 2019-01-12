#include "ServerLogic.h"
#include <ComLib/logger.h>
#include "protocol.h"
#include <ComLib/ComLib.h>

CServerLogic::CServerLogic() :m_bServInit(FALSE), m_port(0){
}

CServerLogic *CServerLogic::GetInstance() {
    static CServerLogic *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new CServerLogic();
    }

    return s_ptr;
}

bool CServerLogic::InitServ(unsigned short port) {
    if (m_bServInit)
    {
        return true;
    }
    m_bServInit = true;
    m_port = port;
    return m_msgServ.InitServ(port, this);
}

list<string> CServerLogic::ParsePackage(string &strPackage) const {
    list<string> result;
    while (true) {
        if (strPackage.size() < sizeof(PackageHeader)) {
            break;
        }

        PackageHeader header;
        memcpy(&header, strPackage.c_str(), sizeof(PackageHeader));
        if (header.m_verify != PACKAGE_VERIFY)
        {
            strPackage.clear();
            break;
        }

        if (strPackage.size() < header.m_size)
        {
            break;
        }
        string strContent = strPackage.substr(sizeof(PackageHeader), header.m_size - sizeof(PackageHeader));
        result.push_back(strContent);
        strPackage.erase(0, header.m_size);
    }
    return result;
}

void CServerLogic::OnServAccept(SOCKET client) {
    CScopedLocker lock(&m_servLock);
}

void CServerLogic::OnServRecvData(SOCKET client, const string &strRecved, string &strResp) {
    CScopedLocker lock(&m_servLock);
    m_servCache[client].append(strRecved);
    string &tmp = m_servCache[client];

    list<string> result = ParsePackage(tmp);
    for (list<string>::const_iterator it = result.begin() ; it != result.end() ; it++)
    {
        OnServRecvComplete(client, *it);
    }
}

void CServerLogic::OnServSocketErr(SOCKET client) {
    CScopedLocker lock(&m_servLock);
    DeleteClientBySocket(client);
}

void CServerLogic::OnServSocketClose(SOCKET socket) {
    CScopedLocker lock(&m_servLock);
    DeleteClientBySocket(socket);
}

/**
{
    "action":"action",
    "route":"12345",
    "content":{"content"}
}
*/
void CServerLogic::OnServRecvComplete(SOCKET client, const string &strData) {
    Value root;
    Reader().parse(strData, root);
    do 
    {
        if (root.type() != objectValue)
        {
            break;
        }

        string strAction = root["action"].asString();
        string strRoute = root["route"].asString();

        PackageHeader header;
        string str;
        if (strAction == "message")
        {
            /**
            需要路由的情况将路由表加入路由缓存
            */
            if (!strRoute.empty())
            {
                m_routeCache[strRoute] = client;
            }

            CScopedLocker lock(&m_servLock);
            string strChannel = root["channel"].asString();
            string package;
            for (list<ClientRegiserCache *>::const_iterator it = m_RegisterCache.begin() ; it != m_RegisterCache.end() ; it++)
            {
                ClientRegiserCache *ij = *it;
                if (ij->mChannels.end() != ij->mChannels.find(strChannel))
                {
                    package = GetMsgPackage(strData);
                    LOGGER_PRINT(L"send message");
                    ::send(ij->mClientSocket, package.c_str(), (int)package.size(), 0);
                }
            }
        } else if (strAction == "reply")
        {
            CScopedLocker lock(&m_servLock);
            map<string, SOCKET>::const_iterator ij = m_routeCache.find(strRoute);
            if (ij != m_routeCache.end())
            {
                header.m_size = sizeof(PackageHeader) + static_cast<unsigned int>(strData.size());
                str.clear();
                str.append((const char *)&header, sizeof(PackageHeader));
                str += strData;
                ::send(ij->second, str.c_str(), static_cast<int>(str.size()), 0);
                m_routeCache.erase(ij);
            }
        } else if (strAction == "register")
        {
            /**
            {
                "clientUnique":"aabbccdd",
                "channel":["1122", "3344", "aabb"]
            }
            */
            CScopedLocker lock(&m_servLock);
            LOGGER_PRINT(L"register %hs", strData.c_str());
            //string clientUnique = vContent["clientUnique"].asString();
            string clientUnique = root["clientUnique"].asString();
            //Value channels = vContent["channel"];
            Value channels = root["channel"];
            if (channels.type() != arrayValue)
            {
                break;
            }

            set<string> tmp;
            for (int i = 0 ; i < (int)channels.size() ; i++)
            {
                tmp.insert(channels[i].asString());
            }

            ClientRegiserCache *ptr = FindClientInCache(clientUnique);
            if (ptr == NULL)
            {
                ptr = new ClientRegiserCache();
                ptr->mClientUnique = clientUnique;
                ptr->mClientSocket = client;
                m_RegisterCache.push_back(ptr);
            }
            ptr->mChannels.insert(tmp.begin(), tmp.end());
            LOGGER_PRINT(L"register success");
        }
    } while (false);
}

ClientRegiserCache *CServerLogic::FindClientInCache(const string &clientUnique) const {
    for (list<ClientRegiserCache *>::const_iterator it = m_RegisterCache.begin() ; it != m_RegisterCache.end() ; it++)
    {
        ClientRegiserCache *ptr = *it;
        if (ptr->mClientUnique == clientUnique)
        {
            return ptr;
        }
    }
    return NULL;
}

bool CServerLogic::DeleteClientBySocket(SOCKET socket) {
    for (list<ClientRegiserCache *>::const_iterator it = m_RegisterCache.begin() ; it != m_RegisterCache.end() ; it++)
    {
        ClientRegiserCache *ptr = *it;
        if (ptr->mClientSocket == socket)
        {
            delete ptr;
            m_RegisterCache.erase(it);
            return true;
        }
    }
    return false;
}