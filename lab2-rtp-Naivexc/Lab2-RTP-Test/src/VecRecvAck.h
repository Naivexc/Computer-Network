#ifndef VEC_RECV_ACK_H
#define VEC_RECV_ACK_H
#include "RecvAckNode.h"
#include <vector>
#include <cstdint>
class VecRecvAck
{
private:
    std::vector<RecvAckNode> vec_;
    uint64_t first_un_recv_;

public:
    /**
     * @brief 将num号packet从未收到的packet组中移除
     */
    void Erase(const uint64_t &num);
    /**
     * @brief 向packet组中添加一个packet
     */
    void PushBack();
    /**
     * @brief
     */
    uint64_t GetFirstUnRecv();
    uint64_t GetNextNum(const uint64_t &num);
};

#endif