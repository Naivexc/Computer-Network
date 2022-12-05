#include "Window.h"
#include <fstream>
#include <cstring>
#include <iostream>
BaseWindow::BaseWindow(const uint64_t &begin, const uint64_t &end, const uint64_t &size)
    : begin_(begin), end_(end), size_(size), recv_packet_ptr_(std::make_unique<rtp_packet_t>()), send_packet_ptr_(std::make_unique<rtp_packet_t>()) {}
const rtp_packet_t &BaseWindow::GetRecvPacket()
{
    return *recv_packet_ptr_;
}
rtp_packet_t &BaseWindow::GetSendPacket()
{
    return *send_packet_ptr_;
}
NetConnectionInfo &BaseWindow::GetSenderInfo()
{
    return *sender_info_ptr_;
}
NetConnectionInfo &BaseWindow::GetReceiverInfo()
{
    return *receiver_info_ptr_;
}
uint64_t BaseWindow::GetBegin()
{
    return begin_;
}
uint64_t BaseWindow::GetEnd()
{
    return end_;
}
uint64_t BaseWindow::GetSize() const
{
    return size_;
}
bool BaseWindow::BeginReachEnd()
{
    return end_ == begin_;
}
void BaseWindow::SetConnectionInfo(NetConnectionInfo sender_info, NetConnectionInfo receiver_info)
{
    receiver_info_ptr_ = std::make_unique<NetConnectionInfo>(std::move(receiver_info));
    sender_info_ptr_ = std::make_unique<NetConnectionInfo>(std::move(sender_info));
}
/**
 * @brief 只会检查checksum和type为ACK,不会检查具体的seqnum
 */
bool BaseWindow::CheckACK()
{
    return recv_packet_ptr_->check_checksum() && recv_packet_ptr_->check_type(RTPType::ACK) && recv_packet_ptr_->rtp.length == 0;
}

SenderWindow::SenderWindow(const uint64_t &begin, const uint64_t &end, const uint64_t &size)
    : BaseWindow(begin, end, size), next_packet_(0)
{
    WINDOW_DEBUG(log_ = std::ofstream("sender_log.txt", std::ios::out | std::ios::trunc);)
}
size_t SenderWindow::SendPacket(rtp_packet_t *packet)
{
    if (packet == nullptr)
        packet = send_packet_ptr_.get();
    WINDOW_DEBUG(packet->PrintInfo(static_cast<uint32_t>(PacketInfo::SeqNum));)
    return SendRtpUdp(sender_info_ptr_->GetFd(), reinterpret_cast<void *>(packet), 0,
                      reinterpret_cast<sockaddr *>(receiver_info_ptr_->GetAddrPtr()), sizeof(sockaddr_in));
}
size_t SenderWindow::RecvPacket()
{
    size_t ret = RecvRtpUdp(sender_info_ptr_->GetFd(), reinterpret_cast<void *>(recv_packet_ptr_.get()), INT_MAX, 0, nullptr, nullptr);
    WINDOW_DEBUG(fmt::print("Sender: recv packet ");)
    WINDOW_DEBUG(recv_packet_ptr_->PrintInfo(static_cast<uint32_t>(PacketInfo::SeqNum));)
    return ret;
}
bool SenderWindow::CheckStartACK()
{
    return recv_packet_ptr_->rtp.type == RTPType::ACK && recv_packet_ptr_->rtp.length == 0 &&
           recv_packet_ptr_->rtp.seq_num == start_packet_seq_num_ && recv_packet_ptr_->check_checksum();
}
bool SenderWindow::CheckEndACK()
{
    return recv_packet_ptr_->rtp.type == RTPType::ACK && recv_packet_ptr_->rtp.length == 0 &&
           recv_packet_ptr_->rtp.seq_num == begin_ && recv_packet_ptr_->check_checksum();
}
VecRecvAck &SenderWindow::GetVecRecvAck()
{
    return recv_ack_;
}
uint64_t SenderWindow::GetNextNum()
{
    return next_packet_;
}
/**
 * @brief 重设下一个要发的包
 * @param 没有参数时设为窗口起点
 */
void SenderWindow::ResetNextPacket()
{
    WINDOW_DEBUG(log_ << "reset next package to " << begin_ << '\n';)
    next_packet_ = begin_;
}

/**
 * @brief 重设下一个要发的包
 * @param next 将next_packet重设为next
 */
void SenderWindow::ResetNextPacket(const uint64_t &next)
{
    WINDOW_DEBUG(log_ << "reset next package to " << next << '\n';)
    next_packet_ = next;
}

