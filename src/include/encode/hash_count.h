
#ifndef VEND_HASH_COUNT_H
#define VEND_HASH_COUNT_H


#include <vector>
#include <cstring>
#include "common/hash_config.h"
#include "common/config.h"

enum BlockType : char {
    FullHash = 1, LeftMost = 2, RightMost = 4, Middle = 8
};

//a class that  dynamic choose for hybrid encode
class HashCount {
public:
    HashCount() {
        hash_array_ = new uint32_t[PER_ENCODE_BIT_SIZE];
        for(int i=0;i<PER_ENCODE_BIT_SIZE;++i){
            hash_array_[i]=0;
        }

//        pre_nepair_ = new uint32_t[PER_ENCODE_BIT_SIZE];
//        memset(pre_nepair_, 0, sizeof(uint32_t) * PER_ENCODE_BIT_SIZE);
    }
    ~HashCount(){
        delete[] hash_array_;
    }
    HashCount(uint32_t block_size) {
        hash_array_ = new uint32_t[block_size];
        for(int i=0;i<block_size;++i){
            hash_array_[i]=0;
        }
        hash_size_ = block_size;
        hash_count_ = 0;
    }

    HashCount(const std::vector<uint32_t> &neighbors, uint32_t block_size) {
        ReConstruct(neighbors, block_size);
    }

    inline uint32_t Hash(uint32_t key) {
        if(hash_size_==0)
            return 0;
        else
            return key % hash_size_;
    }

    void SetHashSize(uint32_t hash_size) {
        hash_size_ = hash_size;
    }

    void RemoveHash(uint32_t key);

    void AddHash(uint32_t key);

    // when block size +1, call the function to reconstruct pre_nepair and hash_array
    void ReConstruct(const std::vector<uint32_t> &neighbors, uint32_t block_size, bool reverse = false);

    uint32_t GetScore(const std::vector<uint32_t> &neighbors, uint32_t block_size, uint32_t first_idx,
                      const BlockType &type);

    // for delta simd version
    void Construct(const std::vector<uint32_t> &neighbors) {
        for (size_t i = 0; i < neighbors.size(); ++i) {
            uint32_t val = Hash(neighbors[i]);
            if (hash_array_[val] == 0)
                hash_count_ += 1;
            hash_array_[val] += 1;
        }
    }

    uint32_t GetVal(const std::vector<uint32_t> &neighbors, int first_idx, int block_size, const BlockType &type) {
        uint32_t *hash_copy = new uint32_t[hash_size_];
        uint32_t first_neighbor, last_neighbor, score = 0;

        memcpy(hash_copy, hash_array_, sizeof(uint32_t) * hash_size_);
        int zero_count = hash_size_ - hash_count_;
        for (int i = first_idx; i < first_idx + block_size; ++i) {
            uint32_t val = Hash(neighbors[i]);
            if (hash_copy[val] == 1)
                zero_count += 1;
            hash_copy[val] -= 1;

        }
        delete hash_copy;

        switch (type) {
            case BlockType::FullHash:
                if (hash_size_ == 0)
                    return 0;
                return VERTEX_SIZE / hash_size_ * (zero_count);
            case BlockType::LeftMost:
                first_neighbor = 0;
                last_neighbor = neighbors[first_idx + block_size - 1];
                break;
            case BlockType::RightMost:
                first_neighbor = neighbors[first_idx];
                last_neighbor = VERTEX_SIZE;
                break;
            case BlockType::Middle:
                first_neighbor = neighbors[first_idx];
                last_neighbor = neighbors[first_idx + block_size - 1];
                break;
        }
        score += last_neighbor - first_neighbor + 1;
        if (hash_size_ != 0 )
            score += (VERTEX_SIZE - last_neighbor + first_neighbor) / hash_size_ * zero_count;

        return score;
    }


protected:
    uint32_t *hash_array_;
    // pre_nepair[i] : from 0~i, how many 0 bits
//    uint32_t *pre_nepair_;
    uint32_t hash_size_;
    uint32_t hash_count_;
    uint32_t neighbor_size_;


};


#endif //VEND_HASH_COUNT_H
