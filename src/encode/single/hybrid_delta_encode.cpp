////===----------------------------------------------------------------------===//
////
////
////
////
////
////
////===----------------------------------------------------------------------===//
//
//#include "hybrid_delta_encode.h"
//
//#include <iostream>
//
//PairType HybridDeltaEncode::NonNeighborTest(uint32_t vertex1, uint32_t vertex2) {
//    auto &encode = encode_bitset_[vertex1];
//    if (IsDecodable(vertex1)) {
//        return FindDecodable(encode, vertex2);
//    } else {
//        return FindNonDecodable(encode, vertex2);
//    }
//}
//
//PairType HybridDeltaEncode::FindDecodable(BitSet<PER_ENCODE_BIT_SIZE> &encode, uint32_t vertex) {
//    std::vector<uint32_t> decode_data(K_SIZE * sizeof(uint32_t), 0);
//    uint32_t decode_size;
//    uint8_t **pp = encode.CastToUint8Pointer(info_bytes_length_);
//    codec_->DecodeUnDirectArray(pp, max_bytes_left_,
//                                decode_data.data(), decode_size);
//    for (int i = 0; i < decode_size; ++i) {
//        if (decode_data[i] == 0)
//            return PairType::NonNeighbor;
//        else if (decode_data[i] == vertex)
//            return PairType::Neighbor;
//    }
//    return PairType::NonNeighbor;
//}
//
//
//PairType HybridDeltaEncode::FindNonDecodable(BitSet<PER_ENCODE_BIT_SIZE> &encode, uint32_t vertex) {
//    //TODO pass param with decode_data to avoid destroying and initialing
//    uint32_t min = 0, max = vertex_id_upper_, bytes_length = encode.BlockGet(3, 5), decode_size;
//    BlockType type = GetBlockModel(encode);
//    uint32_t hash_begin = (info_bytes_length_ + bytes_length) * 8;
//
//    uint8_t **pp = encode.CastToUint8Pointer(1);
//    VarintCodec::FindRes res = codec_->Find(pp, bytes_length, vertex);
//    if (res == VarintCodec::FindRes::EXIST)
//        return PairType::Neighbor;
//    else if (res == VarintCodec::FindRes::NOT_EXIST ||
//             (type == BlockType::LeftMost && res == VarintCodec::FindRes::LESS_THAN_MIN) ||
//             (type == BlockType::RightMost && res == VarintCodec::FindRes::GRATER_THAN_MAX))
//        return PairType::NonNeighbor;
//
//    // hash part
//    if (hash_begin == PER_ENCODE_BIT_SIZE || encode.IsOne(Hash(vertex, hash_begin)))
//        return PairType::Uncertain;
//    else
//        return PairType::NonNeighbor;
//
//}
////PairType HybridDeltaEncode::FindNonDecodable(BitSet<PER_ENCODE_BIT_SIZE> &encode, uint32_t vertex) {
////    //TODO pass param with decode_data to avoid destroying and initialing
////    uint32_t min = 0, max = vertex_id_upper_, bytes_length = encode.BlockGet(3, 5), decode_size;
////    BlockType type = GetBlockModel(encode);
////    uint32_t hash_begin = (info_bytes_length_ + bytes_length) * 8;
////    std::vector<uint32_t> decode_data(K_SIZE * sizeof(uint32_t));
////    uint8_t **pp = encode.CastToUint8Pointer(1);
////    codec_->DecodeUnDirectArray(pp, bytes_length, decode_data.data(), decode_size);
////    if (type != BlockType::LeftMost) {
////        min = bytes_length == 0 ? 0 : decode_data[0];
////    }
////
////    if (type != BlockType::RightMost) {
////        max = bytes_length == 0 ? 0 : decode_data[decode_size - 1];
////    }
////    if (vertex < min || vertex > max) {
////        if (hash_begin == PER_ENCODE_BIT_SIZE || encode.IsOne(Hash(vertex, hash_begin)))
////            return PairType::Uncertain;
////        else
////            return PairType::NonNeighbor;
////    } else {
////        for (int i = 0; i < decode_size; ++i) {
////            if (decode_data[i] == vertex)
////                return PairType::Neighbor;
////        }
////        return PairType::NonNeighbor;
////    }
////}
//
//
//void HybridDeltaEncode::EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) {
//    // std::sort(neighbors.begin(), neighbors.end());
//    auto &encode = encode_bitset_[vertex_id];
//    uint32_t score = 0;
//    uint32_t encode_bytes_length = 0;
//    encode.Clear();
//    /**
//     * TODO optimal encode , initial <gap,bytes-length> when first encode
//     */
//    if(neighbors.empty()){
//        encode.SetOne((uint32_t)0);
//        return;
//    }
//    std::vector<DeltaInfo> delta_infos;
//    codec_->ParseEncode(neighbors.data(), neighbors.size(), delta_infos);
//    if (delta_infos.back().total_bits <= max_bytes_left_ * sizeof(uint8_t)) {
//        encode.SetOne((uint32_t) 0);
//        //TODO fix decode
//        uint8_t **pp = encode.CastToUint8Pointer(info_bytes_length_);
//        codec_->EncodeUnDirectArray(neighbors.data(), neighbors.size(), pp,
//                                    encode_bytes_length);
//    } else {
//        DynamicChoose(encode, neighbors, delta_infos, score);
//    }
//}
//
//void
//HybridDeltaEncode::DynamicChoose(BitSet<PER_ENCODE_BIT_SIZE> &encode, std::vector<uint32_t> &neighbors,
//                                 std::vector<DeltaInfo> &delta_infos,
//                                 uint32_t &score) {
//
//    uint32_t neighbor_size = neighbors.size(), bytes_left = max_bytes_left_ * sizeof(uint8_t);
//    bool oversize = false;
//    BlockInfo best_block, current_block{0, 0, BlockType::FullHash};
//
//    // full_hash
//    best_block = current_block;
//    FullHashScoreCount(neighbors, bytes_left * 8, score);
//    for (int i = 0; i < neighbor_size && !oversize; ++i) {
//        //left most
//        uint32_t pre_bytes = delta_infos[i].val_bits;
//        current_block.block_size = 1;
//        current_block.begin = i;
//        if (i == 0) {
//            current_block.block_type = BlockType::LeftMost;
//        } else if (i != neighbor_size - 1) {
//            current_block.block_size += 1;
//            pre_bytes += delta_infos[i+1].delta_bits;
//            current_block.block_type = BlockType::Middle;
//        } else {
//            current_block.block_type = BlockType::RightMost;
//        }
//        // get score by add block size  iteratively
//        while (pre_bytes <= bytes_left) {
//            if (current_block.block_size + i == neighbor_size)
//                current_block.block_type = BlockType::RightMost;
//            if (ScoreCount(current_block, neighbors, (bytes_left - pre_bytes) * 8, score)) {
//                best_block = current_block;
//            }
//            if (current_block.block_type == BlockType::RightMost)
//                break;
//            current_block.block_size++;
//            pre_bytes += delta_infos[current_block.block_size + i - 1].delta_bits;
//        }
//    }
//    ConstructEncode(best_block, neighbors, encode);
//}
//
//bool HybridDeltaEncode::FullHashScoreCount(std::vector<uint32_t> &neighbors, uint32_t hash_bits, uint32_t &score) {
//    std::vector<bool> hash_val(hash_bits, false);
//    uint32_t hash_count = 0;
//    uint32_t max_hash_count =
//            hash_bits - score * hash_bits / VERTEX_SIZE;
//    // hash part
//    for (int i = 0; i < neighbors.size() && hash_count <= max_hash_count; ++i) {
//        uint32_t hash_now = neighbors[i] % hash_bits;
//        if (!hash_val[hash_now]) {
//            ++hash_count;
//            hash_val[hash_now] = true;
//        }
//    }
//    if (hash_count <= max_hash_count) {
//        score = VERTEX_SIZE * (hash_bits - hash_count) / hash_bits;
//        return true;
//    }
//    return false;
//}
//
//bool HybridDeltaEncode::ScoreCount(HybridDeltaEncode::BlockInfo &block_info, std::vector<uint32_t> &neighbors,
//                                   uint32_t hash_bits, uint32_t &score) {
//
//    uint32_t current_score, hash_count = 0;
//    uint32_t first_idx = block_info.begin, last_idx = block_info.begin + block_info.block_size - 1;
//    if (block_info.block_type == BlockType::RightMost) {
//        current_score = VERTEX_SIZE - neighbors[first_idx] + 1;
//    } else if (block_info.block_type == BlockType::LeftMost) {
//        current_score = neighbors[last_idx];
//    } else {
//        current_score = neighbors[last_idx] - neighbors[first_idx] + 1;
//    }
//
//    if (hash_bits == 0) {
//        if (current_score > score) {
//            score = current_score;
//            return true;
//        }
//        return false;
//    }
//    uint32_t max_hash_count =
//            current_score >= score ? hash_bits :
//            hash_bits - (score - current_score) * hash_bits / (VERTEX_SIZE - current_score);
//    if((neighbors.size()-block_info.block_size)*0.3>max_hash_count)
//        return false;
//    // count hash part
//    // TODO count score &&  fix bug : hash_bits==0
//    std::vector<bool> hash_val(hash_bits, false);
//    for (int i = 0; i < neighbors.size() && hash_count <= max_hash_count; ++i) {
//        if (i >= first_idx && i <= last_idx)
//            continue;
//        uint32_t hash_now = neighbors[i] % hash_bits;
//        if (!hash_val[hash_now]) {
//            ++hash_count;
//            hash_val[hash_now] = true;
//        }
//    }
//    // update score
//    if (hash_count <= max_hash_count) {
//        score = current_score + (VERTEX_SIZE - current_score) * (hash_bits - hash_count) / hash_bits;
//        return true;
//    }
//    return false;
//
//}
//
//void
//HybridDeltaEncode::ConstructEncode(const HybridDeltaEncode::BlockInfo &block_info, std::vector<uint32_t> &neighbors,
//                                   BitSet<PER_ENCODE_BIT_SIZE> &encode) {
//
//    uint8_t **in_pp = encode.CastToUint8Pointer(1);
//    uint32_t out_bytes_length = 0;
//    int last_idx = block_info.begin + block_info.block_size - 1;
//
//    SetBlockModel(&encode, block_info.block_type);
//    if (block_info.block_type != BlockType::FullHash) {
//        codec_->EncodeUnDirectArray(neighbors.data() + block_info.begin, block_info.block_size, in_pp,
//                                    out_bytes_length);
//    }
//    encode.BlockSet(3, 5, out_bytes_length);
//    uint32_t hash_begin = (out_bytes_length + info_bytes_length_) * 8;
//    // encode hash part
//    if(hash_begin==PER_ENCODE_BIT_SIZE)
//        return;
//    for (int i = 0; i < neighbors.size(); ++i) {
//        if (i >= block_info.begin && i <= last_idx)
//            continue;
//        encode.SetOne(Hash(neighbors[i], hash_begin));
//    }
//}
//
//
//
//
//
//
//
//
