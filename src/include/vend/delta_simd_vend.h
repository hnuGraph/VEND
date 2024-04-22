#ifndef VEND_DELTA_SIMD_VEND_H
#define VEND_DELTA_SIMD_VEND_H


#include "vend.h"
#include "encode/SIMD/delta_simd_encode.h"

enum DeltaType : char {
    V0 = 0,     // x1 d2 d3 d4 d5 d6
    V1 = 1,     // x1 x2-x1 x3-x1 x4-x1 x5 x6-x5
    V2 = 2,      // x1 d2 d3 d4 x5 d6 ...
    V3 = 3
};


class DeltaSIMDVEND : public Vend {

public:
    DeltaSIMDVEND() = default;

    DeltaSIMDVEND(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list,
                  const std::string &encode_path,
                  std::shared_ptr<DbEngine> &db) : Vend(
            adj_list, encode_path, db) {
        initEncode();
        encodes_->SetDb(db);
    }

    DeltaSIMDVEND(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list,
                  DeltaType delta_type,
                  const std::string &encode_path,
                  std::shared_ptr<DbEngine> &db) : Vend(
            adj_list, encode_path, db) {
        initEncode(delta_type);
        encodes_->SetDb(db);
    }

    DeltaSIMDVEND(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list,
                  DeltaType delta_type,
                  size_t bit_size,
                  size_t vertex_size,
                  const std::string &encode_path,
                  std::shared_ptr<DbEngine> &db) : Vend(
            adj_list, encode_path, db) {
        initEncode(bit_size, vertex_size, delta_type);
        encodes_->SetDb(db);
    }

    DeltaSIMDVEND(const std::string &encode_path, std::shared_ptr<DbEngine> &db) {
        this->data_db_ = db;
        this->encode_path_=encode_path;
        initEncode(DeltaType::V3);
        encodes_->SetDb(db);
    }

    virtual void BuildEncoding() override;

protected:
    void initEncode(DeltaType type = DeltaType::V0) {
        if (type == DeltaType::V0) {
            encodes_ = std::make_shared<DeltaSIMDEncode>();
        } else if (type == DeltaType::V1) {
            encodes_ = std::make_shared<DeltaSIMDEncodeV1>();
        } else if (type == DeltaType::V2) {
            encodes_ = std::make_shared<DeltaSIMDEncodeV2>();
        } else if (type == DeltaType::V3){
            encodes_ = std::make_shared<DeltaSIMDEncodeV3>();
        }
    }

    void initEncode(size_t bit_size, size_t vertex_size, DeltaType type = DeltaType::V0) {
        if (type == DeltaType::V0) {
            encodes_ = std::make_shared<DeltaSIMDEncode>(bit_size, vertex_size);
        } else if (type == DeltaType::V1) {
            encodes_ = std::make_shared<DeltaSIMDEncodeV1>(bit_size, vertex_size);
        } else if (type == DeltaType::V2) {
            encodes_ = std::make_shared<DeltaSIMDEncodeV2>(bit_size, vertex_size);
        } else if (type == DeltaType::V3){
            encodes_ = std::make_shared<DeltaSIMDEncodeV3>(bit_size, vertex_size);
        }
    }

};


#endif //VEND_DELTA_SIMD_VEND_H
