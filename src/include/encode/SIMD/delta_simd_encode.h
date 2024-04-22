#ifndef VEND_DELTA_SIMD_ENCODE_H
#define VEND_DELTA_SIMD_ENCODE_H

#include <unordered_set>

#include "encode/codec/SIMD/delta_simd_codec.h"
#include "encode/hash_count.h"
#include "encode/single/encode_bitset.h"
#include "simd_encode.h"

/**
 *  version one:
 *      fully encode by delta
 * */
class DeltaSIMDEncode : public SIMDEncode {
   public:
    DeltaSIMDEncode() : SIMDEncode() {
        max_bytes_len_ = PER_ENCODE_BIT_SIZE / 8 - 1;
        per_encode_bits_num_ = PER_ENCODE_BIT_SIZE;
        this->codec_ = new DeltaSIMDCodec();
    }

    DeltaSIMDEncode(size_t bit_size, size_t vertex_size) : SIMDEncode(bit_size, vertex_size) {
        assert(bit_size % 32 == 0);
        max_bytes_len_ = bit_size / 8 - 1;
        per_encode_bits_num_ = bit_size;
        this->codec_ = new DeltaSIMDCodec();
    }

    ~DeltaSIMDEncode() { delete codec_; }

    DeltaSIMDCodec *GetCodec() { return this->codec_; }

    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
#if UNDIRECTED == 1
        if (IsDecodable(vertex1)) {
            if (IsDecodable(vertex2))
                return std::min(FindDecodable(encode_bitset_[vertex1], vertex2),
                                FindDecodable(encode_bitset_[vertex2], vertex1));
            else
                return FindDecodable(encode_bitset_[vertex1], vertex2);
        } else {
            if (IsDecodable(vertex2))
                return FindDecodable(encode_bitset_[vertex2], vertex1);
            else
                return std::min(FindNonDecodable(encode_bitset_[vertex1], vertex2),
                                FindNonDecodable(encode_bitset_[vertex2], vertex1));
        }
#endif

#if UNDIRECTED == 0
        if (IsDecodable(vertex1)) {
            return FindDecodable(encode_bitset_[vertex1], vertex2);
        } else {
            return FindNonDecodable(encode_bitset_[vertex1], vertex2);
        }
#endif
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        BitSetSIMD *encode = encode_bitset_[vertex_id];
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

    void InsertPair(uint32_t vertex1, uint32_t vertex2) override {}

