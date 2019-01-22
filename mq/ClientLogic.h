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
    void *pParam;               //��Ҫ����ص��Ĳ���
    PMsgNotify pNotifyProc;     //�ص�������ַ
    HANDLE_REGISTER index;      //ע�����������ڷ�ע��
};

struct MsgRecvCache {
    string m_strRoute;        //��Ϣ·�ɱ�ʶ
    HANDLE m_hNotify;         //��Ϣ֪ͨ�¼�
    string m_strResult;       //��Ϣ�����
};

class CClientLogic : public ClientEvent {
private:
    CClientLogic();
public:
    static CClientLogic *GetInstance();
    bool InitClient(unsigned short port);

    HANDLE_REGISTER Register(const string &wstrKey, PMsgNotify pfn, void *pParam);
    bool UnRegister(HANDLE_REGISTER hRegister);
    bool Dispatch(const string &wstrKey, const string &wstrValue);
    bool DispatchForResult(const string &wstrKey, const string &wstrValue, string &wstrResult, int iTimeOut);

private:
    static DWORD WINAPI ConnectThread(LPVOID pParam);
    list<string> ParsePackage(string &strPackage) const;
    bool DispatchInternal(const string &wstrKey, const string &wstrValue, const string &wstrRoute) const;
    bool DispatchInCache(const string &wstrKey, const string &wstrValue, const string &wstrRoute) const;

private:
    /**
    �ͻ����¼��ӿ�
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
    client�����������
    */
    CMsgClient m_msgClient;
    CCriticalSectionLockable m_clientLock;      //��Ϣ���ն˶˶�д��
    string m_clientCache;                       //��Ϣ���ն˷������
    map<string, list<NotifyProcInfo>> m_notifyMap;//Ƶ�����ݽ��ջ���
    map<string, MsgRecvCache *> m_recvCache;   //���ݻ�ִ����
    HANDLE_REGISTER m_curIndex;                 //��ǰ������������
    ThreadPoolBase *m_tpool;                    //�������̳߳�
    string m_clientUnique;                      //�ͻ��˱�ʶ
};
#endif //CLIENTLOGIC_DPMSG_H_H_