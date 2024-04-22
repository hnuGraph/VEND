//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VEND_VEND_H
#define VEND_VEND_H

#include <memory>

#include "assert.h"
#include "encode/encode.h"
#include "encode/single/hybrid_encode.h"

class Vend {
   public:
    Vend() = default;

    Vend(std::shared_ptr<std::vector<std::vector<uint32_t>>> &adj_list, const std::string &encode_path,
         std::shared_ptr<DbEngine> &db)
        : adjacency_list_(adj_list), encode_path_(encode_path), data_db_(db) {}

    Vend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : encode_path_(encode_path), data_db_(db) {}

    Vend(std::shared_ptr<std::vector<std::vector<uint32_t>>> &adj_list, std::shared_ptr<DbEngine> &db)
        : adjacency_list_(adj_list), data_db_(db) {}

    /**
     *  execute encoding for the graph
     * */
    virtual void BuildEncoding() = 0;

    virtual void Delete(uint32_t vertex1, uint32_t vertex2) { encodes_->DeletePair(vertex1, vertex2); };

    virtual void Insert(uint32_t vertex1, uint32_t vertex2) { encodes_->InsertPair(vertex1, vertex2); };

    /**
     *  load graph from database
     * */
    virtual void LoadDbData() {
        assert(data_db_ != nullptr);
        adjacency_list_->resize(VERTEX_SIZE + 1);
        data_db_->InitIter();
        uint32_t key;
        std::vector<uint32_t> value;
        while (data_db_->Next(&key, &value)) {
            adjacency_list_->at(key).assign(value.begin(), value.end());
        }
    }

    /**
     *  load encode
     * */
    virtual void LoadEncode() { encodes_->LoadFromDb(encode_path_); };

    /**
     *  write encode into database
     * */
    virtual void EncodePersistent() { encodes_->EncodePersistent(encode_path_); };

    /**
     *  @return 1 : if vertex1-vertex2 is a  edge exists in our encoding
     *          0 : if vertex1-vertex2 is a  edge not exists in our coding
     *          2:  uncertain edge (select by db next )
     * */
    PairType Determine(uint32_t vertex1, uint32_t vertex2) { return encodes_->NEpairTest(vertex1, vertex2); };

    void Decode(uint32_t vertex, HybridEncode::DecodeInfo &decode_info) {
        return encodes_->Decode(vertex, decode_info);
    }

    PairType Determine(uint32_t vertex1, const HybridEncode::DecodeInfo &decode_info1, uint32_t vertex2,
                       const HybridEncode::DecodeInfo &decode_info2) {
        return encodes_->NEpairTest(vertex1, decode_info1, vertex2, decode_info2);
    }

   protected:
    std::shared_ptr<std::vector<std::vector<uint32_t>>> adjacency_list_;

    std::shared_ptr<Encode> encodes_;
    std::string encode_path_;
    std::shared_ptr<DbEngine> data_db_;
};

#endif  // VEND_VEND_H
