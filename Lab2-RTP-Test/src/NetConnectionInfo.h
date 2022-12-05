#ifndef NET_CONNECTION_INFO_H
#define NET_CONNECTION_INFO_H
#include "rtp.h"
#include <cstdint>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <cstdio>
#ifdef __WINDOW_DEBUG__
#include <fmt/core.h>
#endif
class NetConnectionInfo
{
private:
    const char *ip;
    const int32_t fd;
    mutable sockaddr_in addr; //能不能不用mutable
    socklen_t addr_len;

public:
    NetConnectionInfo(const char *__ip__, const int32_t &__fd__);
    int32_t GetFd() const;
    void SetAddr(sa_family_t __family__, in_port_t __port__, const char *__ip__);
    sockaddr_in *GetAddrPtr();
    socklen_t *GetAddrLenPtr();
    void PrintInfo() const;
};

#endif