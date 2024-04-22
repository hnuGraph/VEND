#ifndef VEND_HYBRID_DELTA_VEND_H
#define VEND_HYBRID_DELTA_VEND_H

#include "vend/range_vend.h"
#include "hybrid_vend.h"
#include "encode/single/hybrid_delta_encode.h"
#include <map>


class HybridDeltaVend : public RangeVend {

public:
    HybridDeltaVend(std::shared_ptr<std::vector<std::vector<uint32_t >>> &adj_list, const std::string &encode_path,
                    std::shared_ptr<DbEngine> &db)
            : RangeVend(adj_list, encode_path, db) {
        Init(db);

    }

    HybridDeltaVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : RangeVend(encode_path, db) {}

    void Init(std::shared_ptr<DbEngine> &db) {
        encodes_ = nullptr;
       // encodes_ = std::make_shared<HybridDeltaEncode>();
        encodes_->SetDb(db);
    }

};


#endif //VEND_HYBRID_DELTA_VEND_H