    // decode functions
    virtual PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) {
        if (IsDecodable(vertex1)) {
            return FindDecodable(encode_bitset_[vertex1], vertex2);
        } else {
            return FindNonDecodable(encode_bitset_[vertex1], vertex2);
        }
    }

   protected:
    PairType FindDecodable(BitSetSIMD *encode, uint32_t vertex) {
        uint32_t size = encode->GetEncodeSize();
        if (size == 0) return PairType::NonNeighbor;
        Codec::FindRes res = codec_->FindWithCount(encode->GetEncodePointer(), size, vertex);
        return !res ? PairType::Neighbor : PairType::NonNeighbor;
    }

    PairType FindNonDecodable(BitSetSIMD *encode, uint32_t vertex) {
        uint32_t in_size = encode->GetEncodeSize();
        if (in_size == 0) {
            return encode->IsOne(Hash(vertex, 8)) ? PairType::Uncertain : PairType::NonNeighbor;
        }
        uint8_t *in = encode->GetEncodePointer();
        BlockType type = GetBlockModel(encode);

        Codec::FindRes res = codec_->FindWithCount(in, in_size, vertex);
        if (!res)
            return PairType::Neighbor;
        else if ((res & type)) {
            return PairType::NonNeighbor;
        } else {
            uint32_t hash_begin = (1 + codec_->GetEncodeLength(in, in_size)) * 8;
            if (hash_begin == per_encode_bits_num_ || encode->IsOne(Hash(vertex, hash_begin)))
                return PairType::Uncertain;
            else
                return PairType::NonNeighbor;
        }
    }

    struct BlockInfo {
        int begin;
        // how many integer to write
        int block_size;
        BlockType block_type;
    };

    // encode functions
    virtual void DynamicChoose(BitSetSIMD *encode, std::vector<uint32_t> &neighbors,
                               std::vector<DeltaInfo> &delta_infos) {
        uint32_t neighbor_size = neighbors.size(), score = 0;
        bool oversize = false;
        BlockInfo best_block, current_block{0, 0, BlockType::FullHash};
        // full_hash
        best_block = current_block;
        FullHashScoreCount(neighbors, max_bytes_len_ * 8, score);

        for (int i = 0; i < neighbor_size && !oversize; ++i) {
            // left most
            uint32_t pre_bytes = delta_infos[i].val_bits;
            current_block.block_size = 1;
            current_block.begin = i;
            if (i == 0) {
                current_block.block_type = BlockType::LeftMost;
            } else if (i != neighbor_size - 1) {
                current_block.block_size += 1;
                pre_bytes += delta_infos[i + 1].delta_bits;
                current_block.block_type = BlockType::Middle;
            } else {
                current_block.block_type = BlockType::RightMost;
            }
            // get score by add block size  iteratively
            while (pre_bytes + (current_block.block_size + 3) / 4 <= max_bytes_len_) {
                if (current_block.block_size + i == neighbor_size) current_block.block_type = BlockType::RightMost;
                if (ScoreCount(current_block, neighbors,
                               (max_bytes_len_ - pre_bytes - (current_block.block_size + 3) / 4) * 8, score)) {
                    best_block = current_block;
                }
                if (current_block.block_type == BlockType::RightMost) break;
                current_block.block_size++;
                pre_bytes += delta_infos[current_block.block_size + i - 1].delta_bits;
            }
        }
        ConstructEncode(best_block, neighbors, encode);
    };

    bool FullHashScoreCount(std::vector<uint32_t> &neighbors, uint32_t hash_bits, uint32_t &score) {
        std::vector<bool> hash_val(hash_bits, false);
        uint32_t hash_count = 0;
        uint32_t max_hash_count = hash_bits - score * hash_bits / VERTEX_SIZE;
        // hash part
        for (int i = 0; i < neighbors.size() && hash_count <= max_hash_count; ++i) {
            uint32_t hash_now = neighbors[i] % hash_bits;
            if (!hash_val[hash_now]) {
                ++hash_count;
                hash_val[hash_now] = true;
            }
        }
        if (hash_count <= max_hash_count) {
            score = VERTEX_SIZE * (hash_bits - hash_count) / hash_bits;
            return true;
        }
        return false;
    }

    bool ScoreCount(BlockInfo &block_info, std::vector<uint32_t> &neighbors, uint32_t hash_bits, uint32_t &score) {
        uint32_t current_score, hash_count = 0;
        uint32_t first_idx = block_info.begin, last_idx = block_info.begin + block_info.block_size - 1;
        if (block_info.block_type == BlockType::RightMost) {
            current_score = VERTEX_SIZE - neighbors[first_idx] + 1;
        } else if (block_info.block_type == BlockType::LeftMost) {
            current_score = neighbors[last_idx];
        } else {
            current_score = neighbors[last_idx] - neighbors[first_idx] + 1;
        }

        if (hash_bits == 0) {
            if (current_score > score) {
                score = current_score;
                return true;
            }
            return false;
        }

        uint32_t max_hash_count = current_score >= score
                                      ? hash_bits
                                      : hash_bits - (score - current_score) * hash_bits / (VERTEX_SIZE - current_score);
        if ((neighbors.size() - block_info.block_size) * 0.35 > max_hash_count) return false;
        // count hash part
        // TODO count score &&  fix bug : hash_bits==0
        std::vector<bool> hash_val(hash_bits, false);
        for (int i = 0; i < neighbors.size() && hash_count <= max_hash_count; ++i) {
            if (i >= first_idx && i <= last_idx) continue;
            uint32_t hash_now = neighbors[i] % hash_bits;
            if (!hash_val[hash_now]) {
                ++hash_count;
                hash_val[hash_now] = true;
            }
        }
        // update score
        if (hash_count <= max_hash_count) {
            score = current_score + (VERTEX_SIZE - current_score) * (hash_bits - hash_count) / hash_bits;
            return true;
        }
        return false;
    };

    void ConstructEncode(const BlockInfo &block_info, std::vector<uint32_t> &neighbors, BitSetSIMD *encode) {
        uint8_t *in = encode->GetEncodePointer();
        uint32_t out_bytes_length = 0;
        int last_idx = block_info.begin + block_info.block_size - 1;

        SetBlockModel(encode, block_info.block_type);
        if (block_info.block_type != BlockType::FullHash) {
            SetEncodeSize(encode, block_info.block_size);
            codec_->EncodeArray(neighbors.data() + block_info.begin, block_info.block_size, in, out_bytes_length);
        }
        // hash part
        uint32_t hash_begin = (out_bytes_length + 1) * 8;
        if (hash_begin == per_encode_bits_num_) return;
        for (int i = 0; i < neighbors.size(); ++i) {
            if (i >= block_info.begin && i <= last_idx) continue;
            encode->SetOne(Hash(neighbors[i], hash_begin));
        }
    }

    // TODO  avoid duplicate code
    inline BlockType GetBlockModel(uint32_t vertex) {
        if (encode_bitset_[vertex]->IsOne((uint32_t)1)) return BlockType::RightMost;
        if (!encode_bitset_[vertex]->IsOne((uint32_t)2)) return BlockType::LeftMost;
        return BlockType::Middle;
    };

    inline BlockType GetBlockModel(BitSetSIMD *&encode) {
        if (encode->GetSecondBit()) return BlockType::RightMost;
        if (encode->GetThridBit()) return BlockType::Middle;
        return BlockType::LeftMost;
    };

    void SetBlockModel(BitSetSIMD *bitset, BlockType type) {
        switch (type) {
            case BlockType::LeftMost:
                bitset->BlockSet(1, 2, 0);
                break;
            case BlockType::FullHash:
            case BlockType::Middle:
                bitset->BlockSet(1, 2, 1);
                break;
            case BlockType::RightMost:
                bitset->BlockSet(1, 2, 3);
                break;
        }
    }

    inline void SetEncodeSize(BitSetSIMD *encode, uint32_t val) { encode->BlockSet(3, 5, val); }

    inline uint32_t GetEncodeSize(BitSetSIMD *encode) { return encode->BlockGet(3, 5); }

    inline uint32_t GetEncodeSize(uint32_t vertex) { return encode_bitset_[vertex]->BlockGet(3, 5); }

    inline bool IsDecodable(uint32_t vertex) { return encode_bitset_[vertex]->GetFirstBit(); }

    inline uint32_t Hash(uint32_t vertex_id, uint32_t hash_begin) {
        return vertex_id % (per_encode_bits_num_ - hash_begin) + hash_begin;
    };

    uint32_t max_bytes_len_;
    uint32_t per_encode_bits_num_;

   protected:
    DeltaSIMDCodec *codec_;
};

