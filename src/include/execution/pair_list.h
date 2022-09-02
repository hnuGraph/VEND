//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_PAIR_LIST_H
#define VEND_PAIR_LIST_H

#include "graph/graph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>


static constexpr uint32_t QUERY_LIST_SIZE = 1000000;
static constexpr uint32_t INSERT_LIST_SIZE = 100000;
static constexpr uint32_t DELETE_LIST_SIZE = 100000;

class PairList {

public:
    PairList() {}

    PairList(std::string pair_path, std::string output_path, uint32_t list_size) : pair_path_(pair_path),
                                                                                   output_path_(output_path),
                                                                                   list_size_(list_size) {}

    /**
     * construct graph and vend
     * */
    virtual void Init();

    virtual void Execute() {};

    void LoadPairList(std::string path_path);

    void ListPersistent();

    virtual void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) {};


protected:
    uint32_t list_size_;
    std::string pair_path_;
    std::string output_path_;
    std::vector<std::pair<uint32_t, uint32_t>> pair_list_;
};


#endif //VEND_PAIR_LIST_H
