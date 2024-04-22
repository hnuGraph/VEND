
#ifndef VEND_RANDOM_QUERY_EXECUTION_H
#define VEND_RANDOM_QUERY_EXECUTION_H


#include "query_execution.h"
class RandomQueryExecution:public QueryExecution{
public:
    RandomQueryExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
            VendType vend_type,int db_type)
    : QueryExecution(pair_path,vend_prefix, output_path, db_path,vend_type,db_type){
        
    }



    void CreateList(uint32_t vertex_size, std::vector <std::pair<uint32_t, uint32_t>> *vertex_list) override{

        std::uniform_int_distribution<unsigned> u(1, VERTEX_SIZE);
        std::default_random_engine e;
        e.seed(time(NULL));
        uint32_t vertex1,vertex2;
        while (vertex_list->size() < vertex_size) {
            vertex1 = u(e);
            vertex2 = u(e);
            if(vertex1==vertex2)
                continue;
            vertex_list->push_back({vertex1,vertex2});
        }

    }


};





#endif //VEND_RANDOM_QUERY_EXECUTION_H