/**
 *  X1.... X5....X9
 */
class DeltaSIMDEncodeV1 : public DeltaSIMDEncode {
   public:
    DeltaSIMDEncodeV1() : DeltaSIMDEncode() {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV1();
    }

    DeltaSIMDEncodeV1(size_t bit_size, size_t vertex_size) : DeltaSIMDEncode(bit_size, vertex_size) {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV1();
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        BitSetSIMD *encode = encode_bitset_[vertex_id];
        if (neighbors.empty()) {
            encode->SetOne(0);
            return;
        }
        std::vector<DeltaInfo> delta_infos;
        codec_->ParseEncode(neighbors.data(), neighbors.size(), delta_infos);
        uint32_t encode_byte_len = codec_->GetRequiredBytes(neighbors);
        if (encode_byte_len <= max_bytes_len_) {
            encode->SetOne(0);
            SetEncodeSize(encode, neighbors.size());
            codec_->EncodeArray(neighbors.data(), neighbors.size(), encode->GetEncodePointer(), encode_byte_len);
        } else {
            // encode partial neigbors
            DynamicChoose(encode, neighbors, delta_infos);
        }
    }

   protected:
    void DynamicChoose(BitSetSIMD *encode, std::vector<uint32_t> &neighbors,
                       std::vector<DeltaInfo> &delta_infos) override {
        // TODO remove delta_info (unuseful)
        uint32_t neighbor_size = neighbors.size(), score = 0;
        bool oversize = false;
        BlockInfo current_block{0, 0, BlockType::FullHash};

        // full_hash
        BlockInfo best_block = current_block;
        FullHashScoreCount(neighbors, max_bytes_len_ * 8, score);

        for (int i = 0; i < neighbor_size && !oversize; ++i) {
            // left most
            int terms = 0;
            uint32_t pre_val = neighbors[i];
            uint32_t pre_bytes = delta_infos[i].val_bits;
            current_block.block_size = 1;
            current_block.begin = i;
            if (i == 0) {
                current_block.block_type = BlockType::LeftMost;
            } else if (i != neighbor_size - 1) {
                current_block.block_size += 1;
                pre_bytes += codec_->GetOneEncodeLength(neighbors[i + 1] - pre_val);
                current_block.block_type = BlockType::Middle;
            } else {
                current_block.block_type = BlockType::RightMost;
            }
            // get score by add block size  iteratively
            while (pre_bytes + (current_block.block_size + 3) / 4 <= max_bytes_len_) {
                if (current_block.block_size + i == neighbor_size) current_block.block_type = BlockType::RightMost;
                if (ScoreCount(current_block, neighbors,
                               (max_bytes_len_ - pre_bytes - (current_block.block_size + 3) / 4) * 8, score)) {
                    best_block = current_block;
                }
                if (current_block.block_type == BlockType::RightMost) break;

                current_block.block_size++;

                // X1,X2-X1,X3-X1,X4-X1  X5,X6-X5,X7-X5,X8-X5
                if (current_block.block_size % 4 == 1) {
                    pre_val = neighbors[current_block.block_size + i - 1];
                    pre_bytes += delta_infos[current_block.block_size + i - 1].val_bits;
                } else {
                    pre_bytes += codec_->GetOneEncodeLength(neighbors[current_block.block_size + i - 1] - pre_val);
                }
            }
        }

        ConstructEncode(best_block, neighbors, encode);
    }
};

