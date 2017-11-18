#ifndef _H_GDMD5
#define _H_GDMD5

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ����һ�黺������MD5ֵ(32���ַ�)
// ����1: ������
// ����2: ��������С
// ����3: ���Σ��洢MD5ֵ
// ����4: ���λ�������С������С��33
// ����1��ʾ�ɹ���0��ʾʧ��
int md5_buffer(const char* buffer, int bufSize, char* outBuf, int outSize);

// ����һ���ļ���MD5ֵ(32���ַ�)����֧�ִ���4G���ļ�
// ����1: �ļ���
// ����2: MD5����
// ����3: ���λ�������С������С��33
// ����1��ʾ�ɹ���0��ʾʧ��
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
