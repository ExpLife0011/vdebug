#ifndef PROTOCOL_DPMSG_H_H_
#define PROTOCOL_DPMSG_H_H_
#include <string>

/**
协议头 4字节
int16 m_verify      //2字节校验位 固定值0xfafb
int16 m_size        //2字节数据包大小
//本条交互数据总大小，字节为单位
协议体 Json格式，utf8编码
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