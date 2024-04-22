//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#include "delta_simd_codec.h"

/**
 * fork from stream vbytes
 * @param in
 * @param in_size
 * @param out
 * @param out_bytes_length control+ data
 */
void DeltaSIMDCodec::EncodeArray(const uint32_t *in, size_t in_size, uint8_t *out, uint32_t &out_bytes_length) {
    uint8_t *keyPtr = out;                // keys come immediately after 32-bit count
    uint32_t keyLen = (in_size + 3) / 4;  // 2-bits rounded to full byte
    uint8_t *dataPtr = keyPtr + keyLen;   // variable byte data after all keys
    uint32_t dataLen = EncodeDeltas(in, keyPtr, dataPtr, in_size);
    out_bytes_length = keyLen + dataLen;
}

static uint8_t ENCODE_LENGTH_MASK[4] = {0xFF, 0x03, 0x0F, 0x3F};

// return control + data
size_t DeltaSIMDCodec::GetEncodeLength(const uint8_t *in, size_t in_size) {
    uint32_t res = (in_size + 3) / 4;
    uint32_t data_byte_len = res - 1, mod = in_size & 0x03;
    for (int i = 0; i < data_byte_len; ++i) {
        res += lengthTable[in[i]];
    }
    res += lengthTable[in[data_byte_len] & ENCODE_LENGTH_MASK[mod]] - ((4 - mod) & 0x03);
    return res;
}

size_t DeltaSIMDCodec::GetOneEncodeLength(uint32_t integer) {
    if (integer < (1 << 8))
        return 1;
    else if (integer < (1 << 16))
        return 2;
    else if (integer < (1 << 24))
        return 3;
    else
        return 4;
}

void DeltaSIMDCodec::Decode(const uint8_t *in, size_t in_bytes_length, uint32_t *out, uint32_t &out_size) {}

void DeltaSIMDCodec::ParseEncode(const uint32_t *in, size_t in_size, std::vector<DeltaInfo> &delta_infos) {
    size_t size = 0;
    uint32_t pre = 0;
    for (int i = 0; i < in_size; ++i) {
        const uint32_t val = in[i] - pre;
        pre = in[i];
        uint32_t val_bits = GetOneEncodeLength(val);
        size += val_bits;
        if (i == 0)
            delta_infos.push_back({0, GetOneEncodeLength(in[i]), size});
        else
            delta_infos.push_back({val_bits, GetOneEncodeLength(in[i]), size});
    }
}

uint8_t DeltaSIMDCodec::EncodeDeltas(const uint32_t *in, uint8_t *__restrict__ keyPtr, uint8_t *dataPtr,
                                     uint32_t count) {
    uint32_t prev = 0, len = 0;
    if (count == 0) return 0;  // exit immediately if no data

    uint8_t shift = 0;  // cycles 0, 2, 4, 6, 0, 2, 4, 6, ...
    uint8_t key = 0;
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keyPtr++ = key;
            key = 0;
        }
        uint32_t val = GenValue(prev, in[c], c);
        uint8_t code = EncodeDelta(val, &dataPtr);
        len += code + 1;
        key = (uint8_t)(key | (code << shift));
        shift = (uint8_t)(shift + 2);
    }

    *keyPtr = key;  // write last key (no increment needed)
    return len;     // pointer to first unused data byte
}

// TODO try find lower

static uint8_t LAST_COMPARE_MASK[4] = {0x0F, 0x01, 0x03, 0x07};

