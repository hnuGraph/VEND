//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//





#include "encode/single/hybrid_encode.h"

PairType HybridEncode::NonNeighborTest(uint32_t vertex1, const DecodeInfo &decode_info1, uint32_t vertex2) {

    if (decode_info1.decodable)
        return encode_bitset_[vertex1].BlockFind(1, v_bits_size_, MAX_K_CORE_SIZE, vertex2);
    else {

        if (vertex2 - decode_info1.min <= decode_info1.max - decode_info1.min) {
            return encode_bitset_[vertex1].BlockFind(BLOCK_BEGIN_INDEX, v_bits_size_, decode_info1.block_num, vertex2);
        } else {
            if (encode_bitset_[vertex1].IsOne(Hash(vertex2, decode_info1.hash_begin)))
                return PairType::Uncertain;
            else
                return PairType::NonNeighbor;
        }

    }
}

PairType HybridEncode::NonNeighborTest(uint32_t vertex1, uint32_t vertex2) {

    auto &encode = encode_bitset_[vertex1];
    if (IsDecodable(vertex1)) {
        return encode.BlockFind(1,v_bits_size_,MAX_K_CORE_SIZE,vertex2);
    } else {
        uint32_t min = 0, max = vertex_id_upper_, block_num = encode.GetBlockNum();
        BlockType type = GetBlockModel(vertex1);
        uint32_t hash_begin=block_num * v_bits_size_ + 3 + log_k_;
        // not left most
        if (type != BlockType::LeftMost) {
            min = block_num == 0 ? 0 : encode.BlockGet(3 + log_k_, v_bits_size_);
        }

        if (type != BlockType::RightMost) {
            max = block_num == 0 ? 0 : encode.BlockGet(3 + log_k_ + (block_num - 1) * v_bits_size_, v_bits_size_);
        }


        // search hash
        if (vertex2 < min || vertex2 > max) {
            if (hash_begin==PER_ENCODE_BIT_SIZE||encode.IsOne(Hash(vertex2, hash_begin)))
                return PairType::Uncertain;
            else
                return PairType::NonNeighbor;
        } else {
            return encode.BlockFind(BLOCK_BEGIN_INDEX, v_bits_size_, block_num, vertex2);
        }
    }
}


void HybridEncode::Decode(uint32_t vertex, DecodeInfo &decode_info) {
    auto &encode = encode_bitset_[vertex];
    if (encode.bits_[0] >> 31) {
        decode_info.decodable = true;
        return;
    } else {
        decode_info.decodable = false;
        decode_info.block_num = encode.GetBlockNum();
        if (decode_info.block_num == 0) {
            decode_info.hash_begin = BLOCK_BEGIN_INDEX;
            decode_info.max = 0;
            decode_info.min = 0;
        } else {
            BlockType type = GetBlockModel(encode);
            //    FullHash=1, LeftMost=2, RightMost=4, Middle=8
            if (type & 0x0d)
                decode_info.min = encode.BlockGet(BLOCK_BEGIN_INDEX, v_bits_size_);
            else
                decode_info.min = 0;
            if (type & 0x0b)
                decode_info.max = encode.BlockGet(BLOCK_BEGIN_INDEX + (decode_info.block_num - 1) * v_bits_size_,
                                                  v_bits_size_);
            else
                decode_info.max = vertex_id_upper_;
            decode_info.hash_begin = BLOCK_BEGIN_INDEX + decode_info.block_num * v_bits_size_;
        }
    }
}


void HybridEncode::EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
    std::sort(neighbors.begin(), neighbors.end());
    auto &encode = encode_bitset_[vertex_id];
    uint32_t score;
    encode.Clear();
    if (neighbors.size() <= MAX_K_CORE_SIZE) {
        encode.SetOne((uint32_t)0);
        uint32_t index = 1;
        for (auto iter = neighbors.begin(); iter != neighbors.end(); ++iter, index += v_bits_size_)
            encode.BlockSet(index, v_bits_size_, *iter);
    } else {
        encode = DynamicChoose(vertex_id, neighbors, &score);

    }


}

