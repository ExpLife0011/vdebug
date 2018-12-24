#ifndef SERVHLPR_H_H_
#define SERVHLPR_H_H_
#include <Windows.h>

//ָֹͣ���ķ���
BOOL WINAPI ServStopW(LPCWSTR servName);

//����ָ���ķ���
BOOL WINAPI ServStartW(LPCWSTR servName);

//��ʼ������
BOOL WINAPI InstallLocalService(LPCWSTR cmd, LPCWSTR wszsName, LPCWSTR wszDisplayName, LPCWSTR wszDescripion);

//��������
BOOL WINAPI StartLocalService(LPCWSTR wszSrvName);

//ֹͣ����
BOOL WINAPI StopLocalService(LPCWSTR wszServName);

//�Ƴ�����
BOOL WINAPI RemoveLocalService(LPCWSTR wszServName);

BOOL WINAPI ReportLocalServStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode);
#endif //SERVHLPR_H_H_