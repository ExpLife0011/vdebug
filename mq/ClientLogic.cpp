#include "ClientLogic.h"
#include "protocol.h"
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>

CClientLogic *CClientLogic::GetInstance() {
    static CClientLogic *s_ptr = NULL;

    if (s_ptr == NULL)
    {
        s_ptr = new CClientLogic();
    }
    return s_ptr;
}

CClientLogic::CClientLogic() :
m_tpool(NULL),
m_curIndex(0xff11),
m_bClientInit(FALSE),
m_bConnectSucc(FALSE),
m_port(0){
}

bool CClientLogic::InitClient(unsigned short port) {
    if (m_bClientInit)
    {
        return true;
    }
    m_bClientInit = true;
    m_tpool = GetThreadPool(1, 4);

    static int s_magic = 0xff23;
    srand(GetTickCount() + s_magic++);
    m_clientUnique = FormatA(
        "%d_%04x%04x",
        GetCurrentProcessId(),
        rand() % 0xffff,
        rand() % 0xffff
        );
    m_port = port;
    m_bConnectSucc =  m_msgClient.InitClient("127.0.0.1", port, this, 3000);
    if (!m_bConnectSucc)
    {
        CloseHandle(CreateThread(NULL, 0, ConnectThread, this, 0, NULL));
    }
    return m_bConnectSucc;
}

HANDLE_REGISTER CClientLogic::Register(const string &strKey, PMsgNotify pfn, void *pParam) {
    CScopedLocker lock(&m_clientLock);
    NotifyProcInfo notify;
    notify.pNotifyProc = pfn;
    notify.pParam = pParam;
    notify.index = m_curIndex++;
    m_notifyMap[strKey].push_back(notify);

    Value content;
    Value arry;
    content["action"] = "register";
    content["clientUnique"] = m_clientUnique;

    arry.append(strKey);
    content["channel"] = arry;

    string result = GetMsgPackage(FastWriter().write(content));
    m_msgClient.Send(result);
    return notify.index;
}

bool CClientLogic::UnRegister(HANDLE_REGISTER index) {
    CScopedLocker lock(&m_clientLock);
    map<string, list<NotifyProcInfo>>::iterator it;
    list<NotifyProcInfo>::const_iterator ij;

    for (it = m_notifyMap.begin() ; it != m_notifyMap.end() ; it++) {
        for (ij = it->second.begin() ; ij != it->second.end() ; ij++) {
            if (ij->index == index)
            {
                it->second.erase(ij);
                if (it->second.empty())
                {
                    m_notifyMap.erase(it);
                }
                return true;
            }
        }
    }
    return false;
}

bool CClientLogic::Dispatch(const string &strKey, const string &strValue) {
    DispatchInternal(strKey, strValue, "");
    return true;
}

bool CClientLogic::DispatchInternal(const string &strKey, const string &strValue, const string &strRoute) const {
    Value content;
    content["action"] = "message";
    content["channel"] = strKey;
    content["content"] = strValue;

    if (!strRoute.empty())
    {
        content["result"] = 1;
        content["route"] = strRoute;
    }

    string strContent = FastWriter().write(content);

    PackageHeader header;
    header.m_size = sizeof(PackageHeader) + static_cast<unsigned int>(strContent.size());

    string strData = GetMsgPackage(strContent);
    m_msgClient.Send(strData);
    return true;
}

bool CClientLogic::DispatchForResult(const string &strKey, const string &strValue, string &strResult, int iTimeOut) {
    MsgRecvCache cache;
    cache.m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);

    static unsigned int s_serial = 0;
    srand(GetTickCount() + s_serial++);
    cache.m_strRoute = FormatA("%04x%04x", rand() % 0xffff, rand() % 0xffff);

    {
        CScopedLocker lock(&m_clientLock);
        m_recvCache[cache.m_strRoute] = &cache;
    }
    DispatchInternal(strKey, strValue, cache.m_strRoute);
    WaitForSingleObject(cache.m_hNotify, iTimeOut);
    {
        CScopedLocker lock(&m_clientLock);
        map<string, MsgRecvCache *>::const_iterator it = m_recvCache.find(cache.m_strRoute);
        if (it != m_recvCache.end())
        {
            m_recvCache.erase(it);
        }
    }
    CloseHandle(cache.m_hNotify);
    strResult = cache.m_strResult;
    return true;
}

struct ClientTaskInfo {
    string strKey;
    string strValue;
    string strRoute;
    NotifyProcInfo info;
};