void SenderWindow::MoveWindow(const uint64_t &recv_num)
{
    if (recv_num <= begin_)
        return;
    begin_ = recv_num;
    end_ = begin_ + size_;
    if (end_ > total_packet_num_)
        end_ = total_packet_num_;
    WINDOW_DEBUG(log_ << "move window to [" << begin_ << ", " << end_ << " )\n";)
}

/**
 * @brief 读取文件
 * @return 返回读取的字节数
 */
int32_t SenderWindow::ReadFile(const char *file_name)
{
    std::ifstream file(file_name, std::ios::in | std::ios::binary);
    char buffer[PAYLOAD_SIZE];
    memset(buffer, 0, PAYLOAD_SIZE);
    int32_t read_size = 0;
    while (true)
    {
        file.read(buffer, PAYLOAD_SIZE);
        uint16_t cur_read_size = file.gcount(); //本次读入的字节数
        read_size += cur_read_size;
        if (cur_read_size == 0)
            break;
        packets_.push_back(std::move(std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::Data, cur_read_size, total_packet_num_++), buffer)));
        recv_ack_.PushBack();
    }
    file.close();
    MoveWindow(0);
    return read_size;
}
/**
 * 发完了当前窗口内的所有data packet
 */
bool SenderWindow::Finish()
{
    return next_packet_ == end_;
}
size_t SenderWindow::SendNext()
{
    WINDOW_DEBUG(fmt::print("Sender: send packet ");)
    return SendPacket(packets_[next_packet_++].get());
}
size_t SenderWindow::SendStart()
{
    srand(time(nullptr));
    start_packet_seq_num_ = rand();
    send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_packet_t(rtp_header_t(RTPType::Start, 0, start_packet_seq_num_), nullptr));
    return SendPacket();
}
size_t SenderWindow::SendNextOpt()
{
    WINDOW_DEBUG(log_ << "send data packet " << next_packet_ << '\n';)
    size_t ret = SendPacket(packets_[next_packet_].get());
    ResetNextPacket(recv_ack_.GetNextNum(GetNextNum()));
    return ret;
}
ssize_t SenderWindow::SendEnd()
{
    send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::End, 0, begin_), nullptr);
    return SendPacket();
}

ReceiverWindow::ReceiverWindow(const uint64_t &begin, const uint64_t &end, const uint64_t &size)
    : BaseWindow(begin, end, size), expect_seq_num_(0), total_write_size_(0), num_already_written_(0)
{
    WINDOW_DEBUG(log_ = std::ofstream("sender_log.txt", std::ios::out | std::ios::trunc);)
}
void ReceiverWindow::OpenFile(const char *file_name)
{
    file_to_write_.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
    file_name_ = std::string(file_name);
}
void ReceiverWindow::CloseFile()
{
    file_to_write_.close();
}
size_t ReceiverWindow::SendPacket(rtp_packet_t *packet)
{
    if (packet == nullptr)
        packet = send_packet_ptr_.get();
    // std::cout << "Receiver: send packet " << packet->rtp.seq_num << '\n';
    WINDOW_DEBUG(fmt::print("Receiver: send packet ");)
    WINDOW_DEBUG(packet->PrintInfo();)
    return SendRtpUdp(receiver_info_ptr_->GetFd(), reinterpret_cast<void *>(packet), 0,
                      reinterpret_cast<sockaddr *>(sender_info_ptr_->GetAddrPtr()), sizeof(sockaddr_in));
}
size_t ReceiverWindow::RecvPacket()
{
    size_t ret = RecvRtpUdp(receiver_info_ptr_->GetFd(), reinterpret_cast<void *>(recv_packet_ptr_.get()), INT_MAX, 0, nullptr, nullptr);
    WINDOW_DEBUG(fmt::print("Receiver: recv packet ");)
    // std::cout << "Receiver: recv packet " << recv_packet_ptr_->rtp.seq_num << '\n';
    WINDOW_DEBUG(recv_packet_ptr_->PrintInfo();)
    return ret;
}
ssize_t ReceiverWindow::RecvStartPacket()
{
    size_t ret = RecvRtpUdp(receiver_info_ptr_->GetFd(), reinterpret_cast<void *>(recv_packet_ptr_.get()), INT_MAX, 0, reinterpret_cast<sockaddr *>(sender_info_ptr_->GetAddrPtr()), sender_info_ptr_->GetAddrLenPtr());
    WINDOW_DEBUG(fmt::print("ReceiverWindow::RecvStartPacket\n");)
    WINDOW_DEBUG(fmt::print("Receiver: recv packet ");)
    // std::cout << "Receiver: recv START packet " << '\n';
    WINDOW_DEBUG(recv_packet_ptr_->PrintInfo();)
    return ret;
}
void ReceiverWindow::SetSendPacketForStartACK()
{
    send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::ACK, 0, recv_packet_ptr_->rtp.seq_num), nullptr);
    return;
}
bool ReceiverWindow::CheckDataOrEnd()
{
    if (!recv_packet_ptr_->check_checksum())
        return false;
    return recv_packet_ptr_->check_type(RTPType::Data) || recv_packet_ptr_->check_type(RTPType::End);
}
size_t ReceiverWindow::SendACKPacketForEnd()
{
    send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::ACK, 0, recv_packet_ptr_->rtp.seq_num), nullptr);
    SendPacket();
    file_to_write_.close();
    return total_write_size_;
}
int32_t ReceiverWindow::DealWithRecvDataPacket()
{
    uint32_t seq_num = recv_packet_ptr_->rtp.seq_num;
    if (seq_num >= expect_seq_num_ + size_ || seq_num < expect_seq_num_)
    {
        return 0;
    }
    if (seq_num >= vec_recv_packet_.size())
    {

        vec_recv_packet_.resize(seq_num + 1);
    }
    //必有 seq_num>=expect_seq_num
    if (seq_num > expect_seq_num_)
    {
        vec_recv_packet_[seq_num] = std::move(recv_packet_ptr_);
        recv_packet_ptr_ = std::make_unique<rtp_packet_t>();
        send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::ACK, 0, expect_seq_num_), nullptr);
    }
    else
    {
        vec_recv_packet_[seq_num] = std::move(recv_packet_ptr_);
        recv_packet_ptr_ = std::make_unique<rtp_packet_t>();
        uint64_t min_not_cashed_seq_num = num_already_written_;
        while (min_not_cashed_seq_num < vec_recv_packet_.size() && vec_recv_packet_[min_not_cashed_seq_num] != nullptr)
            ++min_not_cashed_seq_num;
        expect_seq_num_ = min_not_cashed_seq_num;
        send_packet_ptr_ = std::make_unique<rtp_packet_t>(rtp_header_t(RTPType::ACK, 0, min_not_cashed_seq_num), nullptr);
        for (uint32_t i = num_already_written_; i < min_not_cashed_seq_num; ++i)
        {
            file_to_write_.write(vec_recv_packet_[i]->payload, vec_recv_packet_[i]->rtp.length);
            WINDOW_DEBUG(fmt::print("receiver: write packet {}\n", i);)
            // std::cout << "Receiver: write packet " << i << '\n';
            total_write_size_ += vec_recv_packet_[i]->rtp.length;
            ++num_already_written_;
        }
    }
    return 1;
}

