//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_BLOOM_FILTER_ENCODE_H
#define VEND_BLOOM_FILTER_ENCODE_H

#include "util/bitset.h"
#include "common/hash_config.h"
#include "encode/encode.h"
#include <fstream>
#include "math.h"


class BFilterEncode : public Encode {
public:
    BFilterEncode(uint32_t best_hash_nums, uint64_t hash_size) : best_hash_nums_(best_hash_nums), hash_size_(hash_size),
                                                                 hash_param1_(HASH_PARAM1), hash_param2_(HASH_PARAM2),
                                                                 Encode() {}


    /**
     *  hash function for bloom filter
     *  @param key : input key
     *          hash param
     *  @return :  return (key*hash_param1+key2*hash_param2)%|bitset|
     * */
    virtual uint64_t Hash(uint32_t key1, uint32_t key2, uint32_t hash_param1, uint32_t hash_param2) {
        return ((uint64_t)key1 * hash_param1 + (uint64_t)key2 * hash_param2) % hash_size_;
    };

protected:
    uint32_t best_hash_nums_;
    std::vector<uint32_t> hash_param1_;
    std::vector<uint32_t> hash_param2_;
    uint64_t hash_size_;
};

#endif //VEND_BLOOM_FILTER_ENCODE_H
