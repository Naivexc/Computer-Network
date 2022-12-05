#include "rtp.h"
#include "receiver_def.h"
#include "Window.h"
#include <string.h>
#include <cassert>

/**
 * @brief 开启receiver并在所有IP的port端口监听等待连接
 *
 * @param port receiver监听的port
 * @param window_scize window大小
 * @return -1表示连接失败，0表示连接成功
 */
static int receiver_fd;
static std::unique_ptr<NetConnectionInfo> receiver_info_ptr;
static std::unique_ptr<NetConnectionInfo> sender_info_ptr;
static std::unique_ptr<ReceiverWindow> window_ptr;
int initReceiver(uint16_t port, uint32_t window_size)
{
    WINDOW_DEBUG(fmt::print("InitReceive : begin\n");)
    window_ptr = std::make_unique<ReceiverWindow>(0, static_cast<uint64_t>(window_size), window_size);
    receiver_info_ptr = std::make_unique<NetConnectionInfo>("0.0.0.0", socket(AF_INET, SOCK_DGRAM, 0));
    sender_info_ptr = std::make_unique<NetConnectionInfo>("", -1);

    std::unique_ptr<rtp_packet_t> send_packet = std::make_unique<rtp_packet_t>();
    std::unique_ptr<rtp_packet_t> recv_packet = std::make_unique<rtp_packet_t>();

    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];

    receiver_info_ptr->SetAddr(AF_INET, port, "0.0.0.0");
    window_ptr->SetConnectionInfo(*sender_info_ptr, *receiver_info_ptr);
    receiver_fd = window_ptr->GetReceiverInfo().GetFd();
    int bind_res = bind(receiver_fd, (struct sockaddr *)window_ptr->GetReceiverInfo().GetAddrPtr(), sizeof(sockaddr));
    assert(bind_res != -1);

    epoll_ctl(epfd, EPOLL_CTL_DEL, receiver_fd, &evt);

    evt.events = EPOLLIN;
    evt.data.fd = receiver_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, receiver_fd, &evt);

    int event_num = epoll_wait(epfd, events, 16, 10000);
    if (event_num == 0)
    {
        close(receiver_fd);
        WINDOW_DEBUG(fmt::print("InitReceiver : receive start packet timeout,exit\n");)
        return -1;
    }
    window_ptr->RecvStartPacket();
    if (!window_ptr->CheckStart())
    {
        terminateReceiver();
        WINDOW_DEBUG(fmt::print("InitReceiver : receive start packet wrong,exit\n");)
        return -1;
    }
    window_ptr->SetSendPacketForStartACK();
    window_ptr->SendPacket();
    WINDOW_DEBUG(fmt::print("InitReceiver : finished\n");)
    return 0;
}

/**
 * @brief 用于接收数据并在接收完后断开RTP连接
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */
int recvMessage(char *filename)
{
    //打开文件
    WINDOW_DEBUG(fmt::print("RecvMessage : start\n");)
    window_ptr->OpenFile(filename);
    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = receiver_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, receiver_fd, &evt);

    while (true)
    {
        int event_num = epoll_wait(epfd, events, 16, 10000);
        if (event_num == 0)
        {
            window_ptr->CloseFile();
            // terminateReceiver();
            WINDOW_DEBUG(fmt::print("RecvMessage : recv timeout,exit\n");)
            return -1;
        }
        window_ptr->RecvPacket();
        //若checksum不符，直接丢弃
        if (!window_ptr->CheckDataOrEnd())
            continue;
        // checksum符合
        if (window_ptr->GetRecvPacket().rtp.type == RTPType::End)
        {
            WINDOW_DEBUG(fmt::print("RecvMessage : recv finished\n");)
            return window_ptr->SendACKPacketForEnd();
        }
        // packet type 为 DATA
        if (window_ptr->DealWithRecvDataPacket() == 0)
            continue;
        window_ptr->SendPacket();
    }
    return -1;
}

/**
 * @brief 用于接收数据失败时断开RTP连接以及关闭UDP socket
 */
void terminateReceiver()
{
    close(receiver_fd);
    return;
}

/**
 * @brief 用于接收数据并在接收完后断开RTP连接(优化版本的RTP)
 * @param filename 用于接收数据的文件名
 * @return >0表示接收完成后到数据的字节数 -1表示出现其他错误
 */
int recvMessageOpt(char *filename)
{
    window_ptr->OpenFile(filename);
    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = receiver_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, receiver_fd, &evt);

    while (true)
    {
        int event_num = epoll_wait(epfd, events, 16, 10000);
        if (event_num == 0)
        {
            // terminateReceiver();
            window_ptr->CloseFile();
            WINDOW_DEBUG(fmt::print("RecvMessageOpt : recv timeout,exit\n");)
            return -1;
        }
        window_ptr->RecvPacket();
        //若checksum不符，直接丢弃
        if (!window_ptr->CheckDataOrEnd())
            continue;
        // checksum符合
        if (window_ptr->GetRecvPacket().rtp.type == RTPType::End)
        {
            return window_ptr->SendACKPacketForEnd();
        }
        // packet type 为 DATA
        if (window_ptr->DealWithRecvDataPacketOpt() == 0)
            continue;
        window_ptr->SendPacket();
    }
    return -1;
}