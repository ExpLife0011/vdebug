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
    m_tpool = _GetThreadPool(1, 4);

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

HANDLE_REGISTER CClientLogic::Register(const wstring &wstrKey, PMsgNotify pfn, void *pParam) {
    CScopedLocker lock(&m_clientLock);
    NotifyProcInfo notify;
    notify.pNotifyProc = pfn;
    notify.pParam = pParam;
    notify.index = m_curIndex++;
    m_notifyMap[wstrKey].push_back(notify);

    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", "register");
    cJSON_AddStringToObject(content, "clientUnique", m_clientUnique.c_str());
    cJSON_AddStringToObject(content, "channel", WtoU(wstrKey).c_str());

    /*
    Value content;
    content["action"] = "register";
    content["clientUnique"] = m_clientUnique;
    content["channel"].append(WtoU(wstrKey));
    */
    string result = GetMsgPackage(cJSON_Print(content));
    cJSON_Delete(content);
    m_msgClient.Send(result);
    return notify.index;
}

bool CClientLogic::UnRegister(HANDLE_REGISTER index) {
    CScopedLocker lock(&m_clientLock);
    map<wstring, list<NotifyProcInfo>>::iterator it;
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

bool CClientLogic::Dispatch(const wstring &wstrKey, const wstring &wstrValue) {
    DispatchInternal(wstrKey, wstrValue, L"");
    return true;
}

bool CClientLogic::DispatchInternal(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const {
    /**
    Value vContent;
    vContent["action"] = "message";
    vContent["channel"] = WtoU(wstrKey);
    vContent["content"] = WtoU(wstrValue);

    if (!wstrRoute.empty())
    {
        vContent["result"] = 1;
        vContent["route"] = WtoU(wstrRoute);
    }
    string strContent = FastWriter().write(vContent);
    */
    cJSON *content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", "message");
    cJSON_AddStringToObject(content, "channel", WtoU(wstrKey).c_str());
    cJSON_AddStringToObject(content, "content", WtoU(wstrValue).c_str());

    if (!wstrRoute.empty())
    {
        cJSON_AddNumberToObject(content, "result", 1);
        cJSON_AddStringToObject(content, "route", WtoU(wstrRoute).c_str());
    }

    string strContent = cJSON_Print(content);
    cJSON_Delete(content);

    PackageHeader header;
    header.m_size = sizeof(PackageHeader) + static_cast<unsigned short>(strContent.size());

    string strData = GetMsgPackage(strContent);
    m_msgClient.Send(strData);
    return true;
}

bool CClientLogic::DispatchForResult(const wstring &wstrKey, const wstring &wstrValue, wstring &wstrResult, int iTimeOut) {
    MsgRecvCache cache;
    cache.m_hNotify = CreateEventW(NULL, FALSE, FALSE, NULL);

    static unsigned int s_serial = 0;
    srand(GetTickCount() + s_serial++);
    cache.m_wstrRoute = FormatW(L"%04x%04x", rand() % 0xffff, rand() % 0xffff);

    {
        CScopedLocker lock(&m_clientLock);
        m_recvCache[cache.m_wstrRoute] = &cache;
    }
    DispatchInternal(wstrKey, wstrValue, cache.m_wstrRoute);
    WaitForSingleObject(cache.m_hNotify, iTimeOut);
    {
        CScopedLocker lock(&m_clientLock);
        map<wstring, MsgRecvCache *>::const_iterator it = m_recvCache.find(cache.m_wstrRoute);
        if (it != m_recvCache.end())
        {
            m_recvCache.erase(it);
        }
    }
    CloseHandle(cache.m_hNotify);
    wstrResult = cache.m_wstrResult;
    return true;
}

struct ClientTaskInfo {
    wstring wstrKey;
    wstring wstrValue;
    wstring wstrRoute;
    NotifyProcInfo info;
};

bool CClientLogic::DispatchInCache(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const {
    CScopedLocker lock(&m_clientLock);
    map<wstring, list<NotifyProcInfo>>::const_iterator it;
    list<NotifyProcInfo>::const_iterator ij;
    it = m_notifyMap.find(wstrKey);
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
                    LPCWSTR wsz = (mParam->info.pNotifyProc)(mParam->wstrKey.c_str(), mParam->wstrValue.c_str(), mParam->info.pParam);
                    wstring wstr;
                    if (wsz)
                    {
                        wstr = wsz;
                        MsgStrFree(wsz);
                    }

                    if (!mParam->wstrRoute.empty())
                    {
                        //向对端发送回执
                        if (wstr.empty())
                        {
                            wstr = L"state success";
                        }

                        PackageHeader header;
                        cJSON *json = cJSON_CreateObject();
                        cJSON_AddStringToObject(json, "action", "reply");
                        cJSON_AddStringToObject(json, "route", WtoU(mParam->wstrRoute).c_str());
                        cJSON_AddStringToObject(json, "content", WtoU(wstr).c_str());

                        string content = cJSON_Print(json);
                        cJSON_Delete(json);

                        string strReply = GetMsgPackage(content);
                        CClientLogic::GetInstance()->m_msgClient.Send(strReply);
                    }
                }

            private:
                ClientTaskInfo *mParam;
            };

            ClientTaskInfo *ptr = new ClientTaskInfo();
            ptr->info = *ij;
            ptr->wstrKey = wstrKey;
            ptr->wstrValue = wstrValue;
            ptr->wstrRoute = wstrRoute;
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
            GetInstance()->m_bConnectSucc = GetInstance()->m_msgClient.InitClient("127.0.0.1", TCP_PORT_MQ, GetInstance(), 3000);
        } else {
            break;
        }

        Sleep(1000 * 5);
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
    cJSON *content = cJSON_Parse(strData.c_str());

    if (cJSON_Object != content->type)
    {
        return;
    }

    string strAction = cJSON_GetObjectItem(content, "action")->valuestring;
    string strRoute = cJSON_GetObjectItem(content, "route")->valuestring;
    string strContent = cJSON_GetObjectItem(content, "content")->valuestring;

    if (strAction == "message")
    {
        string strChannel = cJSON_GetObjectItem(content, "channel")->valuestring;
        cJSON_Delete(content);
        //推送给所有频道
        DispatchInCache(UtoW(strChannel.c_str()), UtoW(strContent.c_str()), UtoW(strRoute.c_str()));
    } else if (strAction == "reply")
    {
        cJSON_Delete(content);
        if (strRoute.empty())
        {
            return;
        }
        CScopedLocker lock(&m_clientLock);
        wstring wstr = UtoW(strRoute.c_str());
        map<wstring, MsgRecvCache *>::const_iterator it = m_recvCache.find(wstr);
        if (it != m_recvCache.end())
        {
            MsgRecvCache *ptr = it->second;
            ptr->m_wstrResult = UtoW(strContent.c_str());
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
    cJSON *root = cJSON_CreateObject();
    cJSON *arry = cJSON_CreateArray();

    cJSON_AddStringToObject(root, "action", "register");
    cJSON_AddStringToObject(root, "clientUnique", m_clientUnique.c_str());

    for (map<wstring, list<NotifyProcInfo>>::const_iterator it = m_notifyMap.begin() ; it != m_notifyMap.end() ; it++)
    {
        cJSON_AddItemToArray(arry, cJSON_CreateString(WtoU(it->first).c_str()));
    }
    cJSON_AddItemToObject(root, "channel", arry);

    string package = GetMsgPackage(cJSON_Print(root));
    cJSON_Delete(root);
    client.Send(package);
}