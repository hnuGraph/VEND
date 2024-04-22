
#ifndef VEND_LARGE_GRAPH_H
#define VEND_LARGE_GRAPH_H

#include "graph.h"
#include <unordered_map>
#include <malloc.h>
class LargeGraph : public Graph {

public:

    LargeGraph(std::string data_path, std::string db_path) : Graph(data_path, db_path), write_batch(WriteBatchSize) {};


    LargeGraph(std::string db_path, std::string vend_path, VendType vend_type) :
            Graph(db_path, vend_path, vend_type), write_batch(WriteBatchSize) {};

    LargeGraph(std::string data_path, std::string db_path, std::string vend_path, VendType vend_type) :
            Graph(data_path, db_path, vend_path, vend_type), write_batch(WriteBatchSize) {};


    virtual void Init() override;

    void LoadAndSaveData() override;

    void UpdateEdges(std::unordered_map<uint32_t, std::vector<uint32_t>> *adj);


    void InitGraph();
private:
    int write_batch;

};


#endif //VEND_LARGE_GRAPH_H
