#ifndef RTP_H
#define RTP_H
//#define __WINDOW_DEBUG__
#include "util.h"
#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <errno.h>
#include <cstdio>
#include <vector>
#ifdef __WINDOW_DEBUG__
#define WINDOW_DEBUG(x) \
    {                   \
        x               \
    }
#else
#define WINDOW_DEBUG(x) ;
#endif
enum class PacketInfo : uint32_t
{
    Type = 1,
    Length = 1u << 1,
    SeqNum = 1u << 2,
    Checksum = 1u << 3
};

enum class RTPType : uint8_t
{
    Start = 0,
    End = 1,
    Data = 2,
    ACK = 3,
    Empty = 4
};
ssize_t
SendRtpUdp(int fd, void *buf, int flags, const sockaddr *addr, socklen_t addr_len);
ssize_t RecvRtpUdp(int fd, void *__restrict__ buf, size_t n, int flags, sockaddr *__restrict__ addr, socklen_t *__restrict__ addr_len);

#ifdef __cplusplus
extern "C"
{
#endif
#define PAYLOAD_SIZE 1461

    typedef struct __attribute__((__packed__)) RTP_header
    {
        RTPType type;    // 0: START; 1: END; 2: DATA; 3: ACK
        uint16_t length; // Length of data; 0 for ACK, START and END packets
        uint32_t seq_num;
        uint32_t checksum; // 32-bit CRC
        RTP_header(RTPType __type__ = RTPType::Empty, uint16_t __length__ = 0, uint32_t __seq_num__ = 0);
    } rtp_header_t;

    typedef struct __attribute__((__packed__)) RTP_packet
    {
        rtp_header_t rtp;
        char payload[PAYLOAD_SIZE];
        RTP_packet(rtp_header_t __rtp__ = RTP_header(), const char *__payload__ = nullptr);
        bool check_type(RTPType __type__);
        bool check_checksum();
        bool check_ACK(const uint32_t &expect_seq_num);
        void set_checksum();
        void PrintInfo(const uint32_t bit = 1u << 2);

    } rtp_packet_t;

#ifdef __cplusplus
}
#endif
void print_packet(const rtp_packet_t *packet);
#endif // RTP_H
