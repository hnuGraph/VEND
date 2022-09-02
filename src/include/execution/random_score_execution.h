
#ifndef VEND_RANDOM_SCORE_EXECUTION_H
#define VEND_RANDOM_SCORE_EXECUTION_H

#include"score_execution.h"

class RandomScoreExecution : public ScoreExecution {

public:

    RandomScoreExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path) :
            ScoreExecution(pair_path, vend_prefix, output_path, db_path) {};

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override{
        uint32_t vertex_size1 = VERTETX_TEST_SIZE1;
        uint32_t vertex_size2 = vertex_size/VERTETX_TEST_SIZE1;
        std::unordered_set<uint32_t> vertex_list1,vertex_list2;
        RandomCreate(vertex_size1,&vertex_list1,vertex_size2,&vertex_list2);
        for(auto v1:vertex_list1){
            for(auto v2:vertex_list2){
                vertex_list->push_back({v2,v1});
            }
        }
    }

};

#endif //VEND_RANDOM_SCORE_EXECUTION_H
