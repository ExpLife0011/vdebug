#include <WinSock2.h>
#include "mq.h"
#include "ServerLogic.h"
#include "ClientLogic.h"

static BOOL gs_bServInit = FALSE;
static BOOL gs_bClientInit = FALSE;

/**
初始化消息分发服务，一台终端只需要初始化一次
*/
BOOL WINAPI MsgInitServ() {
    if (gs_bServInit)
    {
        return TRUE;
    }
    return (gs_bServInit = CServerLogic::GetInstance()->InitServ());
}

BOOL WINAPI MsgInitClient() {
    if (gs_bClientInit) {
        return TRUE;
    }
    return (gs_bClientInit = CClientLogic::GetInstance()->InitClient());
}

/**
注册关心的频道，该频道有消息会调用传入的回调函数
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCWSTR wszChannel, PMsgNotify pfnProc, void *pParam) {
    return CClientLogic::GetInstance()->Register(wszChannel, pfnProc, pParam);
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
BOOL WINAPI MsgSend(LPCWSTR wszChannel, LPCWSTR wszContent) {
    return CClientLogic::GetInstance()->Dispatch(wszChannel, wszContent);
}

/**
字符串分配接口
*/
LPWSTR WINAPI MsgStrAlloc(int iSize) {
    return new WCHAR[iSize];
}

/**
字符串复制接口
*/
LPCWSTR WINAPI MsgStrCopy(LPCWSTR wsz) {
    if (!wsz)
    {
        return NULL;
    }

    LPWSTR ptr = new WCHAR[lstrlenW(wsz) + 1];
    lstrcpyW(ptr, wsz);
    return ptr;
}

/**
字符串释放接口
*/
VOID WINAPI MsgStrFree(LPCWSTR buffer) {
    delete []buffer;
}

/**
根据频道发送消息并接收回执
wszChannel：频道名称
wszContent：分发的内容
返回：接收到的消息订阅者的回执
*/
LPCWSTR WINAPI MsgSendForResult(LPCTSTR wszChannel, LPCWSTR wszContent) {
    wstring result;
    CClientLogic::GetInstance()->DispatchForResult(wszChannel, wszContent, result, 3000);
    return MsgStrCopy(result.c_str());
}