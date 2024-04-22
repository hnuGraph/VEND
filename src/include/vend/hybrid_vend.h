//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_HYBRID_VEND_H
#define VEND_HYBRID_VEND_H

#include "vend/range_vend.h"
#include "encode/single/hybrid_encode.h"
#include <map>


class HybridVend : public RangeVend {

public:

    HybridVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
               std::shared_ptr<DbEngine> &db) {
        this->data_db_=db;
        this->encode_path_=encode_path;
        this->adjacency_list_=adj_list;
        Init(db);
    }

    HybridVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) {
        this->data_db_ = db;
        this->encode_path_=encode_path;
        Init(db);
    }

    void Init(std::shared_ptr<DbEngine> &db) {
        encodes_ = std::make_shared<HybridEncode>();
        encodes_->SetDb(db);
    }


};


#endif //VEND_HYBRID_VEND_H
