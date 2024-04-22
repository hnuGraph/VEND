
#ifndef VEND_BITHASH_ENCODE_H
#define VEND_BITHASH_ENCODE_H

#include "encode/single/encode_bitset.h"
#include "algorithm"

class BitHashEncode : public EncodeBitSet {


public:
    BitHashEncode() : hash_size_(PER_ENCODE_BIT_SIZE), EncodeBitSet() {}

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        if (vertex1 < vertex2)
            return NonNeighborTest(vertex1, vertex2);
        return NonNeighborTest(vertex2, vertex1);
    }

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override {
        if(encode_bitset_[vertex1]->IsOne(Hash(vertex2)))
            return PairType::Uncertain;
        else
            return PairType::NonNeighbor;

    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        auto &encode = encode_bitset_[vertex_id];
        encode->Clear();
        for (auto &v:neighbors) {
            if (vertex_id<v)
                encode->SetOne(Hash(v));
        }
    };

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override {
        if (vertex1 < vertex2)
            encode_bitset_[vertex1]->SetOne(Hash(vertex2));
        else
            encode_bitset_[vertex2]->SetOne(Hash(vertex1));
    };

    void DeletePair(uint32_t vertex1, uint32_t vertex2) override {
        if (vertex1 > vertex2)
            std::swap(vertex1, vertex2);
        std::vector<uint32_t> neighbors = DbQuery(vertex1);
        auto iter = std::find(neighbors.begin(), neighbors.end(), vertex2);
        if(iter!=neighbors.end())
            neighbors.erase(iter);
        encode_bitset_[vertex1]->Clear();
        EncodeVertex(vertex1, neighbors);
    };

private:
    uint32_t hash_size_;

    inline uint32_t Hash(uint32_t vertex) {
        return vertex % hash_size_;
    };
};


#endif //VEND_BITHASH_ENCODE_H
