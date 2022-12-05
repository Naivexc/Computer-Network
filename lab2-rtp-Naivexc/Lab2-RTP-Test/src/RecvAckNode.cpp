#include "RecvAckNode.h"
RecvAckNode::RecvAckNode(const uint64_t &__last_num__, const uint64_t &__next_num__)
    : last_num_(__last_num__), next_num_(__next_num__){};