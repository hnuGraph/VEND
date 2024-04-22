
#ifndef VEND_BLOCK_ENCODE_H
#define VEND_BLOCK_ENCODE_H

#include "encode_bitset.h"
#include "include/common/hash_config.h"
#include <algorithm>

class CountEncode : public EncodeBitSet {
public:

    CountEncode() : EncodeBitSet(), element_size_(ELEMENT_SIZE), element_nums_((K_SIZE-1) * 32 / ELEMENT_SIZE ) {}


    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {

        if (neighbors.size() <= element_nums_)
            SetFullEncode(vertex_id, neighbors);
        else
            SetCountEncode(vertex_id,neighbors);
    }


    void SetFullEncode(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
        SetFlag(vertex_id, true);
        uint32_t offset = 32;
        for (int i = 0; i < neighbors.size(); ++i) {
            encode_bitset_[vertex_id]->BlockSet(offset, element_size_, neighbors[i]);
            offset += element_size_;
        }
    }

    void SetCountEncode(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
        SetFlag(vertex_id, false);
        for (auto v: neighbors) {
            uint32_t offset = Hash(vertex_id, v) * element_size_;
            uint32_t val = encode_bitset_[vertex_id]->BlockGet(offset, element_size_);
            encode_bitset_[vertex_id]->BlockSet(Hash(vertex_id, v) * element_size_, element_size_, val + 1);
        }
    }

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        return std::min(NonNeighborTest(vertex1,vertex2), NonNeighborTest(vertex2,vertex1));
    }

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override {
        if (GetFlag(vertex1)) {
            uint32_t offset = 32;
            for (int i = 0; i < element_nums_; ++i) {

                uint32_t v = encode_bitset_[vertex1]->BlockGet(offset,element_size_);
                offset+=element_size_;
                if(v==vertex2)
                    return PairType::Neighbor;
                else if(v<vertex2)
                    return PairType::NonNeighbor;
            }
            return PairType::NonNeighbor;
        } else {
            uint32_t offset= Hash(vertex1,vertex2)*element_size_;
            if(encode_bitset_[vertex1]->BlockGet(offset,element_size_)==0)
                return PairType::NonNeighbor;
            else
                return PairType::Uncertain;
        }
    }

    void SetFlag(uint32_t vertex_id, bool flag) {
        encode_bitset_[vertex_id]->BlockSet(0, 32, flag);
    }

    bool GetFlag(uint32_t vertex_id) {
        return encode_bitset_[vertex_id]->BlockGet(0, 32) != 0;
    }

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override{}

    uint32_t GetIntSize() { return element_size_; }
    uint32_t Hash(uint32_t vertex1, uint32_t vertex2) {
        return ((uint64_t)HASH_PARAM1[0] * vertex1 + (uint64_t)HASH_PARAM2[1] * vertex2) % element_nums_ + 1;
    }


private:
    // per bits for counting
    uint32_t element_size_;
    // how many elements for the whole bitset
    uint32_t element_nums_;

};


#endif //VEND_BLOCK_ENCODE_H
