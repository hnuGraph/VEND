
#ifndef VEND_BBFILTER_ENCODE_H
#define VEND_BBFILTER_ENCODE_H

#include "bfilter_bit_encode.h"

class BBFilterEncode : public BFilterBitEncode {
   public:
    BBFilterEncode() : BFilterBitEncode(), block_size_(BLOCK_SIZE), block_nums_(BLOCK_NUMS) {
        //            best_hash_nums_=1;
    }

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
#if UNDIRECTED == 1
        if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
        uint64_t block_offset = BlockHash(vertex1, vertex2) * block_size_;
        for (int i = 0; i < best_hash_nums_; ++i) {
            if (!encode_->IsOne(block_offset + Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i])))
                return PairType::NonNeighbor;
        }
        return PairType::Uncertain;
    };

    void EdgeSet(uint32_t vertex1, uint32_t vertex2) override {
#if UNDIRECTED == 1
        if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
        uint64_t block_offset = BlockHash(vertex1, vertex2) * block_size_;
        for (int i = 0; i < best_hash_nums_; ++i)
            encode_->SetOne(block_offset + Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]));
    };

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override { EdgeSet(vertex1, vertex2); };

    // return block index
    uint64_t BlockHash(uint32_t key1, uint32_t key2) {
        return ((uint64_t)key1 * block_hash1_ + (uint64_t)key2 * block_hash2_) % block_nums_;
    }

    uint64_t Hash(uint32_t key1, uint32_t key2, uint32_t hash_param1, uint32_t hash_param2) override {
        return ((uint64_t)key1 * hash_param1 + (uint64_t)key2 * hash_param2) % block_size_;
    }

   protected:
    // per bits for counting
    uint32_t block_size_;
    // how many elements for the whole bitset
    uint64_t block_nums_;

    uint32_t block_hash1_ = 19692027;
    uint32_t block_hash2_ = 3201251;
};

#endif  // VEND_BBFILTER_ENCODE_H
