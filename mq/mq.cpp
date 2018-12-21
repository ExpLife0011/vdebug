#include <WinSock2.h>
#include "mq.h"
#include "ServerLogic.h"
#include "ClientLogic.h"

static BOOL gs_bServInit = FALSE;
static BOOL gs_bClientInit = FALSE;

/**
��ʼ����Ϣ�ַ�����һ̨�ն�ֻ��Ҫ��ʼ��һ��
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
ע����ĵ�Ƶ������Ƶ������Ϣ����ô���Ļص�����
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCWSTR wszChannel, PMsgNotify pfnProc, void *pParam) {
    return CClientLogic::GetInstance()->Register(wszChannel, pfnProc, pParam);
}

/**
��ע��Ƶ����������ö˿ڷַ���Ϣ
*/
BOOL WINAPI MsgUnRegister(HANDLE_REGISTER index) {
    return CClientLogic::GetInstance()->UnRegister(index);
}

/**
����Ƶ��������Ϣ����Ϣ�������Ὣ����Ϣ�ַ������ĸ�Ƶ������Ϣ������
wszChannel��Ƶ������
wszContent���ַ�������
*/
BOOL WINAPI MsgSend(LPCWSTR wszChannel, LPCWSTR wszContent) {
    return CClientLogic::GetInstance()->Dispatch(wszChannel, wszContent);
}

/**
�ַ�������ӿ�
*/
LPWSTR WINAPI MsgStrAlloc(int iSize) {
    return new WCHAR[iSize];
}

/**
�ַ������ƽӿ�
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
�ַ����ͷŽӿ�
*/
VOID WINAPI MsgStrFree(LPCWSTR buffer) {
    delete []buffer;
}

/**
����Ƶ��������Ϣ�����ջ�ִ
wszChannel��Ƶ������
wszContent���ַ�������
���أ����յ�����Ϣ�����ߵĻ�ִ
*/
LPCWSTR WINAPI MsgSendForResult(LPCTSTR wszChannel, LPCWSTR wszContent) {
    wstring result;
    CClientLogic::GetInstance()->DispatchForResult(wszChannel, wszContent, result, 3000);
    return MsgStrCopy(result.c_str());
}