#include "sender_def.h"
#include <signal.h>
#include "rtp.h"
#include "Window.h"
#include "NetConnectionInfo.h"
#include "RecvAckNode.h"
#include "VecRecvAck.h"
#include "time.h"
#ifdef __WINDOW_DEBUG__
#include <fmt/core.h>
#endif
static std::unique_ptr<NetConnectionInfo> receiver_info_ptr;
static std::unique_ptr<NetConnectionInfo> sender_info_ptr;
static std::unique_ptr<SenderWindow> window_ptr;
/**
 * @brief 用于建立RTP连接
 * @param receiver_ip receiver的IP地址
 * @param receiver_port receiver的端口
 * @param window_size window大小
 * @return -1表示连接失败，0表示连接成功
 **/

int initSender(const char *receiver_ip, uint16_t receiver_port, uint32_t window_size)
{
    //更新全局变量
    window_ptr = std::make_unique<SenderWindow>(0, static_cast<uint64_t>(window_size), window_size);
    receiver_info_ptr = std::make_unique<NetConnectionInfo>(receiver_ip, -1);
    sender_info_ptr = std::make_unique<NetConnectionInfo>("", socket(AF_INET, SOCK_DGRAM, 0));

    int socket_fd = sender_info_ptr->GetFd();
    // socket_fd
    if (socket_fd == -1)
    {
        return -1;
    }

    receiver_info_ptr->SetAddr(AF_INET, receiver_port, receiver_ip);

    window_ptr->SetConnectionInfo(*sender_info_ptr, *receiver_info_ptr);

    window_ptr->SendStart();

    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = sender_info_ptr->GetFd();
    epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &evt);

    int event_num = epoll_wait(epfd, events, 16, 100);
    if (event_num == 0)
    {
        // terminateSender();
        return -1;
    }

    //等待packet
    window_ptr->RecvPacket();

    if (window_ptr->CheckStartACK() == false)
    {
        // terminateSender();
        return -1;
    }
    return 0;
}

/**
 * @brief 用于发送数据
 * @param message 要发送的文件名
 * @return -1表示发送失败，0表示发送成功
 **/
int sendMessage(const char *message)
{
    int socket_fd = sender_info_ptr->GetFd();
    window_ptr->ReadFile(message);
    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = sender_info_ptr->GetFd();
    epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &evt);

    clock_t begin_time = clock();
    clock_t end_time;

    while (true)
    {
        if (window_ptr->BeginReachEnd())
            break;
        int event_num = epoll_wait(epfd, events, 16, 5);
        end_time = clock();

        if ((end_time - begin_time) >= 300)
        {
            window_ptr->ResetNextPacket();
            WINDOW_DEBUG(fmt::print("sender timeout: do not receive ack\n");)
            begin_time = clock();
        }
        for (int i = 0; i < event_num; ++i)
        {
            if (events[i].events & EPOLLIN)
            {
                window_ptr->RecvPacket();
                if (window_ptr->CheckACK() == false)
                    break;
                window_ptr->MoveWindow(window_ptr->GetRecvPacket().rtp.seq_num);
                begin_time = clock();
                break;
            }
        }
        if (window_ptr->Finish())
        {
            continue;
        }
        window_ptr->SendNext();
        begin_time = clock();
    }
    return 0;
}

/**
 * @brief 用于断开RTP连接以及关闭UDP socket
 **/
void terminateSender()
{
    int socket_fd = sender_info_ptr->GetFd();
    window_ptr->SendEnd();
    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = socket_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd, &evt);
    while (true)
    {
        int event_num = epoll_wait(epfd, events, 16, 100);
        if (event_num == 0)
        {
            //超时
            close(socket_fd);
            WINDOW_DEBUG(fmt::print("Sender: timeout: doesn't receive End ACK, close connection\n");)
            return;
        }
        if (events[0].events & EPOLLIN)
        {
            window_ptr->RecvPacket();
            if (window_ptr->CheckEndACK())
            {
                close(socket_fd);
                WINDOW_DEBUG(fmt::print("Sender: receiver End ACK, close connection\n");)
                return;
            }
            WINDOW_DEBUG(fmt::print("Sender: receive packet but is not End ACK, continue to wait\n");)
            continue;
        }
    }
}

/**
 * @brief 用于发送数据(优化版本的RTP)
 * @param message 要发送的文件名
 * @return -1表示发送失败，0表示发送成功
 **/
int sendMessageOpt(const char *message)
{
    window_ptr->ReadFile(message);

    int epfd = epoll_create(128);
    struct epoll_event evt, events[16];
    evt.events = EPOLLIN;
    evt.data.fd = sender_info_ptr->GetFd();
    epoll_ctl(epfd, EPOLL_CTL_ADD, sender_info_ptr->GetFd(), &evt);
    clock_t begin_time = clock();
    clock_t end_time;
    while (true)
    {
        if (window_ptr->BeginReachEnd())
            break;
        int event_num = epoll_wait(epfd, events, 16, 5);
        end_time = clock();
        //重发
        if ((end_time - begin_time) >= 300)
        {
            WINDOW_DEBUG(fmt::print("Sender: timeout resend packet\n");)
            window_ptr->ResetNextPacket();
            begin_time = clock();
        }
        for (int i = 0; i < event_num; ++i)
        {
            if (events[i].events & EPOLLIN)
            {
                window_ptr->RecvPacket();
                if (!window_ptr->CheckACK())
                    break;
                window_ptr->GetVecRecvAck().Erase(window_ptr->GetRecvPacket().rtp.seq_num);
                window_ptr->MoveWindow(window_ptr->GetVecRecvAck().GetFirstUnRecv());
                begin_time = clock();
                break;
            }
        }
        if (window_ptr->Finish())
        {
            continue;
        }
        window_ptr->SendNextOpt();
        begin_time = clock();
    }
    return 0;
}