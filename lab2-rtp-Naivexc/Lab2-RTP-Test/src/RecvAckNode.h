#ifndef RECV_ACK_NODE_HPP
#define RECV_ACK_NODE_HPP
#include <cstdint>
class RecvAckNode
{
    friend class VecRecvAck;

private:
    uint64_t last_num_;
    uint64_t next_num_;

public:
    RecvAckNode(const uint64_t &__last_num__, const uint64_t &__next_num__);
};
#endif