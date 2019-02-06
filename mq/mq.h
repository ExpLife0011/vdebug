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
#define CHANNEL_PROC_SERVER      "channel_proc_server"
#define CHANNEL_DUMP_SERVER      "channel_dump_server"
#define CHANNEL_RPOC32           "channel_proc32"
#define CHANNEL_RPOC64           "channel_proc64"
#define CHANNEL_DUMP32           "channel_dump32"
#define CHANNEL_DUMP64           "channel_dump64"
#define CHANNEL_TASK32           "channel_task32"
#define CHANNEL_TASK64           "channel_task64"

#ifndef MQ_EXPORTS
    #if _WIN64 || WIN64
        #ifdef _DEBUG
            #pragma comment(lib, "../Debug/mq64.lib")
        #else
            #pragma comment(lib, "../Release/mq64.lib")
        #endif //_DEBUG
    #else
        #ifdef _DEBUG
            #pragma comment(lib, "../Debug/mq32.lib")
        #else
            #pragma comment(lib, "../Release/mq32.lib")
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
typedef LPCSTR (WINAPI *PMsgNotify)(LPCSTR wszChannel, LPCSTR wszContent, void *pParam);

#define INVALID_REGISTER_HANDLE     0xffffffff      //��Ч���������
typedef long HANDLE_REGISTER;                       //�������������ɾ��ע��

/**
��ʼ����Ϣ�ַ�����һ̨�ն�ֻ��Ҫһ����Ϣ����
*/
BOOL WINAPI MsgInitServ(unsigned short servPort);

/**
��ʼ����Ϣ���նˣ�ͬһ����ֻ���ʼ��һ�μ���
*/
BOOL WINAPI MsgInitClient(unsigned short servPort);

/**
�ַ�������ӿ�
*/
LPSTR WINAPI MsgStrAlloc(int iSize);

/**
�ַ����ͷŽӿ�
*/
VOID WINAPI MsgStrFree(LPCSTR buffer);

/**
�ַ������ƽӿ�
*/
LPCSTR WINAPI MsgStrCopy(LPCSTR wsz);

/**
ע����ĵ�Ƶ������Ƶ������Ϣ����ô���Ļص�����
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCSTR szChannel, PMsgNotify pfnProc, void *pParam);

/**
��ע��Ƶ����������ö˿ڷַ���Ϣ
*/
BOOL WINAPI MsgUnRegister(HANDLE_REGISTER hRegister);

/**
����Ƶ��������Ϣ����Ϣ�������Ὣ����Ϣ�ַ������ĸ�Ƶ������Ϣ������
wszChannel��Ƶ������
wszContent���ַ�������
*/
BOOL WINAPI MsgSend(LPCSTR szChannel, LPCSTR szContent);

/**
����Ƶ��������Ϣ�����ջ�ִ,����ֵ��Ҫʹ��MsgStrFree�ӿڽ����ͷš�
*/
LPCSTR WINAPI MsgSendForResult(LPCSTR szChannel, LPCSTR szContent);
#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  //DPMSG_INTERFACE_H_H_