#ifndef _H_GDMD5
#define _H_GDMD5

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// 计算一块缓冲区的MD5值(32个字符)
// 参数1: 缓冲区
// 参数2: 缓冲区大小
// 参数3: 出参，存储MD5值
// 参数4: 出参缓冲区大小，不得小于33
// 返回1表示成功，0表示失败
int md5_buffer(const char* buffer, int bufSize, char* outBuf, int outSize);

// 计算一个文件的MD5值(32个字符)，不支持大于4G的文件
// 参数1: 文件名
// 参数2: MD5出参
// 参数3: 出参缓冲区大小，不得小于33
// 返回1表示成功，0表示失败
int md5_file(const char* fileName, char* outBuf, int outSize);
#ifdef _MSC_VER
int md5_file16(const wchar_t* fileName, char* outBuf, int outSize);
#  if UNICODE || _UNICODE
#    define md5_fileT    md5_file16
#  else
#    define md5_fileT    md5_file
#  endif // #  if UNICODE || _UNICODE
#endif // #ifdef _MSC_VER

#ifdef __cplusplus
}
#endif

#endif
