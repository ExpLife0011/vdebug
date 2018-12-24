#ifndef SERVHLPR_H_H_
#define SERVHLPR_H_H_
#include <Windows.h>

//停止指定的服务
BOOL WINAPI ServStopW(LPCWSTR servName);

//启动指定的服务
BOOL WINAPI ServStartW(LPCWSTR servName);

//初始化服务
BOOL WINAPI InstallLocalService(LPCWSTR cmd, LPCWSTR wszsName, LPCWSTR wszDisplayName, LPCWSTR wszDescripion);

//启动服务
BOOL WINAPI StartLocalService(LPCWSTR wszSrvName);

//停止服务
BOOL WINAPI StopLocalService(LPCWSTR wszServName);

//移除服务
BOOL WINAPI RemoveLocalService(LPCWSTR wszServName);

BOOL WINAPI ReportLocalServStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode);
#endif //SERVHLPR_H_H_