void HybridEncode::InsertPair(uint32_t vertex1, uint32_t vertex2) {
    if (NEpairTest(vertex1, vertex2) == PairType::Uncertain)
        return;
    EncodeType encode_type1 = GetEncodeType(vertex1);
    EncodeType encode_type2 = GetEncodeType(vertex2);
    if (encode_type1 > encode_type2) {
        std::swap(vertex1, vertex2);
        std::swap(encode_type1, encode_type2);
    }
    std::vector<uint32_t> neighbor2, neighbor1 = GetAllNeighbors(vertex1);
    neighbor1.push_back(vertex2);

    switch (encode_type1) {
        case EncodeType::UnFull:
            EncodeVertex(vertex1, neighbor1);
            return;
        case EncodeType::Full:
            if (encode_type2 == EncodeType::Full) {
                neighbor2 = GetAllNeighbors(vertex2);
                neighbor2.push_back(vertex1);
                std::sort(neighbor2.begin(), neighbor2.end());
                std::sort(neighbor1.begin(), neighbor1.end());
                RestructChoose(vertex1, neighbor1, vertex2, neighbor2);
            } else {

                EncodeVertex(vertex1, neighbor1);
            }
            return;
        case EncodeType::NonDecodable:
            neighbor2 = GetAllNeighbors(vertex2);
            neighbor2.push_back(vertex1);
            std::sort(neighbor1.begin(), neighbor1.end());
            std::sort(neighbor2.begin(), neighbor2.end());
            RestructChoose(vertex1, neighbor1, vertex2, neighbor2);
            return;
    }
}

void
HybridEncode::RestructChoose(uint32_t vertex1, std::vector<uint32_t> &neighbor1, uint32_t vertex2,
                             std::vector<uint32_t> &neighbor2) {
    uint32_t score1, score2;
    auto bitset1 = DynamicChoose(vertex1, neighbor1, &score1);
    auto bitset2 = DynamicChoose(vertex2, neighbor2, &score2);
    if (score1 > score2)
        encode_bitset_[vertex1] = bitset1;
    else
        encode_bitset_[vertex2] = bitset2;
}

void HybridEncode::DeletePair(uint32_t vertex1, uint32_t vertex2) {
    if (NonNeighborTest(vertex1, vertex2) != PairType::NonNeighbor)
        RemoveNeighbor(vertex1, vertex2);
    if (NonNeighborTest(vertex1, vertex2) != PairType::NonNeighbor)
        RemoveNeighbor(vertex2, vertex1);
}

bool HybridEncode::RemoveNeighbor(uint32_t vertex1, uint32_t vertex2) {
    std::vector<uint32_t> neighbors = GetAllNeighbors(vertex1);
    auto iter = std::find(neighbors.begin(), neighbors.end(), vertex2);
    if (iter == neighbors.end())
        return false;
    neighbors.erase(iter);
    EncodeVertex(vertex1, neighbors);
    return true;
}


