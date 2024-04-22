//===----------------------------------------------------------------------===//
//
//
//                  Encode.h
//          super class for  encoding plan
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VEND_ENCODE_H
#define VEND_ENCODE_H

#include <memory>
#include <set>
#include <vector>

#include "assert.h"
#include "common/config.h"
#include "common/hash_config.h"
#include "dbengine/dbengine.h"

class Encode {
   public:
    struct DecodeInfo {
        bool decodable;
        uint16_t block_num;
        uint16_t hash_begin;
        uint32_t min = 0;
        uint32_t max = 0;

        bool operator<(const DecodeInfo &lsh) const { return this->decodable < lsh.decodable; }
    };

    Encode() : vertex_id_upper_(VERTEX_SIZE + 1), vertex_size_(VERTEX_SIZE) {}

    /**
     *  test whether vertex2 is a neighbor to vertex1
     *  @return  true if vertex2 is a neighbor to vertex1
     * */
    virtual PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) = 0;

    virtual PairType NEpairTest(uint32_t vertex1, const DecodeInfo &decode_info1, uint32_t vertex2,
                                const DecodeInfo &decode_info2){};

    virtual inline void Decode(uint32_t vertex, DecodeInfo &decode_info){};

    // encode edge<vertex1,vertex2>
    virtual void EdgeSet(uint32_t vertex1, uint32_t vertex2){};

    /**
     *  encode one vertex by it's neighbors
     *
     * */
    virtual void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors){};

    virtual void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &out_neigh, std::vector<uint32_t> &in_neigh){};

    virtual void InsertPair(uint32_t vertex1, uint32_t vertex2) = 0;

    virtual void DeletePair(uint32_t vertex1, uint32_t vertex2){};

    virtual bool IsDeletable(uint32_t vertex1, uint32_t vertex2) { return false; };

    virtual void Clear(){};

    /**
     *  load encode from database
     *  @param encode_db_: database to store the encodes
     * */
    virtual void LoadFromDb(DbEngine *encode_db){};

    virtual void LoadFromDb(std::string file_path){};

    /**
     *  write encode into database
     * */
    virtual void EncodePersistent(DbEngine *encode_db){};

    virtual void EncodePersistent(std::string file_path){};

    // directed

    virtual void EncodePersistent(std::string file_path,bool out){};

    virtual void LoadFromDb(std::string file_path,bool out){};

    virtual uint32_t GetIntSize(){};

    virtual uint32_t GetBytesSize(){};

    void SetDb(std::shared_ptr<DbEngine> &db) { db_ = db; }

    // query neighbors from adj db
    std::vector<uint32_t> DbQuery(uint32_t vertex) {
        assert(db_ != nullptr);
        std::vector<uint32_t> neighbors;
        db_->Get(vertex, neighbors);
        return neighbors;
    };

   protected:
    // vertex id range from [1,max_vid_)
    uint32_t vertex_id_upper_;
    // equals vertex_id_upper_ -1
    uint32_t vertex_size_;
    std::shared_ptr<DbEngine> db_;
};

#endif  // VEND_ENCODE_H
