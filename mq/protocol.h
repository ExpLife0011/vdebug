#ifndef PROTOCOL_DPMSG_H_H_
#define PROTOCOL_DPMSG_H_H_
#include <string>

/**
Э��ͷ 4�ֽ�
int16 m_verify      //2�ֽ�У��λ �̶�ֵ0xfafb
int16 m_size        //2�ֽ����ݰ���С
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
    unsigned short m_verify;
    unsigned short m_size;

    PackageHeader() {
        m_verify = PACKAGE_VERIFY;
        m_size = 0;
    }
};

static std::string GetMsgPackage(const std::string &data) {
    std::string result;
    PackageHeader header;
    header.m_size = (unsigned short)data.size() + sizeof(header);

    result.append((const char *)&header, sizeof(header));
    result.append(data);
    return result;
}
#endif