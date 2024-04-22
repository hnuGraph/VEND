#ifndef SRC_INCLUDE_ENCODE_SINGLE_COMPRESSION_H_
#define SRC_INCLUDE_ENCODE_SINGLE_COMPRESSION_H_

#include <vector>

#include "common/config.h"
#include "util/bitset_simd.h"

enum CompMethod { BITS = 0, Vbyte = 1, Delta = 2 };

class Compression {
   public:
    Compression() {}
    Compression(uint32_t total_bits) : total_bits_(total_bits) {}

    virtual uint32_t GetRequiredBits(const std::vector<uint32_t> &list) = 0;

    virtual void Encode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) = 0;

    virtual void DynamicEncode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) = 0;

    virtual PairType Find(uint32_t value, BitSetSIMD *encode, uint32_t shift, bool not_partial) = 0;

   protected:
    uint32_t total_bits_;
};

class BitsComp : public Compression {
   public:
    BitsComp(uint32_t total_bits, uint32_t v_bit_size, uint32_t max_nums)
        : Compression(total_bits), v_bit_size_(v_bit_size), max_nums_(max_nums) {}

    uint32_t GetRequiredBits(const std::vector<uint32_t> &list) override { return list.size() * VERTEX_BIT_SIZE; }

    void Encode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) override {
        for (auto v : list) {
            encode->BlockSet(shift, v_bit_size_, v);
            shift += v_bit_size_;
        }
    }

    void DynamicEncode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) {
        uint32_t max_score = 0, score = 0;
        int idx = -1, size = list.size();
        for (int i = 0; i < size - max_nums_ + 1; ++i) {
            score = list[i + max_nums_ - 1] - list[i];
            if (max_score < score) {
                max_score = score;
                idx = i;
            }
        }
        // set min and max
        assert(max_nums_ > 2);
        encode->BlockSet(shift, v_bit_size_, list[idx]);
        encode->BlockSet(shift, v_bit_size_, list[idx + max_nums_ - 1]);

        for (int i = 1; i < max_nums_ - 1; ++i) {
            encode->BlockSet(shift, v_bit_size_, list[i + idx]);
            shift += v_bit_size_;
        }
    }

    PairType Find(uint32_t value, BitSetSIMD *encode, uint32_t shift, bool not_partial) {
        if (not_partial) {
            while (shift + v_bit_size_ < total_bits_) {
                uint32_t compare = encode->BlockGet(shift, v_bit_size_);
                if (value == compare)
                    return PairType::Neighbor;
                else if (value < compare)
                    return PairType::NonNeighbor;
                shift += v_bit_size_;
            }
            return PairType::NonNeighbor;
        } else {
            uint32_t min = encode->BlockGet(shift, v_bit_size_);
            shift += v_bit_size_;
            uint32_t max = encode->BlockGet(shift, v_bit_size_);
            shift += v_bit_size_;
            if (value == min || value == max) return PairType::Neighbor;
            if (value < min || value > max) return PairType::Uncertain;
            while (shift + v_bit_size_ < total_bits_) {
                uint32_t compare = encode->BlockGet(shift, v_bit_size_);
                if (value == compare)
                    return PairType::Neighbor;
                else if (value < compare)
                    return PairType::NonNeighbor;
                shift += v_bit_size_;
            }
            return PairType::NonNeighbor;
        }
    }

   private:
    uint32_t v_bit_size_;
    uint32_t max_nums_;
};

class VbyteComp : public Compression {
   public:
    VbyteComp(uint32_t total_bits) : Compression(total_bits) {}

    uint32_t GetRequiredBits(const std::vector<uint32_t> &list) override {
        uint32_t bytes = 0;
        for (auto v : list) {
            bytes += 8 * IntegerBytes(v);
        }
        return bytes;
    }

