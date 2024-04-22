//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#include "encode/single/range_encode.h"

PairType RangeEncode::NonNeighborTest(uint32_t vertex1, uint32_t vertex2) {

    auto &encode = encode_bitset_[vertex1];
    if (GetFlag(vertex1)) {
        for (uint32_t index = v_bits_size_; index <= PER_ENCODE_BIT_SIZE - v_bits_size_ + 1; index += v_bits_size_) {
            if (encode->BlockGet(index, v_bits_size_) == vertex2)
                return PairType::Neighbor;
        }
        return PairType::NonNeighbor;
    } else {
        uint32_t min = encode->BlockGet(v_bits_size_, v_bits_size_);
        uint32_t max = encode->BlockGet(v_bits_size_ * max_int_size_, v_bits_size_);
        if (vertex2 == min || vertex2 == max)
            return PairType::Neighbor;
        if (vertex2 < min || vertex2 > max)
            return PairType::Uncertain;
        for (uint32_t index = v_bits_size_;
             index <= v_bits_size_ * max_int_size_; index += v_bits_size_) {
            if (encode->BlockGet(index, v_bits_size_) == vertex2)
                return PairType::Neighbor;
        }
        return PairType::NonNeighbor;
    }
}

std::vector<uint32_t> RangeEncode::GetBlockInt(uint32_t vertex) {
    std::vector<uint32_t> block;
    if (GetFlag(vertex)) {
        for (uint32_t i = 1; i <= max_int_size_; ++i) {
            uint32_t neighbor = encode_bitset_[vertex]->BlockGet(i * v_bits_size_, v_bits_size_);
            if (!neighbor)
                return block;
            block.push_back(neighbor);
        }
    } else {
        for (uint32_t i = 1; i <= max_int_size_; ++i) {
            block.push_back(encode_bitset_[vertex]->BlockGet(i * v_bits_size_, v_bits_size_));
        }
    }
}

void RangeEncode::EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
    std::sort(neighbors.begin(),neighbors.end());
    auto &encode = encode_bitset_[vertex_id];
    encode->Clear();
    if (neighbors.size() <= max_int_size_) {
        SetFlag(vertex_id, true);

        for (int i = 1; i <= neighbors.size(); ++i) {
            encode->BlockSet(i * v_bits_size_, v_bits_size_, neighbors[i - 1]);
        }
    } else {
        SetFlag(vertex_id, false);
        uint32_t max_score = 0, score = 0;
        int idx = -1, size = neighbors.size();
        max_score = neighbors[max_int_size_ - 2];
        for (uint32_t i = 0; i < size - max_int_size_ + 1; ++i) {
            //score = neighbor_temp[i + max_int_size_ - 1] - neighbor_temp[i] - max_int_size_ + 1;
            score = neighbors[i + max_int_size_ - 1] - neighbors[i];
            if (max_score < score) {
                max_score = score;
                idx = i;
            }
        }
        score = vertex_id_upper_ - neighbors[size - max_int_size_ + 1];
        if (score > max_score)
            idx = size - max_int_size_ + 1;

        for (int i = 1; i <= max_int_size_; ++i) {
            if (i == 1 && idx == -1)
                encode->BlockSet(i * v_bits_size_, v_bits_size_, 0);
            else if (i == max_int_size_ && idx ==size-max_int_size_+1)
                encode->BlockSet(i * v_bits_size_, v_bits_size_, vertex_id_upper_);
            else
                encode->BlockSet(i * v_bits_size_, v_bits_size_, neighbors[idx + i - 1]);
        }
    }
}

std::vector<uint32_t> RangeEncode::GetAllNeighbors(uint32_t vertex) {
    if (GetFlag(vertex))
        return GetBlockInt(vertex);
    return DbQuery(vertex);
}

void RangeEncode::InsertPair(uint32_t vertex1, uint32_t vertex2) {
    EncodeType encode_type1 = GetEncodeType(vertex1);
    EncodeType encode_type2 = GetEncodeType(vertex2);
    if (encode_type1 > encode_type2) {
        std::swap(vertex1, vertex2);
        std::swap(encode_type1, encode_type2);
    }
    std::vector<uint32_t> neighbors = GetAllNeighbors(vertex1);
    neighbors.push_back(vertex2);
    EncodeVertex(vertex1, neighbors);
}

void RangeEncode::DeletePair(uint32_t vertex1, uint32_t vertex2) {
    if (NonNeighborTest(vertex1, vertex2) != PairType::NonNeighbor) {
        RemoveNeighbor(vertex1, vertex2);
    }

    if (NonNeighborTest(vertex2, vertex1) != PairType::NonNeighbor) {
        RemoveNeighbor(vertex2, vertex1);
    }

}

bool RangeEncode::RemoveNeighbor(uint32_t vertex1, uint32_t vertex2) {
    std::vector<uint32_t> neighbors = GetAllNeighbors(vertex1);
    auto iter=std::find(neighbors.begin(),neighbors.end(),vertex2);
    if(iter!=neighbors.end()){
        neighbors.erase(iter);
        EncodeVertex(vertex1, neighbors);
        return true;
    }
    return false;


}
