#ifndef SERVHLPR_H_H_
#define SERVHLPR_H_H_
#include <Windows.h>

//初始化服务
BOOL WINAPI InstallLocalServiceW(LPCWSTR image, LPCWSTR cmd, LPCWSTR wszsName, LPCWSTR wszDisplayName, LPCWSTR wszDescripion);
BOOL WINAPI InstallLocalServiceA(LPCSTR image, LPCSTR cmd, LPCSTR name, LPCSTR displayName, LPCSTR descripion);

//启动指定的服务
BOOL WINAPI ServStartW(LPCWSTR servName);
BOOL WINAPI ServStartA(LPCSTR servName);

//停止指定的服务
BOOL WINAPI ServStopW(LPCWSTR servName);
BOOL WINAPI ServStopA(LPCSTR servName);

//启动服务
BOOL WINAPI StartLocalServiceW(LPCWSTR wszSrvName);
BOOL WINAPI StartLocalServiceA(LPCSTR wszSrvName);

//停止服务
BOOL WINAPI StopLocalServiceW(LPCWSTR wszServName);
BOOL WINAPI StopLocalServiceA(LPCSTR servName);

//移除服务
BOOL WINAPI RemoveLocalServiceW(LPCWSTR wszServName);
BOOL WINAPI RemoveLocalServiceA(LPCSTR servName);

BOOL WINAPI ReportLocalServStatus(SERVICE_STATUS_HANDLE hStatus, DWORD dwCurrentStat, DWORD dwWin32ExitCode);
#endif //SERVHLPR_H_H_