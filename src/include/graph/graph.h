//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_GRAPH_H
#define VEND_GRAPH_H


#include "vend/vend.h"
#include "dbengine/rocksdb.h"
#include "vend/vend_factory.h"
#include "dbengine/neo4j.h"
#include "dbengine/dgraph.h"
#include "dbengine/gstore.h"
#include "util/json.cpp"
using json = nlohmann::json;

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "chrono"
#include "util/timer.h"
#include <random>

class Graph {
public:
    Graph(std::string data_path, std::string db_path) : data_path_(data_path), db_path_(db_path),
                                                        vertex_size(VERTEX_SIZE) {}

    Graph(std::string db_path, std::string vend_path, VendType vend_type) : db_path_(db_path), vend_path_(
            vend_path), vend_type_(vend_type), vertex_size(VERTEX_SIZE) {
    };

    Graph(std::string db_path, std::string vend_path, VendType vend_type,int db_type) : db_path_(db_path), vend_path_(
            vend_path), vend_type_(vend_type), db_type_(db_type),vertex_size(VERTEX_SIZE) {

    };
    Graph(std::string data_path, std::string db_path, std::string vend_path, VendType vend_type) : vend_path_(
            vend_path), db_path_(db_path), data_path_(data_path), vend_type_(vend_type), vertex_size(VERTEX_SIZE) {
    };


    virtual void Init();


    /**
     *  read origin data from csv/txt etc. file  and initial adjacency list、edge database
     * */
    void LoadData();

    /**
     *  save graph into database
     * */
    void SaveData();

    virtual void LoadAndSaveData() {
        LoadData();
        SaveData();
    }

    /**
     *  encode graph
     * */
    void BuildEncode();

    PairType GetEdge(uint32_t vertex1, uint32_t vertex2) {
        return vend->Determine(vertex1, vertex2);

    };

    void AddEdge(uint32_t vertex1, uint32_t vertex2, Timer *timer) {
        timer->StartTimer();
        vend->Insert(vertex1, vertex2);
        timer->StopTimer();
        // insert db
        DbInsert(vertex1, vertex2);
    };

    void GetNeighbors(uint32_t vertex1, std::vector<uint32_t> *neighbors) {
        graph_db_->Get(vertex1, neighbors);
    }

    void RemoveEdge(uint32_t vertex1, uint32_t vertex2, Timer *timer) {
        timer->StartTimer();
        vend->Delete(vertex1, vertex2);
        timer->StopTimer();
        DbDelete(vertex1, vertex2);
    };


    /**
     *  functions for database
     *
     * */
    void BackUpDb();

    void DestoryDb();

    void DbInsert(uint32_t vertex1, uint32_t vertex2);

    void DbDelete(uint32_t vertex1, uint32_t vertex2);

    bool DbQuery(uint32_t vertex1, uint32_t vertex2);

    VendType GetVendType() { return vend_type_; }


protected:
    // csv / txt input file path
    std::string data_path_;
    // databse path
    std::string db_path_;
    std::string vend_path_;
    VendType vend_type_;
    int db_type_;
    uint32_t vertex_size = VERTEX_SIZE;
    std::shared_ptr<DbEngine> graph_db_;
    std::shared_ptr<Vend> vend;
    std::shared_ptr<std::vector<std::vector<uint32_t >>> adjacency_list_;

};

#endif //VEND_GRAPH_H
