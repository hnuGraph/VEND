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
#include "assert.h"

uint32_t HashCount::RemoveHash(uint32_t key) {
    uint32_t hash_val = Hash(key);
    if (hash_array_[hash_val] == 1) {
        for (uint32_t i = hash_val; i < hash_size_; ++i)
            pre_nepair_[i] += 1;
    }
    if (hash_array_[hash_val] > 0)
        hash_array_[hash_val] -= 1;

}


uint32_t HashCount::AddHash(uint32_t key) {
    uint32_t hash_val = Hash(key);

    if (hash_array_[hash_val] == 0) {
        for (uint32_t i = hash_val; i < hash_size_; ++i)
            pre_nepair_[i] -= 1;
    }
    hash_array_[hash_val] += 1;
}

void HashCount::ReConstruct(const std::vector<uint32_t> &neighbors, uint32_t block_size, bool reverse) {
    memset(hash_array_, 0, sizeof(uint32_t) * PER_ENCODE_BIT_SIZE);
    memset(pre_nepair_, 0, sizeof(uint32_t) * PER_ENCODE_BIT_SIZE);
    hash_size_ = PER_ENCODE_BIT_SIZE - PREFIX_BIT_SIZE - block_size * VERTEX_BIT_SIZE;
    neighbor_size_ = neighbors.size();
    if (reverse) {
        for (int i = 0; i < neighbor_size_ - block_size - 1; ++i) {
            hash_array_[Hash(neighbors[i])] += 1;
        }

    } else {
        for (int i = block_size; i < neighbor_size_; ++i) {
            hash_array_[Hash(neighbors[i])] += 1;
        }
    }
    pre_nepair_[0] = !hash_array_[0];
    for (int i = 1; i < hash_size_; ++i)
        pre_nepair_[i] = pre_nepair_[i - 1] + !hash_array_[i];
}

uint32_t
HashCount::GetScore(const std::vector<uint32_t> &neighbors, uint32_t block_size, uint32_t first_idx,
                    const BlockType &type) {
    uint32_t score = 0, hash_nepairs = 0, begin_idx = 0, end_idx = 0, first_neighbor = 0, last_neighbor = 0, cut_point1 = 0, cut_point2 = 0, cut_point3 = 0;

    if (hash_size_ != 0) {
        hash_nepairs = pre_nepair_[hash_size_ - 1];
        cut_point3 = VERTEX_SIZE / hash_size_;
    }
    switch (type) {
        case BlockType::FullHash:
            return cut_point3 * hash_nepairs + pre_nepair_[Hash(VERTEX_SIZE)];
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

    if (hash_size_ != 0) {
        cut_point1 = first_neighbor / hash_size_;
        cut_point2 = last_neighbor / hash_size_;
    }

    score += last_neighbor - first_neighbor - block_size + 1;
    score += (cut_point1 + cut_point3 - cut_point2) * hash_nepairs;
    score += pre_nepair_[Hash(first_neighbor)] - !hash_array_[Hash(first_neighbor)];

    if (type != BlockType::RightMost) {
        score += pre_nepair_[Hash(VERTEX_SIZE)] - pre_nepair_[Hash(last_neighbor)] + !hash_array_[Hash(last_neighbor)];
    }

    return score;
}
