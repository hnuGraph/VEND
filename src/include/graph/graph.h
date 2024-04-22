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

#include "dbengine/rocksdb.h"
#include "util/json.cpp"
#include "vend/vend.h"
#include "vend/vend_factory.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>

#include "chrono"
#include "util/timer.h"

using json = nlohmann::json;

class Graph {
   public:
    Graph(std::string data_path, std::string db_path)
        : data_path_(data_path), db_path_(db_path), vertex_size(VERTEX_SIZE), directed_(false) {}

    Graph(std::string db_path, std::string vend_path, VendType vend_type)
        : db_path_(db_path), vend_path_(vend_path), vend_type_(vend_type), vertex_size(VERTEX_SIZE), directed_(false){};

    Graph(std::string db_path, std::string vend_path, VendType vend_type, int db_type)
        : db_path_(db_path),
          vend_path_(vend_path),
          vend_type_(vend_type),
          db_type_(db_type),
          vertex_size(VERTEX_SIZE),
          directed_(false){};

    Graph(std::string db_path, std::string vend_path, VendType vend_type, int db_type, bool directed)
        : db_path_(db_path),
          vend_path_(vend_path),
          vend_type_(vend_type),
          db_type_(db_type),
          vertex_size(VERTEX_SIZE),
          directed_(directed){};

    Graph(std::string data_path, std::string db_path, std::string vend_path, VendType vend_type)
        : vend_path_(vend_path),
          db_path_(db_path),
          data_path_(data_path),
          vend_type_(vend_type),
          vertex_size(VERTEX_SIZE),
          directed_(false){};
    Graph(std::string data_path, std::string db_path, std::string vend_path, VendType vend_type,bool directed)
        : vend_path_(vend_path),
          db_path_(db_path),
          data_path_(data_path),
          vend_type_(vend_type),
          vertex_size(VERTEX_SIZE),
          directed_(directed){};
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

    void LoadDataFromDb() {
        adjacency_list_->resize(VERTEX_SIZE + 1);

        // read data from db
        graph_db_->InitIter();

        uint32_t vertex;
        std::vector<uint32_t> neighbors;
        uint32_t idx = 0;
        uint32_t edges = 0;
        while (graph_db_->Next(&vertex, &neighbors)) {
            // idx++;
            // edges += neighbors.size();
            // if (idx % 1000000 == 0) {
            //     std::cout << "idx: " << idx << " neighbor size:" << edges << std::endl;
            // }
            std::sort(neighbors.begin(), neighbors.end());
            adjacency_list_->at(vertex) = neighbors;
            neighbors.clear();
        }
    }

    PairType GetEdge(uint32_t vertex1, uint32_t vertex2) { return vend->Determine(vertex1, vertex2); };

    void AddEdge(uint32_t vertex1, uint32_t vertex2, Timer *timer) {
        timer->StartTimer();
        vend->Insert(vertex1, vertex2);
        timer->StopTimer();
        // insert db
        // DbInsert(vertex1, vertex2);
    };

    void GetNeighbors(uint32_t vertex1, std::vector<uint32_t> &neighbors) { graph_db_->Get(vertex1, neighbors); }

    int GetNeighborSize(uint32_t vertex) {
        std::vector<uint32_t> neighbors;
        graph_db_->Get(vertex, neighbors);
        return neighbors.size();
    }

    void RemoveEdge(uint32_t vertex1, uint32_t vertex2, Timer *timer) {
        timer->StartTimer();
        vend->Delete(vertex1, vertex2);
        timer->StopTimer();
        // DbDelete(vertex1, vertex2);
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

    bool TestEdgeByAdj(uint32_t vertex1, uint32_t vertex2) {
        assert(adjacency_list_ != nullptr);
        if (adjacency_list_->at(vertex1).size() > adjacency_list_->at(vertex2).size()) std::swap(vertex1, vertex2);
        return std::binary_search(adjacency_list_->at(vertex1).begin(), adjacency_list_->at(vertex1).end(), vertex2);
    }

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
    std::shared_ptr<std::vector<std::vector<uint32_t>>> adjacency_list_;
    std::shared_ptr<std::vector<std::vector<uint32_t>>> in_adj_list_;
    bool directed_;
};

#endif  // VEND_GRAPH_H
