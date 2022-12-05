#ifndef WINDOW_H
#define WINDOW_H
#include "rtp.h"
#include "NetConnectionInfo.h"
#include "VecRecvAck.h"
#include <vector>
#include <cstdint>
#include <memory>
#include <iostream>
#include <fstream>
#include <climits>
#ifdef __WINDOW_DEBUG__
#include <fmt/core.h>
#endif

class BaseWindow
{
    friend class SenderWindow;
    friend class ReceiverWindow;

public:
    BaseWindow(const uint64_t &begin = 0, const uint64_t &end = 0, const uint64_t &size = 0);
    uint64_t GetBegin();
    uint64_t GetEnd();
    uint64_t GetSize() const;
    const rtp_packet_t &GetRecvPacket();
    rtp_packet_t &GetSendPacket();
    NetConnectionInfo &GetSenderInfo();
    NetConnectionInfo &GetReceiverInfo();
    bool BeginReachEnd();
    void SetConnectionInfo(NetConnectionInfo sender_info, NetConnectionInfo receiver_info);
    virtual size_t SendPacket(rtp_packet_t *packet = nullptr) = 0;
    virtual size_t RecvPacket() = 0;
    bool CheckACK();

private:
    uint64_t begin_;
    uint64_t end_;
    const uint64_t size_;
    std::ofstream log_;
    std::unique_ptr<NetConnectionInfo> receiver_info_ptr_;
    std::unique_ptr<NetConnectionInfo> sender_info_ptr_;
    std::unique_ptr<rtp_packet_t> recv_packet_ptr_;
    std::unique_ptr<rtp_packet_t> send_packet_ptr_;
};

class SenderWindow : public BaseWindow
{
public:
    SenderWindow(const uint64_t &begin = 0, const uint64_t &end = 0, const uint64_t &size = 0);
    uint64_t GetNextNum();
    VecRecvAck &GetVecRecvAck();
    void ResetNextPacket();
    void ResetNextPacket(const uint64_t &next);
    bool CheckStartACK();
    bool CheckEndACK();
    virtual size_t SendPacket(rtp_packet_t *packet = nullptr) override;
    virtual size_t RecvPacket() override;

    /**
     * @brief 收到一个ACK后更新window位置
     * @param recv_num 收到的ACK的seq值
     */
    void MoveWindow(const uint64_t &recv_num);
    /**
     * 发完了当前窗口内的所有data packet
     */
    bool Finish();
    int32_t ReadFile(const char *file_name);
    size_t SendStart();
    size_t SendNext();
    size_t SendNextOpt();
    ssize_t SendEnd();

private:
    uint64_t next_packet_;                               //下一个要发的包号
    uint64_t total_packet_num_;                          //总共的包数
    std::vector<std::unique_ptr<rtp_packet_t>> packets_; //待发送的所有包
    VecRecvAck recv_ack_;                                //关于收到的ack的一个结构，用于OPT
    uint32_t start_packet_seq_num_;                      // start packet的seq_num,是一个随机数
};

class ReceiverWindow : public BaseWindow
{
public:
    ReceiverWindow(const uint64_t &begin = 0, const uint64_t &end = 0, const uint64_t &size = 0);
    virtual size_t SendPacket(rtp_packet_t *packet = nullptr) override;
    virtual size_t RecvPacket() override;
    size_t SendACKPacketForEnd();
    bool CheckDataOrEnd();
    bool CheckStart();

    /**
     * @brief 由于第一个包要获得Sender的信息,从而单拎出来一个函数
     */
    ssize_t RecvStartPacket();
    void SetSendPacketForStartACK();
    /**
     * @brief 处理收到的Data,设置send_packet
     * @return 0表示丢弃此包,1表示发送设置好的send_packet
     */
    int32_t DealWithRecvDataPacket();
    int32_t DealWithRecvDataPacketOpt();
    void OpenFile(const char *file_name);
    void CloseFile();

private:
    std::vector<std::unique_ptr<rtp_packet_t>> vec_recv_packet_;
    uint32_t expect_seq_num_ = 0;
    size_t total_write_size_ = 0;
    uint32_t num_already_written_ = 0;
    std::ofstream file_to_write_;
    std::string file_name_;
};

#endif