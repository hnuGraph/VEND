//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#include "encode/bfilter/bfilter_int_encode.h"

PairType BFilterIntEncode::NEpairTest(uint32_t vertex1, uint32_t vertex2) {

    if (vertex1 > vertex2)
        std::swap(vertex1, vertex2);
    for (int i = 0; i < best_hash_nums_; ++i) {
        if (encode_[Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i])]==0)
            return PairType::NonNeighbor;
    }
    return PairType::Uncertain;
}

void BFilterIntEncode::EdgeSet(uint32_t vertex1, uint32_t vertex2) {
    if (vertex1 > vertex2)
        std::swap(vertex1, vertex2);
    for (int i = 0; i < best_hash_nums_; ++i)
        encode_[Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i])] += 1;

}

void BFilterIntEncode::InsertPair(uint32_t vertex1, uint32_t vertex2) {
    EdgeSet(vertex1, vertex2);
}

void BFilterIntEncode::DeletePair(uint32_t vertex1, uint32_t vertex2) {
    if (vertex1 > vertex2)
        std::swap(vertex1, vertex2);
    for (int i = 0; i < best_hash_nums_; ++i) {
        uint32_t hash_val = Hash(vertex1, vertex2, hash_param1_[i], hash_param2_[i]);
        if(encode_[hash_val] > 0)
            encode_[hash_val] -= 1;
    }
}

void BFilterIntEncode::LoadFromDb(std::string file_path) {
    std::ifstream file(file_path);
    int j=0;
    for (uint64_t i = 0; i < hash_size_; ++i) {
        file >> encode_[i];
    }
    file.close();
}

void BFilterIntEncode::EncodePersistent(std::string file_path) {
    std::ofstream file(file_path);
    for (uint64_t i = 0; i < hash_size_; ++i) {
        file << encode_[i] << "\t";
        if(i%1000000==0)
            file.flush();
    }
    std::cout<<" file ouput close"<<std::endl;
    file.close();
};