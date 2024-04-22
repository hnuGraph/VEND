//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_INSERT_DELETE_EXECUTION_H
#define VEND_INSERT_DELETE_EXECUTION_H

#include "delete_execution.h"
class InsertDeleteExecution : public DeleteExecution{

public:

    InsertDeleteExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path,
                          VendType vend_type) : DeleteExecution(pair_path, vend_prefix, output_path, db_path,
                                                                vend_type) {};

    void Execute() override {
        graph_->BackUpDb();
        graph_->Init();
        Init();

        std::unique_ptr<Timer> timer= std::make_unique<Timer>();
        int batch = 0;

        for (auto &pair:pair_list_) {
            ++batch;
            if (graph_->GetVendType() == VendType::BloomFilterBit && batch >= 10)
                break;
            graph_->RemoveEdge(pair.first, pair.second, timer.get());
            graph_->AddEdge(pair.first, pair.second, timer.get());
        }

        std::ofstream output(output_path_, std::ios::out|std::ios::app);
        std::cout << VEND_STRING[vend_type_] << " insert and delete time cost :" << timer->CountTime() << "\n";
        output << VEND_STRING[vend_type_] << ',' << "insert and delete" << ',' << batch << "," << timer->CountTime()
               << "\n";
        graph_->DestoryDb();

    }


};


#endif //VEND_INSERT_DELETE_EXECUTION_H
