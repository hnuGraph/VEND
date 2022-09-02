//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_DELETE_EXECUTION_H
#define VEND_DELETE_EXECUTION_H

#include "execution/execution.h"
#include "execution/pair_list.h"

class DeleteExecution : public PairList, public Execution {

public:
    DeleteExecution() {}

    DeleteExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
                    VendType vend_type) : PairList(pair_path, output_path, DELETE_LIST_SIZE),
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
        std::cout << "start delete" << "\n";
        std::unique_ptr<Timer> timer = std::make_unique<Timer>();
        int batch = 0;
        for (auto &pair: pair_list_) {
            ++batch;
            if (graph_->GetVendType() == VendType::BloomFilterBit && batch >= 10)
                break;
            graph_->RemoveEdge(pair.first, pair.second, timer.get());
        }
        std::ofstream output(output_path_, std::ios::out|std::ios::app);
        std::cout << VEND_STRING[vend_type_] << " delete time cost :" << timer->CountTime() / 1000000 << "\n";
        output << VEND_STRING[vend_type_] << ',' << "delete" << ',' << batch << "," << timer->CountTime() / 1000000
               << "\n";
        //graph_->DestoryDb();
    };

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override {
        srand(time(NULL));
        std::vector<uint32_t> neighbors;
        while (vertex_list->size() < vertex_size) {
            std::vector<uint32_t>().swap(neighbors);
            uint32_t vertex1 = rand() % VERTEX_SIZE + 1;
            graph_->GetNeighbors(vertex1, &neighbors);
            if (neighbors.size() == 0)
                continue;
            uint32_t vertex2 = neighbors[rand() % neighbors.size()];
            vertex_list->push_back(std::pair<uint32_t, uint32_t>(vertex1, vertex2));


        }
    };
};


#endif //VEND_DELETE_EXECUTION_H
