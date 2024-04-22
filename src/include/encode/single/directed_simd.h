//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_SIMD_H_
#define SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_SIMD_H_

#include <cstring>
#include <fstream>
#include <set>
#include <sstream>

#include "algorithm"
#include "assert.h"
#include "dbengine/dbengine.h"
#include "encode/SIMD/delta_simd_encode.h"
#include "encode/hash_count.h"
#include "encode/single/encode_bitset.h"
#include "math.h"
#include "util/bitset.h"
#include "util/utils.h"

class DirectedSIMD : public DeltaSIMDEncode {
   public:
    DirectedSIMD() : DeltaSIMDEncode() {
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
        if (out) {
            encode = encode_bitset_[vertex_id];
        } else {
            encode = in_bitset_[vertex_id];
        }
        if (neighbors.empty()) {
            encode->SetOne(0);
            return;
        }
        std::vector<DeltaInfo> delta_infos;
        codec_->ParseEncode(neighbors.data(), neighbors.size(), delta_infos);
        uint32_t encode_byte_len = delta_infos.back().total_bits + (delta_infos.size() + 3) / 4;
        if (encode_byte_len <= max_bytes_len_) {
            encode->SetOne(0);
            SetEncodeSize(encode, neighbors.size());
            codec_->EncodeArray(neighbors.data(), neighbors.size(), encode->GetEncodePointer(), encode_byte_len);
        } else {
            // encode partial neigbors
            DynamicChoose(encode, neighbors, delta_infos);
        }
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
        BitSetSIMD *encode;
        if (reverse) {
            encode = in_bitset_[vertex1];
        } else {
            encode = encode_bitset_[vertex1];
        }
        if (IsDecodable(vertex1, reverse)) {
            return FindDecodable(encode, vertex2);
        } else {
            return FindNonDecodable(encode, vertex2);
        }
    }

    inline bool IsDecodable(uint32_t vertex, bool out) {
        if (out)
            return encode_bitset_[vertex]->IsOne((uint32_t)0);
        else
            return in_bitset_[vertex]->IsOne((uint32_t)0);
    }
};

#endif  // SRC_INCLUDE_ENCODE_SINGLE_DIRECTED_SIMD_H_