class DeltaSIMDEncodeV2 : public DeltaSIMDEncode {
   public:
    DeltaSIMDEncodeV2() : DeltaSIMDEncode() {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV2();
    }

    DeltaSIMDEncodeV2(size_t bit_size, size_t vertex_size) : DeltaSIMDEncode(bit_size, vertex_size) {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV2();
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        BitSetSIMD *encode = encode_bitset_[vertex_id];
        if (neighbors.empty()) {
            encode->SetOne(0);
            return;
        }
        std::vector<DeltaInfo> delta_infos;
        codec_->ParseEncode(neighbors.data(), neighbors.size(), delta_infos);
        uint32_t encode_byte_len = codec_->GetRequiredBytes(neighbors);
        if (encode_byte_len <= max_bytes_len_) {
            encode->SetOne(0);
            SetEncodeSize(encode, neighbors.size());
            codec_->EncodeArray(neighbors.data(), neighbors.size(), encode->GetEncodePointer(), encode_byte_len);
        } else {
            // encode partial neigbors
            DynamicChoose(encode, neighbors, delta_infos);
        }
    }

   protected:
    void DynamicChoose(BitSetSIMD *encode, std::vector<uint32_t> &neighbors,
                       std::vector<DeltaInfo> &delta_infos) override {
        uint32_t neighbor_size = neighbors.size(), score = 0;
        bool oversize = false;
        BlockInfo current_block{0, 0, BlockType::FullHash};
        // full_hash
        BlockInfo best_block = current_block;
        FullHashScoreCount(neighbors, max_bytes_len_ * 8, score);

        for (int i = 0; i < neighbor_size && !oversize; ++i) {
            // left most
            uint32_t pre_bytes = delta_infos[i].val_bits;
            current_block.block_size = 1;
            current_block.begin = i;
            if (i == 0) {
                current_block.block_type = BlockType::LeftMost;
            } else if (i != neighbor_size - 1) {
                current_block.block_size += 1;
                pre_bytes += delta_infos[i + 1].delta_bits;
                current_block.block_type = BlockType::Middle;
            } else {
                current_block.block_type = BlockType::RightMost;
            }
            // get score by add block size  iteratively
            while (pre_bytes + (current_block.block_size + 3) / 4 <= max_bytes_len_) {
                if (current_block.block_size + i == neighbor_size) current_block.block_type = BlockType::RightMost;
                if (ScoreCount(current_block, neighbors,
                               (max_bytes_len_ - pre_bytes - (current_block.block_size + 3) / 4) * 8, score)) {
                    best_block = current_block;
                }
                if (current_block.block_type == BlockType::RightMost) break;

                current_block.block_size++;

                // X1,d2,d3,d4,X5,d6,d7,d8
                if (current_block.block_size % 4 == 1) {
                    pre_bytes += delta_infos[current_block.block_size + i - 1].val_bits;
                } else {
                    pre_bytes += delta_infos[current_block.block_size + i - 1].delta_bits;
                }
            }
        }
        ConstructEncode(best_block, neighbors, encode);
    }
};