Codec::FindRes DeltaSIMDCodec::FindWithCount(uint8_t *__restrict__ inbyte, uint32_t count, uint32_t element) {
    // find element by simd instruction
    uint32_t control_bytes = (count + 3) / 4;  // number of key bytes
    uint32_t left = count & 0x00000003;
    uint8_t *data_ptr = inbyte + control_bytes;
    __m128i compare = _mm_set1_epi32(element), pre = _mm_set1_epi32(0);
    // TODO replace with find lower
    if (control_bytes == 1) {
        uint8_t control = inbyte[0];
        __m128i data = DecodeData(control, &data_ptr);
        pre = RecoveryFromDelta(data, pre);
        int mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(pre, compare))) & LAST_COMPARE_MASK[left];
        if (mask == 0)
            return _mm_cvtsi128_si32(pre) == element ? Codec::FindRes::EXIST : Codec::FindRes::LESS_THAN_MIN;
        else if (mask == (15 & LAST_COMPARE_MASK[left]))
            return Codec::FindRes::GRATER_THAN_MAX;
        else {
            mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(pre, compare))) & LAST_COMPARE_MASK[left];
            return mask != 0 ? Codec::FindRes::EXIST : Codec::FindRes::NOT_EXIST;
        }
    } else {
        uint8_t control = inbyte[0];
        // TODO when left!=4
        __m128i data = DecodeData(control, &data_ptr);
        pre = RecoveryFromDelta(data, pre);
        uint32_t mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(pre, compare)));
        if (mask == 0) {
            return _mm_cvtsi128_si32(pre) == element ? Codec::FindRes::EXIST : Codec::FindRes::LESS_THAN_MIN;
        } else if (mask != 15) {
            mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(pre, compare)));
            return mask != 0 ? FindRes::EXIST : FindRes::NOT_EXIST;
        } else {
            for (int i = 1; i < control_bytes - 1; ++i) {
                control = inbyte[i];
                data = DecodeData(control, &data_ptr);
                pre = RecoveryFromDelta(data, pre);
                mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(pre, compare)));
                if (mask == 15)
                    continue;
                else {
                    mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(pre, compare)));
                    return mask != 0 ? FindRes::EXIST : FindRes::NOT_EXIST;
                }
            }
            // TODO last byte , try without move (>>left)
            control = inbyte[control_bytes - 1];
            data = DecodeData(control, &data_ptr);
            pre = RecoveryFromDelta(data, pre);
            mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(pre, compare))) & LAST_COMPARE_MASK[left];
            if (mask == (15 & LAST_COMPARE_MASK[left]))
                return FindRes::GRATER_THAN_MAX;
            else {
                mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(pre, compare))) & LAST_COMPARE_MASK[left];
                return mask != 0 ? FindRes::EXIST : FindRes::NOT_EXIST;
            }
        }
    }
}

__m128i DeltaSIMDCodec::DecodeData(uint8_t control, uint8_t *__restrict__ *data_ptr) {
    uint8_t len = lengthTable[control];
    __m128i Data = _mm_loadu_si128((__m128i *)*data_ptr);
    __m128i Shuf = *(__m128i *)&shuffleTable[control];

    Data = _mm_shuffle_epi8(Data, Shuf);
    *data_ptr += len;
    return Data;
}

__m128i DeltaSIMDCodec::RecoveryFromDelta(__m128i data, __m128i &pre) {
    // input : [X5-X4,X6-X5,X7-X6,X8-X7]  pre:[X1,X2,X3,X4]
    __m128i pre_data = _mm_shuffle_epi32(pre, 0xFF);  // copy with last element
    __m128i add = _mm_slli_si128(data, 4);
    data = _mm_add_epi32(data, add);
    add = _mm_slli_si128(data, 8);
    data = _mm_add_epi32(data, add);
    data = _mm_add_epi32(data, pre_data);
    return data;
}

uint32_t DeltaSIMDCodec::GetRequiredBytes(const std::vector<uint32_t> &list) {
    uint32_t len = (list.size() + 3) / 4, pre = 0;
    for (int i = 0; i < list.size(); ++i) {
        len += GetOneEncodeLength(GenValue(pre, list[i], i));
    }
    return len;
}

