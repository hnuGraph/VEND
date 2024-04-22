//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_HYBRID_H_
#define SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_HYBRID_H_

#include <cstring>
#include <fstream>
#include <set>
#include <sstream>

#include "algorithm"
#include "assert.h"
#include "dbengine/dbengine.h"
#include "encode/hash_count.h"
#include "encode/single/encode_bitset.h"
#include "encode/single/hybrid_encode.h"
#include "math.h"
#include "util/bitset.h"
#include "util/utils.h"

class DirectedHybrid : public HybridEncode {
   public:
    DirectedHybrid() : HybridEncode() {
        this->in_bitset_ = new BitSetSIMD *[VERTEX_SIZE + 1];
        for (uint32_t i = 0; i < VERTEX_SIZE + 1; ++i) {
            in_bitset_[i] = new BitSetSIMD(PER_ENCODE_BIT_SIZE);
        }
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &out_neigh, std::vector<uint32_t> &in_neigh) override {
        EncodeVertex(vertex_id, out_neigh, true);
        EncodeVertex(vertex_id, in_neigh, false);
    };

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors, bool out) {
        uint32_t score;
        BitSetSIMD *encode;
        if (neighbors.size() <= MAX_K_CORE_SIZE) {
            if (out) {
                encode = encode_bitset_[vertex_id];
            } else {
                encode = in_bitset_[vertex_id];
            }
            encode->SetOne((uint32_t)0);
            uint32_t index = 1;
            for (auto iter = neighbors.begin(); iter != neighbors.end(); ++iter, index += v_bits_size_)
                encode->BlockSet(index, v_bits_size_, *iter);
        } else {
            DynamicChoose(vertex_id, neighbors, score, out);
        }
    }

    void DynamicChoose(uint32_t vertex, const std::vector<uint32_t> &neighbors, uint32_t &count, bool out) {
        uint32_t max_score = 0, neighbor_size = neighbors.size();

        BitSetSIMD *bitset;
        if (out)
            bitset = encode_bitset_[vertex];
        else
            bitset = in_bitset_[vertex];
        HashCount *hash_count = new HashCount();
        // full hash
        hash_count->ReConstruct(neighbors, 0);
        ChooseHighest(neighbors, max_score, *bitset, hash_count, 0, 0, BlockType::FullHash);

        uint32_t block_size = 1;

#if THRESHOLD
        assert(threshold != 0);
        if (neighbors.size() >= threshold) block_size = MAX_INTEGER_SIZE;
#endif

        for (; block_size <= MAX_INTEGER_SIZE; ++block_size) {
            // left most
            hash_count->ReConstruct(neighbors, block_size);
            ChooseHighest(neighbors, max_score, *bitset, hash_count, block_size, 0, BlockType::LeftMost);
            if (block_size == 1) {
                hash_count->ReConstruct(neighbors, block_size, true);
                ChooseHighest(neighbors, max_score, *bitset, hash_count, block_size, neighbor_size - block_size,
                              BlockType::RightMost);
                continue;
            }
            // middle
            uint32_t first_idx;
            for (first_idx = 1; first_idx <= neighbor_size - block_size - 1; ++first_idx) {
                hash_count->AddHash(neighbors[first_idx - 1]);
                hash_count->RemoveHash(neighbors[first_idx + block_size - 1]);
                ChooseHighest(neighbors, max_score, *bitset, hash_count, block_size, first_idx, BlockType::Middle);
            }
            // right most
            ChooseHighest(neighbors, max_score, *bitset, hash_count, block_size, first_idx, BlockType::RightMost);
        }

        delete hash_count;
        count = max_score;
    }

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        bool encode_type1 = IsDecodable(vertex1, true), encode_type2 = IsDecodable(vertex2, false);

        if (encode_type1) {
            if (encode_type2) {
                PairType type1 = NonNeighborTest(vertex1, vertex2, false);
                if (type1 == PairType::Neighbor)
                    return type1;
                else {
                    PairType type2 = NonNeighborTest(vertex2, vertex1, true);
                    return std::min(type1, type2);
                }
            } else {
                return NonNeighborTest(vertex1, vertex2, false);
            }
        } else {
            if (encode_type2) {
                return NonNeighborTest(vertex2, vertex1, true);
            } else {
                PairType type1 = NonNeighborTest(vertex1, vertex2, false);
                PairType type2 = NonNeighborTest(vertex2, vertex1, true);
                return std::min(type1, type2);
            }
        }
    };

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2, bool reverse) {
        auto encode = encode_bitset_[vertex1];
        if (reverse) {
            encode = in_bitset_[vertex1];
        }

        if (encode->IsOne((uint32_t)0)) {
            if (HSIMD == 0)
                return encode->BlockFind(1, v_bits_size_, MAX_K_CORE_SIZE, vertex2);
            else
                return encode->BlockFindSIMD(1, v_bits_size_, MAX_K_CORE_SIZE, vertex2);
        } else {
            uint32_t min = 0, max = vertex_id_upper_, block_num = encode->GetBlockNum();
            BlockType type = GetBlockModel(*encode);
            uint32_t hash_begin = block_num * v_bits_size_ + 3 + log_k_;
            // not left most
            if (type != BlockType::LeftMost) {
                min = block_num == 0 ? 0 : encode->BlockGet(3 + log_k_, v_bits_size_);
            }

            if (type != BlockType::RightMost) {
                max = block_num == 0 ? 0 : encode->BlockGet(3 + log_k_ + (block_num - 1) * v_bits_size_, v_bits_size_);
            }

            // search hash
            if (vertex2 < min || vertex2 > max) {
                if (hash_begin == PER_ENCODE_BIT_SIZE || encode->IsOne(Hash(vertex2, hash_begin)))
                    return PairType::Uncertain;
                else
                    return PairType::NonNeighbor;
            } else {
                if (HSIMD == 0)
                    return encode->BlockFind(BLOCK_BEGIN_INDEX, v_bits_size_, block_num, vertex2);
                else
                    return encode->BlockFindSIMD(BLOCK_BEGIN_INDEX, v_bits_size_, block_num, vertex2);
            }
        }
    }

    inline bool IsDecodable(uint32_t vertex, bool out) {
        if (out)
            return encode_bitset_[vertex]->IsOne((uint32_t)0);
        else
            return in_bitset_[vertex]->IsOne((uint32_t)0);
    }
};

#endif  // SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_HYBRID_H_