class DeltaSIMDEncodeV3 : public DeltaSIMDEncodeV2 {
   public:
    DeltaSIMDEncodeV3() : DeltaSIMDEncodeV2() {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV3();
    }

    DeltaSIMDEncodeV3(size_t bit_size, size_t vertex_size) : DeltaSIMDEncodeV2(bit_size, vertex_size) {
        delete codec_;
        this->codec_ = new DeltaSIMDCodecV3();
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        BitSetSIMD *encode = encode_bitset_[vertex_id];
        if (neighbors.empty()) {
            encode->SetOne(0);
        }
        std::vector<DeltaInfo> delta_infos;

        uint32_t encode_byte_len = codec_->GetRequiredBytes(neighbors);
        SetEncodeSize(encode, encode_byte_len);

        if (encode_byte_len <= max_bytes_len_) {
            encode->SetOne(0);
            SetEncodeSize(encode, neighbors.size());
            codec_->EncodeArray(neighbors.data(), neighbors.size(), encode->GetEncodePointer(), encode_byte_len);
        } else {
            // encode partial neigbors

            DynamicChoose(encode, neighbors, delta_infos);
        }
    }

    /**
     *  insert function
     *
     * */
    void InsertPair(uint32_t vertex1, uint32_t vertex2) override {
        if (NEpairTest(vertex1, vertex2) == PairType::Uncertain) return;
        bool type1 = IsDecodable(vertex1);
        bool type2 = IsDecodable(vertex2);
        if (type1 < type2) {
            std::swap(vertex1, vertex2);
            std::swap(type1, type2);
        }
        std::vector<uint32_t> neighbor1 = GetAllNeighbors(vertex1);
        neighbor1.push_back(vertex2);

        if (type1) {
            EncodeVertex(vertex1, neighbor1);
        } else {
            std::vector<uint32_t> neighbor2 = GetAllNeighbors(vertex2);
            neighbor2.push_back(vertex1);
            EncodeVertex(vertex2, neighbor2);
        }
    };

