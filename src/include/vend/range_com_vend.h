#ifndef SRC_INCLUDE_VEND_RANGE_COM_VEND_H_
#define SRC_INCLUDE_VEND_RANGE_COM_VEND_H_

#include <map>
#include "omp.h"
#include "util/timer.h"
#include "vend/range_vend.h"
#include "encode/single/range_com_encode.h"
class RangeCompVend : public RangeVend {
   public:
    RangeCompVend(std::shared_ptr<std::vector<std::vector<uint32_t>>> &adj_list, const std::string &encode_path,
                  std::shared_ptr<DbEngine> &db, int compress)
        : RangeVend(adj_list, encode_path, db) {
        Init(db, compress);
    }

    RangeCompVend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : RangeVend(encode_path, db) {}

    void Init(std::shared_ptr<DbEngine> &db, int compress) {
        encodes_ = nullptr;
        encodes_ = std::make_shared<RangeCompEncode>(compress);
        encodes_->SetDb(db);
    }

    void BuildEncoding() override;
};
#endif  // SRC_INCLUDE_VEND_RANGE_COM_VEND_H_
