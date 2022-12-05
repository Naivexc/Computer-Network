#include "NetConnectionInfo.h"
NetConnectionInfo::NetConnectionInfo(const char *__ip__, const int32_t &__fd__) : ip(__ip__), fd(__fd__), addr_len(sizeof(sockaddr))
{
    memset(&addr, 0, sizeof(sockaddr));
};

int32_t NetConnectionInfo::GetFd() const
{
    return fd;
}

void NetConnectionInfo::SetAddr(sa_family_t __family__, in_port_t __port__, const char *__ip__)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = __family__;
    addr.sin_port = htons(__port__);
    inet_pton(__family__, __ip__, &addr.sin_addr);
}

sockaddr_in *NetConnectionInfo::GetAddrPtr()
{
    return &addr;
}
socklen_t *NetConnectionInfo::GetAddrLenPtr()
{
    return &addr_len;
}

void NetConnectionInfo::PrintInfo() const
{
    WINDOW_DEBUG(fmt::print("ip: {}\n", addr.sin_addr.s_addr);)
    if (fd == -1)
        WINDOW_DEBUG(fmt::print("fd: unknown\n");)
    else
        WINDOW_DEBUG(fmt::print("fd: {}\n", fd);)
    WINDOW_DEBUG(fmt::print("port: {}\n", ntohs(addr.sin_port));)
    WINDOW_DEBUG(fmt::print("family: {}\n", addr.sin_family == AF_INET ? "AF_INET" : "???");)
}