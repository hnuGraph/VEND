//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_INSERT_EXECUTION_H
#define VEND_INSERT_EXECUTION_H


#include "execution/execution.h"
#include "execution/pair_list.h"
#include <memory>
class InsertExecution : public PairList, public Execution {

public:
    InsertExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
                    VendType vend_type) : PairList(pair_path, output_path, INSERT_LIST_SIZE),
                                          Execution(db_path, vend_prefix, vend_type) {
        if (!IS_LARGE)
            graph_ = std::make_shared<Graph>(db_path, vend_path_, vend_type);
        else
            graph_ = std::make_shared<LargeGraph>(db_path, vend_path_, vend_type);
    }

    void Execute() override {
        //graph_->BackUpDb();
        graph_->Init();
        Init();
        std::unique_ptr<Timer> timer= std::make_unique<Timer>();
        for (auto &pair:pair_list_) {
            graph_->AddEdge(pair.first, pair.second, timer.get());
        }
        std::ofstream output(output_path_,std::ios::out|std::ios::app);
        std::cout << VEND_STRING[vend_type_] << " insert time cost :" << timer->CountTime()/1000000 << "\n";
        output << VEND_STRING[vend_type_] << ',' << "insert" << ',' << list_size_ << "," << timer->CountTime()/1000000 << "\n";
       // graph_->DestoryDb();
    };

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override {
        std::uniform_int_distribution<unsigned> u(1, VERTEX_SIZE + 1);
        std::default_random_engine e;
        e.seed(time(NULL));
        while (vertex_list->size() < vertex_size) {
            uint32_t vertex1 = u(e) % VERTEX_SIZE + 1;
            uint32_t vertex2 = u(e) % VERTEX_SIZE + 1;
            if (vertex1 == vertex2)
                continue;
            if (!graph_->DbQuery(vertex1, vertex2)) {
                vertex_list->push_back(std::pair<uint32_t, uint32_t>(vertex1, vertex2));
            }
        }

    };
};


#endif //VEND_INSERT_EXECUTION_H