    void DeletePair(uint32_t vertex1, uint32_t vertex2) override {
        if (NonNeighborTest(vertex1, vertex2) != PairType::NonNeighbor) RemoveNeighbor(vertex1, vertex2);
        if (NonNeighborTest(vertex2, vertex1) == PairType::NonNeighbor) RemoveNeighbor(vertex2, vertex1);
    };

   protected:
    void DynamicChoose(BitSetSIMD *encode, std::vector<uint32_t> &neighbors,
                       std::vector<DeltaInfo> &delta_infos) override {
        uint32_t neighbor_size = neighbors.size(), score = 0;
        bool oversize = false;
        BlockInfo current_block{0, 0, BlockType::FullHash};
        // full_hash
        BlockInfo best_block = current_block;

#if THRESHOLD
        if (neighbors.size() <= threshold) {
#endif
            FullHashScoreCount(neighbors, max_bytes_len_ * 8, score);
            std::vector<HashCount *> slot_array(PER_ENCODE_BIT_SIZE, nullptr);
            std::vector<bool> slot_flag(PER_ENCODE_BIT_SIZE, false);
            for (int i = 0; i < neighbor_size && !oversize; ++i) {
                // left most
                current_block.block_size = 1;
                current_block.begin = i;
                if (i == 0) {
                    current_block.block_type = BlockType::LeftMost;
                } else if (i != neighbor_size - 1) {
                    current_block.block_size += 1;
                    current_block.block_type = BlockType::Middle;
                } else {
                    current_block.block_type = BlockType::RightMost;
                }
                // get score by add block size  iteratively
                while (true) {
                    uint32_t byte_len =
                        codec_->GetRequiredBytes(neighbors, current_block.begin, current_block.block_size) +
                        (current_block.block_size + 3) / 4;
                    if (byte_len > max_bytes_len_) break;
                    if (current_block.block_size + i == neighbor_size) current_block.block_type = BlockType::RightMost;

                    int slot_size = (max_bytes_len_ - byte_len) * 8;
                    if (slot_array[slot_size] == nullptr) {
                        HashCount *temp = new HashCount(slot_size);
                        temp->Construct(neighbors);
                        slot_array[slot_size] = temp;
                    }
                    uint32_t current_score = slot_array[slot_size]->GetVal(
                        neighbors, current_block.begin, current_block.block_size, current_block.block_type);
                    //                if (ScoreCount(current_block, neighbors,
                    //                               (max_bytes_len_ - byte_len) * 8, score)) {
                    if (current_score > score) {
                        score = current_score;
                        best_block = current_block;
                    }
                    if (current_block.block_type == BlockType::RightMost) break;
                    current_block.block_size++;
                }
            }
            for (auto &s : slot_array) {
                if (s != nullptr) delete s;
            }

#if THRESHOLD
        }
#endif
        ConstructEncode(best_block, neighbors, encode);
    }

   private:
    bool RemoveNeighbor(uint32_t vertex1, uint32_t vertex2) {
        std::vector<uint32_t> neighbors = GetAllNeighbors(vertex1);
        //    auto iter = std::find(neighbors.begin(), neighbors.end(), vertex2);
        //    if (iter == neighbors.end())
        //        return false;
        //    neighbors.erase(iter);
        auto iter = std::remove(neighbors.begin(), neighbors.end(), vertex2);
        neighbors.erase(iter, neighbors.end());
        EncodeVertex(vertex1, neighbors);
        return true;
    }

    std::vector<uint32_t> GetAllNeighbors(uint32_t vertex) {
        if (encode_bitset_[vertex]->IsOne((uint32_t)0)) return GetBlockInteger(vertex);
        return DbQuery(vertex);
    }

    std::vector<uint32_t> GetBlockInteger(uint32_t vertex) { return DbQuery(vertex); }
};


#endif  // VEND_DELTA_SIMD_ENCODE_H
