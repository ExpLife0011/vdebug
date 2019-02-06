/**
模块间实时消息通讯组件，主要通过网络连接实现进程间消息交换 20181024 lougd
数据交换主要是通过"订阅" "发布"的方式进行。
接口说明：
MsgInitServ：初始化消息服务，同一台pc终端初始化一次即可。
MsgInitClient：初始化消息接收者。
MsgRegister：消息订阅接口，通过第一个参数指定自己感兴趣的"频道"，第二个参数数收到消息后会被调用的回调函数。
MsgSend：消息发布接口，第一个参数是消息要发布到的频道，第二个参数是要发布消息的内容。
*/
#ifndef DPMSG_INTERFACE_H_H_
#define DPMSG_INTERFACE_H_H_
#include <Windows.h>

//注册的相关功能频道
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
wszChannel:注册时关注的频道名
wszContent:具体传递的数据内容
返回：需要的回执，必须通过MsgStrAlloc来分配内存，不需要手动释放，由框架自动释放
*/
typedef LPCSTR (WINAPI *PMsgNotify)(LPCSTR wszChannel, LPCSTR wszContent, void *pParam);

#define INVALID_REGISTER_HANDLE     0xffffffff      //无效的索引句柄
typedef long HANDLE_REGISTER;                       //索引句柄，用于删除注册

/**
初始化消息分发服务，一台终端只需要一个消息服务
*/
BOOL WINAPI MsgInitServ(unsigned short servPort);

/**
初始化消息接收端，同一进程只需初始化一次即可
*/
BOOL WINAPI MsgInitClient(unsigned short servPort);

/**
字符串分配接口
*/
LPSTR WINAPI MsgStrAlloc(int iSize);

/**
字符串释放接口
*/
VOID WINAPI MsgStrFree(LPCSTR buffer);

/**
字符串复制接口
*/
LPCSTR WINAPI MsgStrCopy(LPCSTR wsz);

/**
注册关心的频道，该频道有消息会调用传入的回调函数
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCSTR szChannel, PMsgNotify pfnProc, void *pParam);

/**
反注册频道，不再向该端口分发消息
*/
BOOL WINAPI MsgUnRegister(HANDLE_REGISTER hRegister);

/**
根据频道发送消息，消息服务器会将该消息分发给订阅该频道的消息接收者
wszChannel：频道名称
wszContent：分发的内容
*/
BOOL WINAPI MsgSend(LPCSTR szChannel, LPCSTR szContent);

/**
根据频道发送消息并接收回执,返回值需要使用MsgStrFree接口进行释放。
*/
LPCSTR WINAPI MsgSendForResult(LPCSTR szChannel, LPCSTR szContent);
#ifdef __cplusplus
}
#endif  //__cplusplus
#endif  //DPMSG_INTERFACE_H_H_