bool CClientLogic::DispatchInCache(const string &strKey, const string &strValue, const string &strRoute) const {
    CScopedLocker lock(&m_clientLock);
    map<string, list<NotifyProcInfo>>::const_iterator it;
    list<NotifyProcInfo>::const_iterator ij;
    it = m_notifyMap.find(strKey);
    if (it != m_notifyMap.end())
    {
        for (ij = it->second.begin() ; ij != it->second.end() ; ij++)
        {
            class TaskRunable : public ThreadRunable {
            public:
                TaskRunable (ClientTaskInfo *param) {
                    mParam = param;
                }

                virtual ~TaskRunable() {
                    delete mParam;
                }

            public:
                void run() {
                    LPCSTR sz = (mParam->info.pNotifyProc)(mParam->strKey.c_str(), mParam->strValue.c_str(), mParam->info.pParam);
                    string str;
                    if (sz)
                    {
                        str = sz;
                        MsgStrFree(sz);
                    }

                    if (!mParam->strRoute.empty())
                    {
                        //向对端发送回执
                        if (str.empty())
                        {
                            str = "state success";
                        }

                        Value root;
                        root["action"] = "reply";
                        root["route"] = mParam->strRoute;
                        root["content"] = str;

                        string strReply = GetMsgPackage(FastWriter().write(root));
                        CClientLogic::GetInstance()->m_msgClient.Send(strReply);
                    }
                }

            private:
                ClientTaskInfo *mParam;
            };

            ClientTaskInfo *ptr = new ClientTaskInfo();
            ptr->info = *ij;
            ptr->strKey = strKey;
            ptr->strValue = strValue;
            ptr->strRoute = strRoute;
            m_tpool->exec(new TaskRunable(ptr));
        }
    }
    return true;
}

DWORD CClientLogic::ConnectThread(LPVOID pParam) {
    while (true) {
        if (!GetInstance()->m_bConnectSucc)
        {
            LOGGER_PRINT(L"尝试连接服务端...");
            GetInstance()->m_bConnectSucc = GetInstance()->m_msgClient.InitClient("127.0.0.1", GetInstance()->m_port, GetInstance(), 500);
        } else {
            break;
        }

        Sleep(1000);
    }
    return 0;
}

list<string> CClientLogic::ParsePackage(string &strPackage) const {
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

void CClientLogic::OnClientRecvData(CMsgClient &client, const string &strRecved, string &strResp) {
    CScopedLocker lock(&m_clientLock);
    m_clientCache.append(strRecved);
    list<string> result = ParsePackage(m_clientCache);
    for (list<string>::const_iterator it = result.begin() ; it != result.end() ; it++)
    {
        OnClientRecvComplete(*it);
    }
}

void CClientLogic::OnClientSocketErr(CMsgClient &client) {
}

void CClientLogic::OnClientRecvComplete(const string &strData) {
    Value content;

    Reader().parse(strData, content);
    if (content.type() != objectValue)
    {
        return;
    }

    utf8_mstring strAction = GetStrFormJson(content, "action");
    utf8_mstring strRoute = GetStrFormJson(content, "route");
    utf8_mstring strContent = GetStrFormJson(content, "content");

    if (strAction == "message")
    {
        string strChannel = content["channel"].asString();
        //推送给所有频道
        DispatchInCache(strChannel, strContent.c_str(), strRoute.c_str());
    } else if (strAction == "reply")
    {
        if (strRoute.empty())
        {
            return;
        }

        CScopedLocker lock(&m_clientLock);
        string str = strRoute.c_str();
        map<string, MsgRecvCache *>::const_iterator it = m_recvCache.find(str);
        if (it != m_recvCache.end())
        {
            MsgRecvCache *ptr = it->second;
            ptr->m_strResult = strContent;
            if (ptr->m_hNotify != NULL)
            {
                SetEvent(ptr->m_hNotify);
            }
            m_recvCache.erase(it);
        }
    }
}

void CClientLogic::OnClientConnect(CMsgClient &client) {
    LOGGER_PRINT(L"OnClientConnect");
    CScopedLocker lock(&m_clientLock);
    Value root;
    Value arry;

    root["action"] = "register";
    root["clientUnique"] = m_clientUnique;

    for (map<string, list<NotifyProcInfo>>::const_iterator it = m_notifyMap.begin() ; it != m_notifyMap.end() ; it++)
    {
        arry.append(it->first);
    }
    root["channel"] = arry;

    string package = GetMsgPackage(FastWriter().write(root));
    client.Send(package);
}