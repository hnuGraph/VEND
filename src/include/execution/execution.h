//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_EXECUTION_H
#define VEND_EXECUTION_H

#include <string>
#include <random>
#include "graph/graph.h"
#include "graph/large_graph.h"
#include "memory"

class Execution {

public:
    Execution(){}
    Execution(std::string db_path, std::string vend_prefix, VendType vend_type) : db_path_(db_path), vend_path_(
            VendFactory::GetVendPath(vend_prefix, vend_type)), vend_type_(vend_type) {}

    virtual void Execute() = 0;

protected:
    std::string db_path_;
    std::string vend_path_;
    std::shared_ptr<Graph> graph_;
    VendType vend_type_;


};


#endif //VEND_EXECUTION_H