BitSet<PER_ENCODE_BIT_SIZE>
HybridEncode::DynamicChoose(uint32_t vertex, std::vector<uint32_t> &neighbors, uint32_t *count) {

    uint32_t max_score = 0, neighbor_size = neighbors.size();
    BitSet<PER_ENCODE_BIT_SIZE> best_bitset;
    HashCount *hash_count = new HashCount();
    // full hash
    hash_count->ReConstruct(neighbors, 0);
    ChooseHighest(neighbors, &max_score, &best_bitset, hash_count, 0, 0, BlockType::FullHash);
    for (uint32_t block_size = 1; block_size <= MAX_INTEGER_SIZE; ++block_size) {
        // left most
        hash_count->ReConstruct(neighbors, block_size);
        ChooseHighest(neighbors, &max_score, &best_bitset, hash_count, block_size, 0, BlockType::LeftMost);
        if (block_size == 1) {
            hash_count->ReConstruct(neighbors, block_size, true);
            ChooseHighest(neighbors, &max_score, &best_bitset, hash_count, block_size, neighbor_size - block_size,
                          BlockType::RightMost);
            continue;
        }
        // middle
        uint32_t first_idx;
        for (first_idx = 1; first_idx <= neighbor_size - block_size - 1; ++first_idx) {
            hash_count->AddHash(neighbors[first_idx - 1]);
            hash_count->RemoveHash(neighbors[first_idx + block_size - 1]);
            ChooseHighest(neighbors, &max_score, &best_bitset, hash_count, block_size, first_idx, BlockType::Middle);
        }
        // right most
        ChooseHighest(neighbors, &max_score, &best_bitset, hash_count, block_size, first_idx, BlockType::RightMost);
    }
    delete hash_count;
    if (count != nullptr)
        *count = max_score;
    return best_bitset;
}


void
HybridEncode::ChooseHighest(const std::vector<uint32_t> &neighbors, uint32_t *max_score,
                            BitSet<PER_ENCODE_BIT_SIZE> *best_bitset, HashCount *hash_count,
                            uint32_t block_size, uint32_t first_idx,
                            const BlockType &type) {
    uint32_t score = hash_count->GetScore(neighbors, block_size, first_idx, type);
    if (*max_score < score) {
        *max_score = score;
        std::vector<uint32_t> neighbor_encode(neighbors.begin() + first_idx,
                                              neighbors.begin() + first_idx + block_size);
        std::vector<uint32_t> neighbor_left(neighbors.begin(), neighbors.begin() + first_idx);
        if ((first_idx + block_size - 1) != neighbors.size())
            neighbor_left.insert(neighbor_left.end(), neighbors.begin() + first_idx + block_size, neighbors.end());
        SetNonDecodable(neighbor_encode, neighbor_left, type, best_bitset);
    }
}


void
HybridEncode::SetNonDecodable(const std::vector<uint32_t> &neighbor_encode, const std::vector<uint32_t> &neighbor_left,
                              const BlockType &type,
                              BitSet<PER_ENCODE_BIT_SIZE> *bitset) {
    bitset->Clear();
    //bitset->SetZero(0);
    SetBlockModel(bitset, type);
    bitset->BlockSet(3, log_k_, neighbor_encode.size());
    for (uint32_t index = 3 + log_k_, i = 0; i < neighbor_encode.size(); ++i, index += v_bits_size_) {
        bitset->BlockSet(index, v_bits_size_, neighbor_encode[i]);
    }
    uint32_t hash_begin = neighbor_encode.size() * v_bits_size_ + log_k_ + 3;
    for (auto iter = neighbor_left.begin(); iter != neighbor_left.end(); ++iter) {
        bitset->SetOne(Hash(*iter, hash_begin));
    }
}


std::vector<uint32_t> HybridEncode::GetBlockInteger(uint32_t vertex) {
    std::vector<uint32_t> block_integers;
    if (IsDecodable(vertex)) {
        for (uint32_t index = 1;
             index <= PER_ENCODE_BIT_SIZE - v_bits_size_ + 1; index += v_bits_size_) {
            uint32_t neighbor = encode_bitset_[vertex].BlockGet(index, v_bits_size_);
            if (!neighbor)
                return block_integers;
            block_integers.push_back(neighbor);
        }

    } else {
        for (uint32_t index = 3 + log_k_;
             index < 3 + log_k_ + GetBlockSize(vertex) * v_bits_size_; index += v_bits_size_) {
            block_integers.push_back(encode_bitset_[vertex].BlockGet(index, v_bits_size_));
        }

    }
    return block_integers;
}


//TODO
uint32_t HybridEncode::NePairCount(uint32_t vertex_id, const BitSet<PER_ENCODE_BIT_SIZE> &encode) {
    return 0;
}









