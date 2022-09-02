//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_QUERY_EXECUTION_H
#define VEND_QUERY_EXECUTION_H

#include "pair_list.h"
#include "util/timer.h"
#include "algorithm"

class QueryExecution : public PairList, public Execution {

public:
    QueryExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
                   VendType vend_type, int db_type)
            : PairList(pair_path, output_path + ".query", QUERY_LIST_SIZE), Execution(db_path, vend_prefix, vend_type) {
        if (!IS_LARGE)
            graph_ = std::make_shared<Graph>(db_path, vend_path_, vend_type, db_type);
        else
            graph_ = std::make_shared<LargeGraph>(db_path, vend_path_, vend_type);
    }

    void EdgeQuery(uint32_t vertex1, uint32_t vertex2) {
        if (graph_->GetEdge(vertex1, vertex2) == PairType::Uncertain)
            graph_->DbQuery(vertex1, vertex2);
        else
            ++filtered_;
    }

    void Execute() override {
        //graph_->BackUpDb();
        graph_->Init();
        Init();
        Timer *timer = new Timer();
        timer->StartTimer();
        if (vend_type_ != VendType::NoVend) {
            for (auto &pair: pair_list_) {
                if (graph_->GetEdge(pair.first, pair.second) == PairType::Uncertain) {
                    graph_->DbQuery(pair.first, pair.second);

                } else
                    ++filtered_;
            }
        } else {
            for (auto &pair: pair_list_) {
                graph_->DbQuery(pair.first, pair.second);

            }
        }
        timer->StopTimer();
        double percentage = (double) filtered_ / QUERY_LIST_SIZE * 100;
        std::ofstream output(output_path_, std::ios::out|std::ios::app);
        std::cout << VEND_STRING[vend_type_] << " query time cost :" << timer->CountTime() / 1000000 << " filtered: "
                  << filtered_ << " percentage: " << percentage << "\n";
        output << VEND_STRING[vend_type_] << "," << "query " << "," << list_size_ << "," << timer->CountTime() / 1000000
               << "," << filtered_ << "," << percentage << "\n";
        //graph_->DestoryDb();

        std::cout << "memory usage:" << memory::getMemory() << " KB" << std::endl;
    };

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override {
        std::vector<uint32_t> random_array;
        std::uniform_int_distribution<unsigned> u;
        std::default_random_engine e(time(NULL));

        for (int i = 1; i <= VERTEX_SIZE; ++i)
            random_array.push_back(i);

        std::shuffle(random_array.begin(), random_array.end(), e);
        uint32_t vertex1, vertex2;
//
        while (vertex_list->size() < vertex_size) {
            for (auto vertex: random_array) {
                std::vector<uint32_t> neighbors;
                graph_->GetNeighbors(vertex, &neighbors);
                int neighbor_size = neighbors.size();
                vertex1 = neighbors[u(e) % neighbor_size];
                graph_->GetNeighbors(vertex1, &neighbors);
                neighbor_size = neighbors.size();
                vertex2 = neighbors[u(e) % neighbor_size];
                if (vertex == vertex2)
                    continue;
                vertex_list->push_back({vertex, vertex2});
                if (vertex_list->size() >= vertex_size) {
                    return;
                }

            }
        }
    };

private:
    uint64_t filtered_ = 0;
};

#endif //VEND_QUERY_EXECUTION_H
