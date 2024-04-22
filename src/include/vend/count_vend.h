
#ifndef VEND_COUNT_VEND_H
#define VEND_COUNT_VEND_H

#include "vend/range_vend.h"
#include "encode/single/count_encode.h"
#include <map>


class CountVend : public RangeVend {

public:
    CountVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
            std::shared_ptr<DbEngine> &db)
    : RangeVend(adj_list, encode_path, db) {
        Init(db);
    }

    CountVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : RangeVend(encode_path, db) {
        Init(db);
    }

    void Init(std::shared_ptr<DbEngine> &db) {
        encodes_ = nullptr;
        encodes_ = std::make_shared<CountEncode>();
        encodes_->SetDb(db);
    }

};


#endif //VEND_COUNT_VEND_H
