#include "VecRecvAck.h"

void VecRecvAck::Erase(const uint64_t &num)
{
    if (num == first_un_recv_)
        first_un_recv_ = vec_[num].next_num_;
    if (vec_[num].last_num_ >= 0 && vec_[num].last_num_ < vec_.size())
        vec_[vec_[num].last_num_].next_num_ = vec_[num].next_num_;
    if (vec_[num].next_num_ >= 0 && vec_[num].last_num_ < vec_.size())
        vec_[vec_[num].next_num_].last_num_ = vec_[num].last_num_;
}

void VecRecvAck::PushBack()
{
    if (vec_.empty())
    {
        first_un_recv_ = 0;
        vec_.push_back(RecvAckNode(-1, 1));
    }
    else
    {
        vec_.back().next_num_ = vec_.size();
        vec_.push_back(RecvAckNode(vec_.size() - 1, vec_.size() + 1));
    }
}

uint64_t VecRecvAck::GetFirstUnRecv()
{
    return first_un_recv_;
}

uint64_t VecRecvAck::GetNextNum(const uint64_t &num)
{
    return vec_[num].next_num_;
}