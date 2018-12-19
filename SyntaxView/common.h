#ifndef COMMON_SYNTAX_H_H_
#define COMMON_SYNTAX_H_H_
#include <Windows.h>

//��ӡ������Ϣ
VOID PrintDbgInternal(LPCWSTR wszTarget, LPCSTR szFile, DWORD dwLine, LPCWSTR wszFormat, ...);
#define dp(f, ...) PrintDbgInternal(L"SyntaxView", __FILE__, __LINE__, f, ##__VA_ARGS__)
#endif