__m128i DeltaSIMDCodecV1::RecoveryFromDelta(__m128i data, __m128i &pre) {
    int8_t mask[16] = {-1, -1, -1, -1, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
    data = _mm_add_epi32(data, _mm_shuffle_epi8(data, *(__m128i *)mask));
    return data;
}

__m128i DeltaSIMDCodecV2::RecoveryFromDelta(__m128i data, __m128i &pre) {
    __m128i add = _mm_slli_si128(data, 4);
    data = _mm_add_epi32(data, add);
    add = _mm_slli_si128(data, 8);
    data = _mm_add_epi32(data, add);
    return data;
}

uint8_t DeltaSIMDCodecV3::EncodeDeltas(const uint32_t *in, uint8_t *keyPtr, uint8_t *dataPtr, uint32_t count) {
    assert(count <= 40);
    uint32_t prev = 0, len = 0;
    if (count == 0) return 0;  // exit immediately if no data

    uint8_t shift = 0;  // cycles 0, 2, 4, 6, 0, 2, 4, 6, ...
    uint8_t key = 0;
    uint32_t *order = ary_order[count - 1];
    for (uint32_t c = 0; c < count; c++) {
        if (shift == 8) {
            shift = 0;
            *keyPtr++ = key;
            key = 0;
        }
        uint32_t val = GenValue(prev, in[order[c]], c);
        uint8_t code = EncodeDelta(val, &dataPtr);
        len += code + 1;
        key = (uint8_t)(key | (code << shift));
        shift = (uint8_t)(shift + 2);
    }

    *keyPtr = key;  // write last key (no increment needed)
    return len;     // pointer to first unused data byte
}

uint32_t DeltaSIMDCodecV3::GetRequiredBytes(const std::vector<uint32_t> &list) {
    uint32_t len = (list.size() + 3) / 4, pre = 0;
    // since k<=10 , the bytes len must < 40
    // TODO optimize this
    if (list.size() > 40) {
        return list.size() + len;
    }
    uint32_t *order = ary_order[list.size() - 1];
    for (int i = 0; i < list.size(); ++i) {
        len += GetOneEncodeLength(GenValue(pre, list[order[i]], i));
    }
    return len;
}

uint32_t DeltaSIMDCodecV3::GetRequiredBytes(const std::vector<uint32_t> &list, uint32_t begin, uint32_t block_size) {
    uint32_t len = (block_size + 3) / 4, pre = 0;
    uint32_t *order = ary_order[block_size - 1];
    for (int i = 0; i < block_size; ++i) {
        len += GetOneEncodeLength(GenValue(pre, list[order[i] + begin], i));
    }
    return len;
}

Codec::FindRes DeltaSIMDCodecV3::FindWithCount(uint8_t *inbyte, uint32_t count, uint32_t element) {
    TreeNode *root = trees_ary[count].get_root();

    // find element by simd instruction
    uint32_t control_bytes = (count + 3) / 4;  // number of key bytes

    // first level
    uint8_t *data_ptr = inbyte + control_bytes;
    __m128i compare = _mm_set1_epi32(element);

    if (control_bytes == 1) {
        uint32_t left = count & 0x03;
        __m128i data = DecodeData(inbyte[0], &data_ptr);
        data = RecoveryFromDelta(data);

        if (HSIMD != 0) {
            uint32_t decode[4], recovery[4];
            decode[0] = _mm_extract_epi32(data, 0);
            decode[1] = _mm_extract_epi32(data, 1);
            decode[2] = _mm_extract_epi32(data, 2);
            decode[3] = _mm_extract_epi32(data, 3);
            RecoveryFromDelta(decode, recovery);
            for (int i = 0; i < 4; ++i) {
                if (element == decode[i]) return Codec::FindRes::EXIST;
            }
        }

        if ((_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(data, compare))) & LAST_COMPARE_MASK[left]) != 0)
            return Codec::FindRes::EXIST;
        int mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(data, compare))) & LAST_COMPARE_MASK[left];
        if (mask == 0)
            return Codec::FindRes::LESS_THAN_MIN;
        else if (mask == (15 & LAST_COMPARE_MASK[left]))
            return Codec::FindRes::GRATER_THAN_MAX;
        else {
            return Codec::FindRes::NOT_EXIST;
        }
    } else {
        uint8_t control = inbyte[0];
        // TODO when left!=4
        __m128i data = DecodeData(control, &data_ptr);
        data = RecoveryFromDelta(data);

        if (HSIMD != 0) {
            uint32_t decode[4], recovery[4];
            decode[0] = _mm_extract_epi32(data, 0);
            decode[1] = _mm_extract_epi32(data, 1);
            decode[2] = _mm_extract_epi32(data, 2);
            decode[3] = _mm_extract_epi32(data, 3);
            RecoveryFromDelta(decode, recovery);
            for (int i = 0; i < 4; ++i) {
                if (element == decode[i]) return Codec::FindRes::EXIST;
            }
        }

        if (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(data, compare))) != 0) return Codec::FindRes::EXIST;
        uint32_t mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(data, compare)));
        if (mask == 0) {
            return Codec::FindRes::LESS_THAN_MIN;
        } else if (mask == 15) {
            return FindRes::GRATER_THAN_MAX;
        } else {
            uint32_t *control_byte_len = new uint32_t[control_bytes];
            countByteLen(inbyte, control_bytes, &control_byte_len);
            for (int i = 0; i < root->child_size_; ++i) {
                if (mask == root->masks[i])
                    return DFS(root->children[i], compare, count, control_byte_len, inbyte, &data_ptr);
            }
            delete[] control_byte_len;
            return FindRes::NOT_EXIST;
        }
    }
}