    void Encode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) override {
        std::vector<uint32_t> all_bits = GetAllBits(list);
        for (int i = 0; i < list.size(); ++i) {
            encode->BlockSet(shift, all_bits[i], compress(list[i]));
            shift += all_bits[i];
        }
    }

    void DynamicEncode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) {
        uint32_t max_score = 0, score = 0;
        uint32_t idx = -1, size = list.size(), pos = 0, best_block = 0, best_score = 0;
        std::vector<uint32_t> all_bits = GetAllBits(list);

        for (int i = 0; i < size - 1; ++i) {
            int block_size = 0, bits_num = 0;
            while (block_size < size - i) {
                bits_num += all_bits[i + block_size];
                if (bits_num + shift > total_bits_) break;
                ++block_size;
            }
            score = list[i + block_size] - list[i];
            if (score >= best_score) {
                best_block = block_size;
                idx = i;
                best_score = score;
            }
        }
        // set min and max
        assert(best_block >= 2);

        encode->BlockSet(shift, all_bits[idx], compress(list[idx]));
        shift += all_bits[idx];
        encode->BlockSet(shift, all_bits[idx + best_block - 1], compress(list[idx + best_block - 1]));
        shift += all_bits[idx + best_block - 1];
        for (int i = 1; i < best_block - 1; ++i) {
            encode->BlockSet(shift, all_bits[i + idx], compress(list[i + idx]));
            shift += all_bits[i + idx];
        }
    }

    PairType Find(uint32_t value, BitSetSIMD *encode, uint32_t shift, bool not_partial) {
        uint8_t *in = encode->GetOriginPointer();
        uint32_t len = shift / 8;
        in += len;
        if (not_partial) {
            while (len < total_bits_ / 8) {
                uint32_t next = decode(in, &len);
                if (value == next)
                    return PairType::Neighbor;
                else if (value < next)
                    return PairType::NonNeighbor;
            }
            return PairType::NonNeighbor;
        } else {
            uint32_t min = decode(in, &len);
            uint32_t max = decode(in, &len);
            if (value == min || value == max) return PairType::Neighbor;
            if (value < min || value > max) return PairType::Uncertain;

            while (len < total_bits_ / 8) {
                uint32_t next = decode(in, &len);
                if (value == next)
                    return PairType::Neighbor;
                else if (value < next)
                    return PairType::NonNeighbor;
            }
            return PairType::NonNeighbor;
        }
    }

   protected:
    inline uint32_t IntegerBytes(uint32_t value) {
        uint32_t idx = 1;
        for (; idx < 4; ++idx) {
            if (value <= range[idx - 1]) break;
        }
        return idx;
    }

    std::vector<uint32_t> GetAllBits(const std::vector<uint32_t> &list) {
        std::vector<uint32_t> res;
        for (auto v : list) {
            res.push_back(IntegerBytes(v) * 8);
        }
        return res;
    }

    uint32_t compress(uint32_t value) {
        uint32_t bytes = IntegerBytes(value);
        uint32_t res = 0;
        if (bytes == 1) {
            return value;
        } else if (bytes == 2) {
            res = ((value & 0x3F80) << 1) | 0x8000;
            res |= (value & 0x7F);

        } else if (bytes == 3) {
            res = ((value & 0x1FC000) << 2) | 0x808000;
            res |= ((value & 0x3F80) << 1);
            res |= (value & 0x7F);
        } else {
            res = ((value & 0xFE00000) << 3) | 0x80808000;
            res |= ((value & 0x1FC000) << 2);
            res |= ((value & 0x3F80) << 1);
            res |= (value & 0x7F);
        }
        return res;
    }

    virtual uint32_t decode(uint8_t *__restrict__ &ptr, uint32_t *len) {
        uint32_t res = 0;
        while (true) {
            (*len)++;
            res = (res << 7) | (*ptr & 0x7F);
            if (!(*ptr & 0x80)) {
                ptr++;
                break;
            }
            ptr++;
        }
        return res;
    }

    uint32_t range[3] = {127, 16383, 2097151};
};

