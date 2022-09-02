//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_BF_BIT_VEND_H
#define VEND_BF_BIT_VEND_H


#include "vend.h"
#include "encode/bfilter/bfilter_bit_encode.h"
#include "encode/bfilter/bbfilter_encode.h"
#include "encode/bfilter/cbfilter_encode.h"
#include "vend_factory.h"

class BFilterBitVend : public Vend {

public:
    BFilterBitVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
                   std::shared_ptr<DbEngine> &db)
            : Vend(adj_list, encode_path, db) {
        encodes_ = std::make_shared<BFilterBitEncode>();
        encodes_->SetDb(db);
    }
    BFilterBitVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
                   std::shared_ptr<DbEngine> &db,int type)
            : Vend(adj_list, encode_path, db) {

        if(type==0)
            encodes_ = std::make_shared<BFilterBitEncode>();
        else if(type==1)
            encodes_ = std::make_shared<CBFilterEncode>();
        else
            encodes_ = std::make_shared<BBFilterEncode>();
        encodes_->SetDb(db);
    }

    BFilterBitVend( const std::string &encode_path,
                   std::shared_ptr<DbEngine> &db,int type)
            : Vend(encode_path, db) {

        if(type==0)
            encodes_ = std::make_shared<BFilterBitEncode>();
        else if(type==1)
            encodes_ = std::make_shared<CBFilterEncode>();
        else
            encodes_ = std::make_shared<BBFilterEncode>();
        encodes_->SetDb(db);
    }
    BFilterBitVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db)
            : Vend(encode_path, db) {
        adjacency_list_= nullptr;
        encodes_ = std::make_shared<BFilterBitEncode>();
        encodes_->SetDb(db);
    }

    void BuildEncoding() override {
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
            std::cout<<"build encode from db\n";
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


    void Delete(uint32_t vertex1, uint32_t vertex2) override {
        if (encodes_->IsDeletable(vertex1, vertex2))
            encodes_->DeletePair(vertex1, vertex2);
        else {
            LoadDbData();
            encodes_->Clear();
            BuildEncoding();
        }
    };
};


#endif //VEND_BF_BIT_VEND_H
