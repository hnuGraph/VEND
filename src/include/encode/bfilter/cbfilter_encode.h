
#ifndef VEND_CFBFILTER_ENCODE_H
#define VEND_CFBFILTER_ENCODE_H

#include "bfilter_bit_encode.h"

class CBFilterEncode : public BFilterBitEncode {

public:
    CBFilterEncode()
            : BFilterBitEncode(COUNT_HASH_BEST_NUMS), element_size_(ELEMENT_SIZE),
              element_nums_(((uint64_t) VERTEX_SIZE * K_SIZE * 32) / ELEMENT_SIZE) {}


    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override;

    void EdgeSet(uint32_t vertex1, uint32_t vertex2) override;

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override;

    uint64_t Hash(uint32_t key1, uint32_t key2, uint32_t hash_param1, uint32_t hash_param2) override {
        return (((uint64_t) key1 * hash_param1 + (uint64_t) key2 * hash_param2) % element_nums_)*element_size_;
    }

    bool IsDeletable(uint32_t vertex1, uint32_t vertex2) override;

    void DeletePair(uint32_t vertex1, uint32_t vertex2) override;

protected:
    // per bits for counting
    uint32_t element_size_;
    // how many elements for the whole bitset
    uint64_t element_nums_;

};

#endif //VEND_CFBFILTER_ENCODE_H
