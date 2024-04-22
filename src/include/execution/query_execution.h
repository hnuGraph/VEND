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

#include <unordered_map>

#include "algorithm"
#include "execution.h"
#include "pair_list.h"
#include "util/timer.h"

#if VERIFY == 1
static bool exist = false;
static std::set<std::pair<uint32_t, uint32_t>> res_map1;
static std::set<std::pair<uint32_t, uint32_t>> res_map2;
#endif
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
    QueryExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
                   VendType vend_type, int db_type, bool directed)
        : PairList(pair_path, output_path + ".query", QUERY_LIST_SIZE), Execution(db_path, vend_prefix, vend_type) {
        graph_ = std::make_shared<Graph>(db_path, vend_path_, vend_type, db_type, directed);
    }
    void EdgeQuery(uint32_t vertex1, uint32_t vertex2) {
        if (graph_->GetEdge(vertex1, vertex2) == PairType::Uncertain)
            graph_->DbQuery(vertex1, vertex2);
        else
            ++filtered_;
    }

    void Execute() override {
        // graph_->BackUpDb();
        graph_->Init();
        Init();
        std::cout << " init pair finished" << std::endl;
        int res = 0;
        Timer *timer = new Timer();
        timer->StartTimer();

#if VERIFY == 1
        if (vend_type_ != VendType::NoVend) {
            for (auto &pair : pair_list_) {
                PairType type = graph_->GetEdge(pair.first, pair.second);
                if (type == PairType::Uncertain) {
                    if (graph_->DbQuery(pair.first, pair.second)) {
                        ++res;
                        if (!exist) {
                            res_map1.insert({pair.first, pair.second});
                        } else {
                            res_map2.insert({pair.first, pair.second});
                        }
                    }
                } else if (type == PairType::Neighbor) {
                    if (!exist) {
                        res_map1.insert({pair.first, pair.second});
                    } else {
                        res_map2.insert({pair.first, pair.second});
                    }
                    ++res;
                    ++filtered_;
                } else {
                    ++filtered_;
                }
            }
        } else {
            for (auto &pair : pair_list_) {
                //                if (graph_->DbQuery(pair.first, pair.second))
                //                    ++res;
                if (graph_->DbQuery(pair.first, pair.second)) {
                    ++res;
                    if (!exist) {
                        res_map1.insert({pair.first, pair.second});
                    } else {
                        res_map2.insert({pair.first, pair.second});
                    }
                }
            }
        }
#endif
        uint32_t f_items = 0, true_false = 0;
#if VERIFY == 0

        if (vend_type_ != VendType::NoVend) {
            for (auto &pair : pair_list_) {
                PairType type = graph_->GetEdge(pair.first, pair.second);
                if (type != PairType::Uncertain) {
                    ++filtered_;
                    if (type == PairType::Neighbor)
                        ++res;
                    else
                        ++true_false;
                } else {
#if IN_MEMORY == 0
                    if (!graph_->DbQuery(pair.first, pair.second)) {
                        ++f_items;
                    } else {
                        ++res;
                    }
#else

                    if (!graph_->TestEdgeByAdj(pair.first, pair.second)) {
                        ++f_items;
                    } else {
                        ++res;
                    }
#endif
                }
            }
        } else {
            for (auto &pair : pair_list_) {
#if IN_MEMORY == 0
                if (graph_->DbQuery(pair.first, pair.second)) ++res;

#else
                if (graph_->TestEdgeByAdj(pair.first, pair.second)) ++res;
#endif
            }
        }
#endif
        timer->StopTimer();

#if VERIFY == 1
        if (exist) {
            std::cout << "compare two set:" << std::endl;
            for (auto v : res_map1) {
                if (res_map2.find(v) == res_map2.end()) {
                    std::cout << v.first << " " << v.second << std::endl;
                }
            }
            std::cout << " pair set:res_map2 -res_map1" << std::endl;
            for (auto v : res_map2) {
                if (res_map1.find(v) == res_map1.end()) {
                    std::cout << v.first << " " << v.second << std::endl;
                    // PairType type = graph_->GetEdge(v.first, v.second);
                }
            }
        }
        exist = true;
#endif

        double percentage = (double)filtered_ / QUERY_LIST_SIZE * 100;
        double score = (double)true_false / (QUERY_LIST_SIZE - res) * 100;
        std::ofstream output(output_path_, std::ios::out | std::ios::app);
        std::cout << VEND_STRING[vend_type_] << " query time cost :" << timer->CountTime() / 1000000000
                  << " filtered: " << filtered_ << " percentage: " << percentage << " res:" << res
                  << " score: " << score << "\n";
        output << VEND_STRING[vend_type_] << ","
               << "query "
               << "," << list_size_ << "," << timer->CountTime() / 1000000000 << "," << filtered_ << "," << percentage
               << "\n";
// graph_->DestoryDb();
#if FALSE_OPSITIVE == 1
        std::cout << "false positive ratio : " << (double)f_items / (f_items + filtered_) * 100 << std::endl;
        std::cout << "false positive ratio (v2): "
                  << (double)f_items / (pair_list_.size() - filtered_ + true_false) * 100 << std::endl;
#endif
        std::cout << "memory usage:" << memory::getMemory() << " KB" << std::endl;
    };

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override {
        std::vector<uint32_t> random_array;
        std::uniform_int_distribution<unsigned> u;
        std::default_random_engine e(time(NULL));

        for (int i = 1; i <= VERTEX_SIZE; ++i) random_array.push_back(i);

        std::shuffle(random_array.begin(), random_array.end(), e);
        uint32_t vertex1, vertex2;
        // v1

        // while (vertex_list->size() < vertex_size) {
        //     for (auto vertex : random_array) {
        //         std::vector<uint32_t> neighbors;
        //         graph_->GetNeighbors(vertex, neighbors);
        //         int neighbor_size = neighbors.size();
        //         if(neighbor_size==0) continue;
        //         vertex1 = neighbors[u(e) % neighbor_size];
        //         graph_->GetNeighbors(vertex1, neighbors);
        //         neighbor_size = neighbors.size();
        //         if(neighbor_size==0) continue;
        //         vertex2 = neighbors[u(e) % neighbor_size];
        //         if (vertex == vertex2) continue;

        //         if (graph_->GetNeighborSize(vertex) > graph_->GetNeighborSize(vertex2)) {
        //             std::swap(vertex, vertex2);
        //         }

        //         vertex_list->push_back({vertex, vertex2});
        //         if (vertex_list->size() >= vertex_size) {
        //             return;
        //         }
        //     }
        // }

        // v2 : more common neighbors
        while (vertex_list->size() < vertex_size) {
            for (auto vertex : random_array) {
                std::vector<uint32_t> neighbors;
                graph_->GetNeighbors(vertex, neighbors);
                int neighbor_size = neighbors.size();
                if (neighbor_size <= 1) continue;

                int random1 = u(e) % neighbor_size;
                int random2 = u(e) % neighbor_size;

                uint32_t v1 = neighbors[random1], v2 = neighbors[random2];
                if (v1 == v2) continue;
                // if (graph_->GetNeighborSize(v1) > graph_->GetNeighborSize(v2)) {
                //     std::swap(v1, v2);
                // }
                vertex_list->push_back({v1, v2});

                if (vertex_list->size() >= vertex_size) return;
            }
        }
    };

   private:
    uint64_t filtered_ = 0;
    uint64_t nepass_ = 0;
};

#endif  // VEND_QUERY_EXECUTION_H
