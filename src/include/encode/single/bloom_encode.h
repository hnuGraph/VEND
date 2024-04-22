
#ifndef VEND_BLOOM_FILTER_H
#define VEND_BLOOM_FILTER_H

#include "encode_bitset.h"
#include "include/common/hash_config.h"
#include <algorithm>

class BloomEncode : public EncodeBitSet {

public:
    BloomEncode() : EncodeBitSet(), block_size_(K_SIZE - 1), bit_size_((K_SIZE - 1) * 32) {}

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {

        if (neighbors.size() <= block_size_)
            SetFullEncode(vertex_id, neighbors);
        else
            SetBloomEncode(vertex_id, neighbors);
    }

    void SetFullEncode(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
        SetFlag(vertex_id, true);
        uint32_t offset = 32;
        for (int i = 0; i < neighbors.size(); ++i) {
            encode_bitset_[vertex_id]->BlockSet(offset, 32, neighbors[i]);
            offset += 32;
        }
    }

    void SetBloomEncode(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
        SetFlag(vertex_id, false);
        for (auto v: neighbors) {
            encode_bitset_[vertex_id]->SetOne(Hash(vertex_id, v));
        }
    }

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        return std::min(NonNeighborTest(vertex1, vertex2), NonNeighborTest(vertex2, vertex1));
    }

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override {
        if (GetFlag(vertex1)) {
            uint32_t offset = 32;
            for (int i = 0; i < block_size_; ++i) {
                uint32_t v = encode_bitset_[vertex1]->BlockGet(offset, 32);
                offset += 32;
                if (v == vertex2)
                    return PairType::Neighbor;
            }
            return PairType::NonNeighbor;
        } else {
            if (encode_bitset_[vertex1]->IsOne(Hash(vertex1, vertex2)))
                return PairType::Uncertain;
            else
                return PairType::NonNeighbor;
        }
    }

    void SetFlag(uint32_t vertex_id, bool flag) {
        encode_bitset_[vertex_id]->BlockSet(0, 32, flag);
    }

    bool GetFlag(uint32_t vertex_id) {
        return encode_bitset_[vertex_id]->BlockGet(0, 32) != 0;
    }

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override {}

    uint32_t GetIntSize() { return K_SIZE-1; }

    uint32_t Hash(uint32_t vertex1, uint32_t vertex2) {
        return ((uint64_t)HASH_PARAM1[0] * vertex1 +(uint64_t) HASH_PARAM2[1] * vertex2) % bit_size_ + 32;
    }

private:
    uint32_t block_size_;
    uint32_t bit_size_;

};


#endif //VEND_BLOOM_FILTER_H
