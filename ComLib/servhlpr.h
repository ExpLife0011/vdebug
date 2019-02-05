#ifndef SERVHLPR_H_H_
#define SERVHLPR_H_H_
#include <Windows.h>

//��ʼ������
BOOL WINAPI InstallLocalServiceW(LPCWSTR image, LPCWSTR cmd, LPCWSTR wszsName, LPCWSTR wszDisplayName, LPCWSTR wszDescripion);
BOOL WINAPI InstallLocalServiceA(LPCSTR image, LPCSTR cmd, LPCSTR name, LPCSTR displayName, LPCSTR descripion);

//����ָ���ķ���
BOOL WINAPI ServStartW(LPCWSTR servName);
BOOL WINAPI ServStartA(LPCSTR servName);

//ָֹͣ���ķ���
BOOL WINAPI ServStopW(LPCWSTR servName);
BOOL WINAPI ServStopA(LPCSTR servName);

//��������
BOOL WINAPI StartLocalServiceW(LPCWSTR wszSrvName);
BOOL WINAPI StartLocalServiceA(LPCSTR wszSrvName);

//ֹͣ����
BOOL WINAPI StopLocalServiceW(LPCWSTR wszServName);
BOOL WINAPI StopLocalServiceA(LPCSTR servName);

//�Ƴ�����
BOOL WINAPI RemoveLocalServiceW(LPCWSTR wszServName);
BOOL WINAPI RemoveLocalServiceA(LPCSTR servName);

BOOL WINAPI ReportLocalServStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode);
#endif //SERVHLPR_H_H_