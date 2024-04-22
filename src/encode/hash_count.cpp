//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#include "encode/hash_count.h"

void HashCount::RemoveHash(uint32_t key) {
    uint32_t hash_val = Hash(key);
    //    if (hash_array_[hash_val] == 1) {
    //        for (uint32_t i = hash_val; i < hash_size_; ++i)
    //            pre_nepair_[i] += 1;
    //    }
    if (hash_array_[hash_val] > 0) {
        hash_array_[hash_val] -= 1;
        if (hash_array_[hash_val] == 0) hash_count_ -= 1;
    }
}

void HashCount::AddHash(uint32_t key) {
    uint32_t  hash_val = Hash(key);

    //    if (hash_array_[hash_val] == 0) {
    //        for (uint32_t i = hash_val; i < hash_size_; ++i)
    //            pre_nepair_[i] -= 1;
    //    }
    if (hash_array_[hash_val] == 0) hash_count_ += 1;
    hash_array_[hash_val] += 1;
}

void HashCount::ReConstruct(const std::vector<uint32_t> &neighbors, uint32_t block_size, bool reverse) {
    for (int i = 0; i < PER_ENCODE_BIT_SIZE; ++i) {
        hash_array_[i] = 0;
    }
    hash_count_ = 0;
    //    memset(pre_nepair_, 0, sizeof(uint32_t) * PER_ENCODE_BIT_SIZE);
    hash_size_ = PER_ENCODE_BIT_SIZE - PREFIX_BIT_SIZE - block_size * VERTEX_BIT_SIZE;
    neighbor_size_ = neighbors.size();
    if (reverse) {
        for (int i = 0; i < neighbor_size_ - block_size - 1; ++i) {
            uint32_t val = Hash(neighbors[i]);
            if (hash_array_[val] == 0) hash_count_ += 1;
            hash_array_[val] += 1;
        }

    } else {
        for (int i = block_size; i < neighbor_size_; ++i) {
            uint32_t val = Hash(neighbors[i]);
            if (hash_array_[val] == 0) hash_count_ += 1;
            hash_array_[val] += 1;
        }
    }
    //    pre_nepair_[0] = !hash_array_[0];
    //    for (int i = 1; i < hash_size_; ++i)
    //        pre_nepair_[i] = pre_nepair_[i - 1] + !hash_array_[i];
}

uint32_t HashCount::GetScore(const std::vector<uint32_t> &neighbors, uint32_t block_size, uint32_t first_idx,
                             const BlockType &type) {
    uint32_t score = 0, hash_nepairs = 0, begin_idx = 0, end_idx = 0, first_neighbor = 0, last_neighbor = 0,
             cut_point1 = 0, cut_point2 = 0, cut_point3 = 0;

    switch (type) {
        case BlockType::FullHash:
            return VERTEX_SIZE / hash_size_ * (hash_size_ - hash_count_);
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

    //    if (hash_size_ != 0) {
    //        cut_point1 = first_neighbor / hash_size_;
    //        cut_point2 = last_neighbor / hash_size_;
    //    }

    score += last_neighbor - first_neighbor + 1;
    score += hash_size_==0 ? 0: (VERTEX_SIZE - last_neighbor + first_neighbor) / hash_size_ * (hash_size_ - hash_count_);

    return score;
}
