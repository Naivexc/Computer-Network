#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cassert>
#include "rtp.h"
#include <string.h>
#include <fstream>
#ifdef __WINDOW_DEBUG__
#include <fmt/core.h>
#endif
#include <errno.h>

RTP_header::RTP_header(RTPType __type__, uint16_t __length__, uint32_t __seq_num__) : type(__type__),
                                                                                      length(__length__), seq_num(__seq_num__)
{
}

RTP_packet::RTP_packet(rtp_header_t __rtp__, const char *src_payload) : rtp(__rtp__)
{
    if (src_payload == nullptr)
    {
        set_checksum();
        return;
    }
    if (rtp.length > PAYLOAD_SIZE)
    {
        printf("error: payload size should not be greater than 1461 bytes\n");
        assert(false);
    }
    memcpy(payload, src_payload, rtp.length);
    set_checksum();
    return;
}

bool RTP_packet::check_type(RTPType __type__)
{
    return rtp.type == __type__;
}

bool RTP_packet::check_checksum()
{
    uint32_t cur_checksum = rtp.checksum;
    rtp.checksum = 0;
    set_checksum();
    return cur_checksum == rtp.checksum;
}

bool RTP_packet::check_ACK(const uint32_t &expect_seq_num)
{
    return expect_seq_num == rtp.seq_num && check_type(RTPType::ACK) && check_checksum();
}

void RTP_packet::set_checksum()
{
    rtp.checksum = 0;
    rtp.checksum = compute_checksum(reinterpret_cast<const void *>(this), sizeof(rtp_header_t) + rtp.length);
}

ssize_t SendRtpUdp(int fd, void *buf, int flags, const sockaddr *addr, socklen_t addr_len)
{
    ssize_t ret = 0;
    int len = reinterpret_cast<rtp_packet_t *>(buf)->rtp.length + sizeof(rtp_header_t);
    while (ret < len)
    {
        ssize_t b = sendto(fd, static_cast<char *>(buf) + ret, len - ret, 0, addr, addr_len);
        if (b == 0)
        {
            WINDOW_DEBUG(fmt::print("send_rtp_udp: socket closed\n");)
            return b;
        }
        if (b < 0)
        {
            WINDOW_DEBUG(fmt::print("send_rtp_udp: error, %d\n", errno);)
            WINDOW_DEBUG(fmt::print("fd:%d \n", fd);)
            return b;
        }
        ret += b;
    }
    return ret;
}

ssize_t RecvRtpUdp(int fd, void *__restrict__ buf, size_t n, int flags, sockaddr *__restrict__ addr, socklen_t *__restrict__ addr_len)
{
    ssize_t ret = 0;
    //读入RTP头
    n = 1472;
    ssize_t len = static_cast<ssize_t>(reinterpret_cast<rtp_header_t *>(buf)->length + sizeof(rtp_header_t));
    len = n;
    while (ret < len)
    {
        ssize_t b = recvfrom(fd, static_cast<char *>(buf) + ret, len - ret, flags, addr, addr_len);
        if (b >= static_cast<ssize_t>(sizeof(rtp_header_t)))
            len = reinterpret_cast<rtp_header_t *>(buf)->length + sizeof(rtp_header_t);
        if (b == 0)
        {
            WINDOW_DEBUG(fmt::print("recv_rtp_udp: socket closed\n");)
            return b;
        }
        if (b < 0)
        {
            WINDOW_DEBUG(fmt::print("recv_rtp_udp: error\n");)
            return b;
        }
        ret += b;
    }
    return ret;
    len = static_cast<ssize_t>(reinterpret_cast<rtp_header_t *>(buf)->length + sizeof(rtp_header_t));
    while (ret < len)
    {
        ssize_t b = recvfrom(fd, static_cast<char *>(buf) + ret, len - ret, flags, addr, addr_len);
        if (b == 0)
        {
            WINDOW_DEBUG(fmt::print("recv_rtp_udp: socket closed\n");)
            return b;
        }
        if (b < 0)
        {
            WINDOW_DEBUG(fmt::print("recv_rtp_udp: error\n");)
            return b;
        }
        ret += b;
    }
    return ret;
}

void RTP_packet::PrintInfo(uint32_t bit)
{
    if (bit & 1)
    {
        WINDOW_DEBUG(fmt::print("packet type is ");)
        if (rtp.type == RTPType::Start)
            WINDOW_DEBUG(fmt::print("START, ");)
        else if (rtp.type == RTPType::ACK)
            WINDOW_DEBUG(fmt::print("ACK, ");)
        else if (rtp.type == RTPType::Data)
            WINDOW_DEBUG(fmt::print("DATA, ");)
        else if (rtp.type == RTPType::End)
            WINDOW_DEBUG(fmt::print("END, ");)
        else if (rtp.type == RTPType::Empty)
            WINDOW_DEBUG(fmt::print("EMPTY, ");)
        else
            WINDOW_DEBUG(fmt::print("ERROR: {}, ", static_cast<uint32_t>(rtp.type));)
    }
    if (bit & 2)
        WINDOW_DEBUG(fmt::print("payload length is {}, ", static_cast<uint16_t>(rtp.length));)
    if (bit & 4)
        WINDOW_DEBUG(fmt::print("seq_num is {}, ", static_cast<uint32_t>(rtp.seq_num));)
    if (bit & 8)
        WINDOW_DEBUG(fmt::print("checksum is {}, ", static_cast<uint32_t>(rtp.checksum));)
    WINDOW_DEBUG(fmt::print("\n");)
}