bool ReceiverWindow::CheckStart()
{
    return recv_packet_ptr_->check_checksum() && recv_packet_ptr_->check_type(RTPType::Start) && recv_packet_ptr_->rtp.length == 0;
}

int32_t ReceiverWindow::DealWithRecvDataPacketOpt()
{
    uint32_t seq_num = recv_packet_ptr_->rtp.seq_num;
    if (seq_num >= expect_seq_num_ + size_)
    {
        return 0;
    }
    if (seq_num >= vec_recv_packet_.size())
    {
        vec_recv_packet_.resize(seq_num + 1);
    }
    // if (seq_num < expect_seq_num_)
    //{
    //     return 0;
    // }
    //必有 seq_num>=expect_seq_num
    if (seq_num > expect_seq_num_)
    {
        vec_recv_packet_[seq_num] = std::move(recv_packet_ptr_);
        recv_packet_ptr_ = std::make_unique<rtp_packet_t>();
        *send_packet_ptr_ = rtp_packet_t(rtp_header_t(RTPType::ACK, 0, seq_num), nullptr);
    }
    else if (seq_num == expect_seq_num_)
    {
        vec_recv_packet_[seq_num] = std::move(recv_packet_ptr_);
        recv_packet_ptr_ = std::make_unique<rtp_packet_t>();
        uint64_t min_not_cashed_seq_num = num_already_written_;
        while (min_not_cashed_seq_num < vec_recv_packet_.size() && vec_recv_packet_[min_not_cashed_seq_num] != nullptr)
            ++min_not_cashed_seq_num;
        expect_seq_num_ = min_not_cashed_seq_num;
        *send_packet_ptr_ = rtp_packet_t(rtp_header_t(RTPType::ACK, 0, seq_num), nullptr);
        for (uint32_t i = num_already_written_; i < min_not_cashed_seq_num; ++i)
        {

            file_to_write_.write(vec_recv_packet_[i]->payload, vec_recv_packet_[i]->rtp.length);
            total_write_size_ += vec_recv_packet_[i]->rtp.length;
            ++num_already_written_;
        }
    }
    else
    {
        *send_packet_ptr_ = rtp_packet_t(rtp_header_t(RTPType::ACK, 0, seq_num), nullptr);
    }
    return 1;
}