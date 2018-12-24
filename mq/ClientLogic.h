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
    wstring m_wstrRoute;      //��Ϣ·�ɱ�ʶ
    HANDLE m_hNotify;         //��Ϣ֪ͨ�¼�
    wstring m_wstrResult;     //��Ϣ�����
};

class CClientLogic : public ClientEvent {
private:
    CClientLogic();
public:
    static CClientLogic *GetInstance();
    bool InitClient(unsigned short port);

    HANDLE_REGISTER Register(const wstring &wstrKey, PMsgNotify pfn, void *pParam);
    bool UnRegister(HANDLE_REGISTER hRegister);
    bool Dispatch(const wstring &wstrKey, const wstring &wstrValue);
    bool DispatchForResult(const wstring &wstrKey, const wstring &wstrValue, wstring &wstrResult, int iTimeOut);

private:
    static DWORD WINAPI ConnectThread(LPVOID pParam);
    list<string> ParsePackage(string &strPackage) const;
    bool DispatchInternal(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const;
    bool DispatchInCache(const wstring &wstrKey, const wstring &wstrValue, const wstring &wstrRoute) const;

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
    map<wstring, list<NotifyProcInfo>> m_notifyMap;//Ƶ�����ݽ��ջ���
    map<wstring, MsgRecvCache *> m_recvCache;   //���ݻ�ִ����
    HANDLE_REGISTER m_curIndex;                 //��ǰ������������
    ThreadPoolBase *m_tpool;                    //�������̳߳�
    string m_clientUnique;                      //�ͻ��˱�ʶ
};
#endif //CLIENTLOGIC_DPMSG_H_H_