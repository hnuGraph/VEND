#ifndef VEND_BITSET_SIMD_H
#define VEND_BITSET_SIMD_H

#include <cstring>
#include <memory>
#include <vector>

#include "assert.h"
#include "common/config.h"
#include "encode/codec/SIMD/simd_common.h"

#define DIVIDE8(x) (x >> 3)
#define MOD8(x) (x & 0x07)

static constexpr uint8_t IS_ONE_MASK[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

// 0111 1111, 0011 1111
// 1011 1111, 1001 1111
static constexpr uint8_t ZERO_BLOCK_MASK[8][9] = {
    {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00},
    {0xFF, 0xBF, 0x9F, 0x8F, 0x87, 0x83, 0x81, 0x80},
    {0xFF, 0xDF, 0xCF, 0xC7, 0xC3, 0xC1, 0xC0},
    {0xFF, 0xEF, 0xE7, 0xE3, 0xE1, 0xE0},
    {0xFF, 0xF7, 0xF3, 0xF1, 0xF0},
    {0xFF, 0xFB, 0xF9, 0xF8},
    {0xFF, 0xFD, 0xFC},
    {0xFF, 0xFE},
};

// 1000 0000 , 1100 0000,1110 0000 .....
// 0100 0000 , 0110 0000 , 0111 0000
static constexpr uint8_t ONE_BLOCK_MASK[8][9] = {
    {0, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF},
    {0, 0x40, 0x60, 0x70, 0x78, 0x7C, 0x7E, 0x7F},
    {0, 0x20, 0x30, 0x38, 0x3C, 0x3E, 0x3F},
    {0, 0x10, 0x18, 0x1C, 0x1E, 0x1F},
    {0, 0x08, 0x0C, 0x0E, 0x0F},
    {0, 0x04, 0x06, 0x07},
    {0, 0x02, 0x03},
    {0, 0x01},
};

static constexpr uint8_t LEFT_ONE_MASK[9] = {0x00, 0x01, 0x03, 0x07, 0x0F,
                                             0x1F, 0x3F, 0x7F, 0

};

class BitSetSIMD {
   public:
    BitSetSIMD(size_t size) : size_(size / 8) {
        data_ = new uint8_t[size_];
        Clear();
    }

    BitSetSIMD(const BitSetSIMD &bitset) {
        this->size_ = bitset.size_;
        this->data_ = new uint8_t[size_];
        memcpy(data_, bitset.data_, sizeof(uint8_t) * size_);
    }

    BitSetSIMD &operator=(const BitSetSIMD &rhs) {
        delete[] data_;
        this->size_ = rhs.size_;
        this->data_ = new uint8_t[size_];
        memcpy(data_, rhs.data_, sizeof(uint8_t) * size_);
        return *this;
    }

    ~BitSetSIMD() {
        if (data_ != nullptr) delete[] data_;
        data_ = nullptr;
    }

    inline void SetOne(size_t pos) { data_[DIVIDE8(pos)] |= 1 << (7 - MOD8(pos)); }

    inline void SetZero(size_t pos) { data_[DIVIDE8(pos)] &= 0xFF ^ (1 << (7 - MOD8(pos))); }

    inline bool IsOne(size_t pos) { return data_[pos >> 3] & IS_ONE_MASK[pos & 0x07]; }

    inline bool Clear() {
        memset(data_, 0, sizeof(uint8_t) * size_);
        return true;
    }

    inline uint16_t GetBlockNum() { return BlockGet(3, LOG_K); };

    void BlockSet(size_t begin, uint32_t block_size, uint32_t value) {
        uint8_t begin_idx = DIVIDE8(begin);
        size_t end_idx = (begin + block_size - 1) / 8;
        uint8_t left = 7 - (begin + block_size - 1) % 8;
        if (begin_idx != end_idx) {
            data_[end_idx] &= LEFT_ONE_MASK[left];
            data_[end_idx] |= value << left;
            block_size -= 8 - left;
            value = value >> (8 - left);
            while (block_size > 8) {
                data_[--end_idx] &= 0x00;
                data_[end_idx] |= value;
                value = value >> 8;
                block_size -= 8;
            }
            data_[begin_idx] &= ZERO_BLOCK_MASK[MOD8(begin)][block_size];
            data_[begin_idx] |= value;
        } else {
            data_[begin_idx] &= ZERO_BLOCK_MASK[MOD8(begin)][block_size];
            data_[begin_idx] |= value << left;
        }
    };

    void BlockSet(const std::vector<uint8_t> &encode) {
        assert(encode.size() == size_);
        for (size_t i = 0; i < size_; ++i) data_[i] = encode[i];
    };

    void BlockSet(const std::vector<uint32_t> &encode) {
        assert(encode.size() * 4 == size_);
        for (size_t i = 0; i < encode.size(); ++i) {
            // big endian
            data_[4 * i + 3] = (encode[i] & 0x000000ff);
            data_[4 * i + 2] = (encode[i] & 0x0000ff00) >> 8;
            data_[4 * i + 1] = (encode[i] & 0x00ff0000) >> 16;
            data_[4 * i] = (encode[i] & 0xff000000) >> 24;
        }
    };

    std::vector<uint32_t> Dump() {
        assert(size_ % 4 == 0);
        std::vector<uint32_t> integers;
        for (size_t i = 0; i < size_ / 4; ++i) {
            integers.push_back((data_[4 * i] << 24) + (data_[4 * i + 1] << 16) + (data_[4 * i + 2] << 8) +
                               data_[4 * i + 3]);
        }
        return integers;
    }

    uint32_t BlockGet(size_t begin, uint32_t block_size) {
        // block_size <= 32

        size_t i = DIVIDE8(begin);
        size_t last = begin + block_size - 1;
        size_t j = DIVIDE8(last);
        if (i == j) {
            return (data_[i] & ONE_BLOCK_MASK[MOD8(begin)][block_size]) >> (7 - MOD8(last));
        } else {
            uint8_t last_mod = MOD8(last);
            uint32_t val = data_[j] >> (7 - last_mod);
            for (size_t idx = 1; idx < j - i; ++idx) {
                val += data_[j - idx] << (8 * (idx - 1) + last_mod + 1);
            }
            val += (data_[i] & ONE_BLOCK_MASK[MOD8(begin)][8 - MOD8(begin)]) << (block_size - 8 + MOD8(begin));
            return val;
        }
    };

    PairType BlockFind(size_t begin, size_t block_size, uint32_t block_num, uint32_t value) {
        for (size_t i = begin; i < block_size * block_num + begin; i += block_size) {
            size_t val = BlockGet(i, block_size);
            if (val == value)
                return PairType::Neighbor;
            else if (value < val)
                return PairType::NonNeighbor;
        }
        return PairType::NonNeighbor;
    }

    PairType BlockFindSIMD(size_t begin, size_t block_size, uint32_t block_num, uint32_t value) {
        __m128i compare = _mm_set1_epi32(value);
        __m128i keys;
        size_t shift = 0;
        for (int t = 0; t < block_num / 4; ++t, shift += block_size * 4) {
            keys = _mm_set_epi32(BlockGet(begin + shift, block_size), BlockGet(begin + shift + block_size, block_size),
                                 BlockGet(begin + shift + block_size * 2, block_size),
                                 BlockGet(begin + shift + block_size * 3, block_size));
            if (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32 (keys, compare))) != 0) return PairType::Neighbor;
        }
        block_num = block_num % 4;
        if (block_num == 0) {
            return PairType::NonNeighbor;
        } else if (block_num == 1) {
            keys = _mm_set_epi32(BlockGet(begin + shift, block_size), 0, 0, 0);
        } else if (block_num == 2) {
            keys = _mm_set_epi32(BlockGet(begin + shift, block_size), BlockGet(begin + shift + block_size, block_size),
                                 0, 0);
        } else if (block_num == 3) {
            keys = _mm_set_epi32(BlockGet(begin + shift, block_size), BlockGet(begin + shift + block_size, block_size),
                                 BlockGet(begin + shift + block_size * 2, block_size), 0);
        }
        if (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(keys, compare))) != 0) return PairType::Neighbor;
        return PairType::NonNeighbor;
    }

    inline uint8_t *GetPointer(size_t offset) { return data_ + offset; }

    inline uint8_t *GetEncodePointer() { return data_ + 1; }

    inline uint8_t *GetOriginPointer(){return data_;}


    inline bool GetSecondBit() { return data_[0] & 0x40; }

    inline bool GetFirstBit() { return data_[0] & 0x80; }

    inline bool GetThridBit() { return data_[0] & 0x20; }

    inline uint32_t GetBitSize() { return size_; };

    // encode->BlockGet(3, 5);
    inline uint32_t GetEncodeSize() { return data_[0] & 0x1F; };

    void Reconstruct(BitSetSIMD *rhs) {
        assert(rhs->size_ == size_);
        memcpy(data_, rhs->data_, sizeof(uint8_t) * size_);
    }

   private:
    size_t size_;
    uint8_t *data_;
};

#endif  // VEND_BITSET_SIMD_H
