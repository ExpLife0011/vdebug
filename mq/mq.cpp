#include <WinSock2.h>
#include "mq.h"
#include "ServerLogic.h"
#include "ClientLogic.h"

static BOOL gs_bServInit = FALSE;
static BOOL gs_bClientInit = FALSE;

/**
初始化消息分发服务，一台终端只需要初始化一次
*/
BOOL WINAPI MsgInitServ(unsigned short port) {
    if (gs_bServInit)
    {
        return TRUE;
    }
    return (gs_bServInit = CServerLogic::GetInstance()->InitServ(port));
}

BOOL WINAPI MsgInitClient(unsigned short port) {
    if (gs_bClientInit) {
        return TRUE;
    }
    return (gs_bClientInit = CClientLogic::GetInstance()->InitClient(port));
}

/**
注册关心的频道，该频道有消息会调用传入的回调函数
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCSTR szChannel, PMsgNotify pfnProc, void *pParam) {
    return CClientLogic::GetInstance()->Register(szChannel, pfnProc, pParam);
}

/**
反注册频道，不再向该端口分发消息
*/
BOOL WINAPI MsgUnRegister(HANDLE_REGISTER index) {
    return CClientLogic::GetInstance()->UnRegister(index);
}

/**
根据频道发送消息，消息服务器会将该消息分发给订阅该频道的消息接收者
wszChannel：频道名称
wszContent：分发的内容
*/
BOOL WINAPI MsgSend(LPCSTR szChannel, LPCSTR szContent) {
    return CClientLogic::GetInstance()->Dispatch(szChannel, szContent);
}

/**
字符串分配接口
*/
LPSTR WINAPI MsgStrAlloc(int iSize) {
    return new CHAR[iSize];
}

/**
字符串复制接口
*/
LPCSTR WINAPI MsgStrCopy(LPCSTR sz) {
    if (!sz)
    {
        return NULL;
    }

    LPSTR ptr = new CHAR[lstrlenA(sz) + 1];
    lstrcpyA(ptr, sz);
    return ptr;
}

/**
字符串释放接口
*/
VOID WINAPI MsgStrFree(LPCSTR buffer) {
    delete []buffer;
}

/**
根据频道发送消息并接收回执
wszChannel：频道名称
wszContent：分发的内容
返回：接收到的消息订阅者的回执
*/
LPCSTR WINAPI MsgSendForResult(LPCSTR szChannel, LPCSTR szContent) {
    string result;
    CClientLogic::GetInstance()->DispatchForResult(szChannel, szContent, result, 3000);
    return MsgStrCopy(result.c_str());
}