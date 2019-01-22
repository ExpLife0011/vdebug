#include <WinSock2.h>
#include "mq.h"
#include "ServerLogic.h"
#include "ClientLogic.h"

static BOOL gs_bServInit = FALSE;
static BOOL gs_bClientInit = FALSE;

/**
��ʼ����Ϣ�ַ�����һ̨�ն�ֻ��Ҫ��ʼ��һ��
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
ע����ĵ�Ƶ������Ƶ������Ϣ����ô���Ļص�����
*/
HANDLE_REGISTER WINAPI MsgRegister(LPCSTR szChannel, PMsgNotify pfnProc, void *pParam) {
    return CClientLogic::GetInstance()->Register(szChannel, pfnProc, pParam);
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
BOOL WINAPI MsgSend(LPCSTR szChannel, LPCSTR szContent) {
    return CClientLogic::GetInstance()->Dispatch(szChannel, szContent);
}

/**
�ַ�������ӿ�
*/
LPSTR WINAPI MsgStrAlloc(int iSize) {
    return new CHAR[iSize];
}

/**
�ַ������ƽӿ�
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
�ַ����ͷŽӿ�
*/
VOID WINAPI MsgStrFree(LPCSTR buffer) {
    delete []buffer;
}

/**
����Ƶ��������Ϣ�����ջ�ִ
wszChannel��Ƶ������
wszContent���ַ�������
���أ����յ�����Ϣ�����ߵĻ�ִ
*/
LPCSTR WINAPI MsgSendForResult(LPCSTR szChannel, LPCSTR szContent) {
    string result;
    CClientLogic::GetInstance()->DispatchForResult(szChannel, szContent, result, 3000);
    return MsgStrCopy(result.c_str());
}