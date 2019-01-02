#ifndef PROTOCOL_DPMSG_H_H_
#define PROTOCOL_DPMSG_H_H_
#include <string>

/**
Э��ͷ 4�ֽ�
int32 m_verify      //4�ֽ�У��λ �̶�ֵ0xfafb
int32 m_size        //4�ֽ����ݰ���С
//�������������ܴ�С���ֽ�Ϊ��λ
Э���� Json��ʽ��utf8����
{
    "action":"message",
    "route":"asdfeebb",
    "channel":"channel",
    "content":"aaddeeff"
}
{
    "action":"reply",
    "route":"asdfeebb",
    "content":"aaddeeff"
}
*/
#define PACKAGE_VERIFY  0xfafb

struct PackageHeader {
    unsigned int m_verify;
    unsigned int m_size;

    PackageHeader() {
        m_verify = PACKAGE_VERIFY;
        m_size = 0;
    }
};

static std::string GetMsgPackage(const std::string &data) {
    std::string result;
    PackageHeader header;
    header.m_size = (unsigned int)data.size() + sizeof(header);

    result.append((const char *)&header, sizeof(header));
    result.append(data);
    return result;
}
#endif