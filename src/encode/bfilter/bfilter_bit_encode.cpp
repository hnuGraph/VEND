//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#include "encode/bfilter/bfilter_bit_encode.h"

void BFilterBitEncode::EdgeSet(uint32_t vertex1, uint32_t vertex2) {
#if UNDIRECTED == 1
    if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
    for (int i = 0; i < best_hash_nums_; ++i) encode_->SetOne(Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]));
}

PairType BFilterBitEncode::NEpairTest(uint32_t vertex1, uint32_t vertex2) {
#if UNDIRECTED == 1
    if (vertex1 > vertex2) std::swap(vertex1, vertex2);
#endif
    for (int i = 0; i < best_hash_nums_; ++i) {
        if (!encode_->IsOne(Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]))) return PairType::NonNeighbor;
    }
    return PairType::Uncertain;
}

void BFilterBitEncode::InsertPair(uint32_t vertex1, uint32_t vertex2) { EdgeSet(vertex1, vertex2); }

void BFilterBitEncode::LoadFromDb(std::string file_path) {
    std::ifstream file(file_path);
    uint32_t key;
    std::vector<uint32_t> hash_key;
    for (uint64_t i = 0; i < hash_size_ / 32; ++i) {
        file >> key;
        hash_key.push_back(key);
    }
    encode_->BlockSet(hash_key);
    file.close();
}

void BFilterBitEncode::EncodePersistent(std::string file_path) {
    std::ofstream file(file_path);
    std::vector<uint32_t> hash_key = encode_->Dump();
    for (uint64_t i = 0; i < hash_size_ / 32; ++i) {
        file << hash_key[i] << "\t";
        if (i % 1000000 == 0) file.flush();
    }
    file.close();
};