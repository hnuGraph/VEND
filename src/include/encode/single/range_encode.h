
#ifndef VEND_RANGE_ENCODE_H
#define VEND_RANGE_ENCODE_H

#include "encode/single/encode_bitset.h"
#include "algorithm"
#include "util/utils.h"
class RangeEncode : public EncodeBitSet {
public:
    RangeEncode() : max_int_size_(K_SIZE - 1), v_bits_size_(32), EncodeBitSet() {};


    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        PairType type1=NonNeighborTest(vertex1, vertex2);
        PairType type2=NonNeighborTest(vertex2, vertex1);

        if(type1<type2)
            return type1;
        else
            return type2;
    }

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override;


    std::vector<uint32_t> GetBlockInt(uint32_t vertex);

    // return true if fully encode
    virtual bool GetFlag(uint32_t vertex) {
        return encode_bitset_[vertex]->BlockGet(0, v_bits_size_);
    };

    // flag == true means fully encode
    virtual void SetFlag(uint32_t vertex, bool flag) {
        encode_bitset_[vertex]->BlockSet(0, v_bits_size_, flag);
    };

    std::vector<uint32_t> GetAllNeighbors(uint32_t vertex);

    EncodeType GetEncodeType(uint32_t vertex) {
        if (!GetFlag(vertex))
            return EncodeType::NonDecodable;
        if (encode_bitset_[vertex]->BlockGet(max_int_size_ * v_bits_size_, v_bits_size_) == 0)
            return EncodeType::UnFull;
        return EncodeType::Full;
    }

    // return false if vertex2 is not a neighbor
    bool RemoveNeighbor(uint32_t vertex1, uint32_t vertex2);

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override;

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override;

    void DeletePair(uint32_t vertex1, uint32_t vertex2) override;

    uint32_t GetIntSize() { return max_int_size_; }

protected:
    uint32_t max_int_size_;
    uint32_t v_bits_size_;
    uint32_t size_=0;
};


#endif //VEND_RANGE_ENCODE11_H
