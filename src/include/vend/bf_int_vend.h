//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_BF_INT_VEND_H
#define VEND_BF_INT_VEND_H

#include "vend.h"
#include "encode/bfilter/bfilter_int_encode.h"

class BFilterIntVend : public Vend {

public:
    BFilterIntVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
                   std::shared_ptr<DbEngine> &db)
            : Vend(adj_list, encode_path, db) {
        encodes_ = std::make_shared<BFilterIntEncode>();
        encodes_->SetDb(db);
    }

    BFilterIntVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : Vend(encode_path, db) {
        adjacency_list_= nullptr;
        encodes_ = std::make_shared<BFilterIntEncode>();
        encodes_->SetDb(db);
    }

    void BuildEncoding() override {
        //assert(adjacency_list_->size() == VERTEX_SIZE + 1);
        assert(encodes_ != nullptr);
        if (adjacency_list_) {
            //std::cout << "build encode from adj \n";
            for (uint32_t vertex1 = 1; vertex1 < adjacency_list_->size(); ++vertex1) {
                for (auto &vertex2: adjacency_list_->at(vertex1)) {
                    if (vertex1 < vertex2)
                        encodes_->EdgeSet(vertex1, vertex2);
                }
            }

        } else {
            data_db_->InitIter();
            uint32_t vertex1;
            std::vector<uint32_t> neighbors;
            while (data_db_->Next(&vertex1, &neighbors)) {
                for (auto vertex2: neighbors) {
                    if (vertex1 < vertex2)
                        encodes_->EdgeSet(vertex1, vertex2);
                }
            }
        }
    }
};


#endif //VEND_BF_INT_VEND_H
