

#include "encode/bfilter/cbfilter_encode.h"

PairType CBFilterEncode::NEpairTest(uint32_t vertex1, uint32_t vertex2) {
#if UNDIRECTED == 1
    if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
    for (int i = 0; i < best_hash_nums_; ++i) {
        uint64_t hash_val = Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]);
        uint32_t count = encode_->BlockGet(hash_val, element_size_);
        if (count == 0) return PairType::NonNeighbor;
    }

    return PairType::Uncertain;
}

void CBFilterEncode::EdgeSet(uint32_t vertex1, uint32_t vertex2) {
#if UNDIRECTED == 1
    if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
    for (int i = 0; i < best_hash_nums_; ++i) {
        uint64_t hash_val = Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]);
        // std::cout<<"cbf:"<<"element_size_:"<<element_size_<<",element_nums_"<<element_nums_<<",v1:"<<vertex1<<",v2:"<<vertex2<<",hash
        // time:"<<i<<",hash val :"<<hash_val<<std::endl;
        uint32_t count = encode_->BlockGet(hash_val, element_size_);
        encode_->BlockSet(hash_val, element_size_, count + 1);
    }
}

void CBFilterEncode::InsertPair(uint32_t vertex1, uint32_t vertex2) { EdgeSet(vertex1, vertex2); }

bool CBFilterEncode::IsDeletable(uint32_t vertex1, uint32_t vertex2) { return true; }

void CBFilterEncode::DeletePair(uint32_t vertex1, uint32_t vertex2) {
    if (vertex1 > vertex2) std::swap(vertex1, vertex2);
    for (int i = 0; i < best_hash_nums_; ++i) {
        uint64_t hash_val = Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]);
        uint32_t count = encode_->BlockGet(hash_val, element_size_);
        encode_->BlockSet(hash_val, element_size_, count - 1);
    }
}
