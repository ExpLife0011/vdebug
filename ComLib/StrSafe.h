#ifndef THREAD_SAFE_STR_H_H_
#define THREAD_SAFE_STR_H_H_

const char *__stdcall SafeStrCopyA(const char *);
const wchar_t *__stdcall SafeStrCopyW(const wchar_t *);
#endif