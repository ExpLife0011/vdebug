/**
ģ���ʵʱ��ϢͨѶ�������Ҫͨ����������ʵ�ֽ��̼���Ϣ���� 20181024 lougd
���ݽ�����Ҫ��ͨ��"����" "����"�ķ�ʽ���С�
�ӿ�˵����
MsgInitServ����ʼ����Ϣ����ͬһ̨pc�ն˳�ʼ��һ�μ��ɡ�
MsgInitClient����ʼ����Ϣ�����ߡ�
MsgRegister����Ϣ���Ľӿڣ�ͨ����һ������ָ���Լ�����Ȥ��"Ƶ��"���ڶ����������յ���Ϣ��ᱻ���õĻص�������
MsgSend����Ϣ�����ӿڣ���һ����������ϢҪ��������Ƶ�����ڶ���������Ҫ������Ϣ�����ݡ�
*/
#ifndef DPMSG_INTERFACE_H_H_
#define DPMSG_INTERFACE_H_H_
#include <Windows.h>

//ע�����ع���Ƶ��
#define MQ_CHANNEL_DBG_SERVER      L"mq_event_server"
#define MQ_CHANNEL_DBG_CLIENT32    L"mq_event_client_proc32"
#define MQ_CHANNEL_DBG_CLIENT64    L"mq_event_client_proc64"
#define MQ_CHANNEL_DBG_DUMP32      L"mq_event_client_dump32"
#define MQ_CHANNEL_DBG_DUMP64      L"mq_event_client_dump64"

#ifndef MQ_EXPORTS
    #if _WIN64 || WIN64
        #ifdef _DEBUG
            #pragma comment(lib, "../Debug/x64/mq64.lib")
        #else
            #pragma comment(lib, "../Release/x64/mq64.lib")
        #endif //_DEBUG
    #else
        #ifdef _DEBUG
            #pragma comment(lib, "../Debug/x32/mq32.lib")
        #else
            #pragma comment(lib, "../Release/x32/mq32.lib")
        #endif //_DEBUG
    #endif //_WIN64
#endif //MQ_EXPORTS

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

/**
wszChannel:ע��ʱ��ע��Ƶ����
wszContent:���崫�ݵ���������
���أ���Ҫ�Ļ�ִ������ͨ��MsgStrAlloc�������ڴ棬����Ҫ�ֶ��ͷţ��ɿ���Զ��ͷ�
*/
typedef LPCWSTR (WINAPI *PMsgNotify)(LPCWSTR wszChannel, LPCWSTR wszContent, void *pParam);

#define INVALID_REGISTER_HANDLE     0xffffffff      //��Ч���������
typedef long HANDLE_REGISTER;                       //�������������ɾ��ע��

/**
��ʼ����Ϣ�ַ�����һ̨�ն�ֻ��Ҫһ����Ϣ����
*/
BOOL WINAPI MsgInitServ();

/**
��ʼ����Ϣ���նˣ�ͬһ����ֻ���ʼ��һ�μ���
*/
BOOL WINAPI MsgInitClient();

/**
�ַ�������ӿ�
*/
LPWSTR WINAPI MsgStrAlloc(int iSize);

/**
�ַ����ͷŽӿ�
*/
VOID WINAPI MsgStrFree(LPCWSTR buffer);

/**
�ַ������ƽӿ�
*/
LPCWSTR WINAPI MsgStrCopy(LPCWSTR wsz);

/**
ע����ĵ�Ƶ������Ƶ������Ϣ����ô���Ļص�����
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCWSTR wszChannel, PMsgNotify pfnProc, void *pParam);

/**
��ע��Ƶ����������ö˿ڷַ���Ϣ
*/
BOOL WINAPI MsgUnRegister(HANDLE_REGISTER hRegister);

/**
����Ƶ��������Ϣ����Ϣ�������Ὣ����Ϣ�ַ������ĸ�Ƶ������Ϣ������
wszChannel��Ƶ������
wszContent���ַ�������
*/
BOOL WINAPI MsgSend(LPCWSTR wszChannel, LPCWSTR wszContent);

/**
����Ƶ��������Ϣ�����ջ�ִ,����ֵ��Ҫʹ��MsgStrFree�ӿڽ����ͷš�
*/
LPCWSTR WINAPI MsgSendForResult(LPCWSTR wszChannel, LPCWSTR wszContent);
#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  //DPMSG_INTERFACE_H_H_