__m128i DeltaSIMDCodecV3::DecodeData(uint8_t control, uint8_t *__restrict__ *data_ptr) {
    __m128i Data = _mm_loadu_si128((__m128i *)*data_ptr);
    __m128i Shuf = *(__m128i *)&shuffleTable[control];
    Data = _mm_shuffle_epi8(Data, Shuf);
    return Data;
}

Codec::FindRes DeltaSIMDCodecV3::DFS(TreeNode *node, __m128i &compare, int count,
                                     uint32_t *__restrict__ control_bytes_len, uint8_t *__restrict__ inbytes,
                                     uint8_t *__restrict__ *data_ptr) {
    uint8_t control = inbytes[node->idx_];
    uint8_t *ptr = *data_ptr;
    ptr += control_bytes_len[node->idx_];
    __m128i data = DecodeData(control, &ptr);
    data = RecoveryFromDelta(data);

    if (HSIMD != 0) {
        uint32_t decode[4], recovery[4], comp;
        comp = _mm_extract_epi32(compare, 0);
        decode[0] = _mm_extract_epi32(data, 0);
        decode[1] = _mm_extract_epi32(data, 1);
        decode[2] = _mm_extract_epi32(data, 2);
        decode[3] = _mm_extract_epi32(data, 3);
        RecoveryFromDelta(decode, recovery);
        for (int i = 0; i < 4; ++i) {
            if (comp == decode[i]) return Codec::FindRes::EXIST;
        }
    }
    if (node->children == nullptr) {
        if (node->idx_ == (count + 3) / 4 - 1) {
            uint8_t left = count & 0x00000003;
            return (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(data, compare))) & LAST_COMPARE_MASK[left]) != 0
                       ? Codec::FindRes::EXIST
                       : Codec::FindRes::NOT_EXIST;
        } else {
            return _mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(data, compare))) != 0 ? FindRes::EXIST
                                                                                          : FindRes::NOT_EXIST;
        }
    } else {
        if (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(data, compare))) != 0) return FindRes::EXIST;
        uint8_t mask = _mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32(data, compare)));
        for (int i = 0; i < node->child_size_; ++i) {
            if (mask == node->masks[i])
                return DFS(node->children[i], compare, count, control_bytes_len, inbytes, data_ptr);
            return FindRes::NOT_EXIST;
        }
    }
}

void DeltaSIMDCodecV3::DecodeControl(uint8_t *inbyte, int count, uint32_t **out) {
    // pre length , so  (count+2)/4
    uint32_t group_nums = (count + 2) / 4;
    uint32_t *len = *out;
    *len++ = 0;
    __m128i pre = _mm_set1_epi32(0);
    for (int i = 0; i < group_nums; ++i) {
        __m128i data = _mm_set_epi32(TotalLengthTable[inbyte[i]][3], TotalLengthTable[inbyte[i]][2],
                                     TotalLengthTable[inbyte[i]][1], TotalLengthTable[inbyte[i]][0]);
        data = _mm_add_epi32(data, pre);
        _mm_storeu_si128((__m128i *)len, data);
        pre = _mm_set1_epi32(len[3]);
        len += 4;
    }
}

void DeltaSIMDCodecV3::countByteLen(uint8_t *inbyte, uint32_t group_nums, uint32_t **out) {
    uint32_t *len = *out;
    *len++ = 0;
    for (int i = 0; i < group_nums - 1; ++i) {
        len[i] = len[i - 1] + lengthTable[inbyte[i]];
    }
}