class VbyteDeltaComp : public VbyteComp {
   public:
    VbyteDeltaComp(uint32_t total_bits) : VbyteComp(total_bits) {}
    uint32_t GetRequiredBits(const std::vector<uint32_t> &list) override {
        std::vector<uint32_t> deltas = GetDeltas(list);
        return VbyteComp::GetRequiredBits(deltas);
    }

    void Encode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) override {
        std::vector<uint32_t> deltas = GetDeltas(list);
        VbyteComp::Encode(deltas, encode, shift);
    }

    void DynamicEncode(const std::vector<uint32_t> &list, BitSetSIMD *encode, uint32_t shift) {
        uint32_t max_score = 0, score = 0;
        uint32_t idx = -1, size = list.size(), pos = 0, best_block = 0, best_score = 0;
        auto delta_bits = GetDeltaBits(list);
        auto all_bits = VbyteComp::GetAllBits(list);
        for (int i = 0; i < size - 3; ++i) {
            uint32_t bits_num = all_bits[i] + all_bits[i + 1];
            if (bits_num + shift > total_bits_) continue;
            score = list[i + 1] - list[i];
            if (score > best_score) {
                best_score = score;
                idx = i;
                best_block = 2;
            }
            for (int j = i + 2; j < size - 1; ++j) {
                score = list[j] - list[i];
                bits_num += all_bits[j] - all_bits[j - 1] + delta_bits[j - 2];
                if (bits_num + shift > total_bits_) break;
                if (score > best_score) {
                    best_score = score;
                    idx = i;
                    best_block = j - i + 1;
                }
            }
        }
        assert(best_block >= 2);
        encode->BlockSet(shift, all_bits[idx], compress(list[idx]));
        shift += all_bits[idx];
        encode->BlockSet(shift, all_bits[idx + best_block - 1], compress(list[idx + best_block - 1]));
        shift += all_bits[idx + best_block - 1];

        for (int i = 1; i < best_block - 1; ++i) {
            encode->BlockSet(shift, delta_bits[idx + i - 1], compress(list[i + idx] - list[i + idx - 1]));
            shift += delta_bits[idx + i - 1];
        }
    }

    PairType Find(uint32_t value, BitSetSIMD *encode, uint32_t shift, bool not_partial) {
        uint8_t *in = encode->GetOriginPointer();
        uint32_t len = shift / 8;
        in += len;
        if (not_partial) {
            uint32_t pre = 0;
            while (len < total_bits_ / 8) {
                uint32_t next = decode(in, &len) + pre;
                pre = next;
                if (value == next)
                    return PairType::Neighbor;
                else if (value < next)
                    return PairType::NonNeighbor;
            }
            return PairType::NonNeighbor;
        } else {
            uint32_t min = decode(in, &len);
            uint32_t max = decode(in, &len);
            uint32_t pre = min;
            if (value == min || value == max) return PairType::Neighbor;
            if (value < min || value > max) return PairType::Uncertain;

            while (len < total_bits_ / 8) {
                uint32_t next = decode(in, &len) + pre;
                pre = next;
                if (value == next)
                    return PairType::Neighbor;
                else if (value < next)
                    return PairType::NonNeighbor;
            }
            return PairType::NonNeighbor;
        }
    }

   private:
    std::vector<uint32_t> GetDeltas(const std::vector<uint32_t> &list) {
        std::vector<uint32_t> res;
        if (list.size() == 0) return res;
        res.push_back(list[0]);
        for (int i = 0; i < list.size() - 1; ++i) {
            res.push_back(list[i + 1] - list[i]);
        }
        return res;
    }

    std::vector<uint32_t> GetDeltaBits(const std::vector<uint32_t> &list) {
        std::vector<uint32_t> res;
        res.push_back(VbyteComp::IntegerBytes(list[1] - list[0]) * 8);
        for (int i = 1; i < list.size() - 1; ++i) {
            res.push_back(VbyteComp::IntegerBytes(list[i + 1] - list[i]) * 8);
        }
        return res;
    }
};

#endif  // SRC_INCLUDE_ENCODE_SINGLE_COMPRESSION_H_
