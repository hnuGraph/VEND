#ifndef SRC_INCLUDE_VEND_DIRECTED_VEND_H_
#define SRC_INCLUDE_VEND_DIRECTED_VEND_H_

#include "encode/single/directed_hybrid.h"
#include "vend.h"

class Directed_Hyb_Vend : public Vend {
   public:
    Directed_Hyb_Vend() = default;

    Directed_Hyb_Vend(std::shared_ptr<std::vector<std::vector<uint32_t>>> &adj_list,
                      std::shared_ptr<std::vector<std::vector<uint32_t>>> &in_adj, const std::string &out_encode_path,
                      const std::string &in_encode_path, std::shared_ptr<DbEngine> &db)
        :  in_adj_list_(in_adj), in_encode_path_(in_encode_path),Vend(adj_list, out_encode_path, db) {
        Init(db);
    }
    Directed_Hyb_Vend(const std::string &encode_path, std::shared_ptr<DbEngine> &db) : Vend(encode_path, db) {
        adjacency_list_ = nullptr;
        Init(db);
    }

    virtual void Init(std::shared_ptr<DbEngine> &db) {
        encodes_ = std::make_shared<DirectedHybrid>();
        encodes_->SetDb(db);
    }

    void BuildEncoding() override;
    void LoadEncode() override {
        encodes_->LoadFromDb(encode_path_, true);
        encodes_->LoadFromDb(in_encode_path_, false);
    };
    void EncodePersistent() override {
        encodes_->EncodePersistent(encode_path_, true);
        encodes_->EncodePersistent(in_encode_path_, false);
    };

   protected:
    struct K_Node {
        uint32_t id;
        uint32_t neighbors_num;

        bool operator==(const K_Node &rhs) const { return id == rhs.id && neighbors_num == rhs.neighbors_num; }

        bool operator<(const K_Node &rhs) const {
            if (this->neighbors_num == rhs.neighbors_num) return id < rhs.id;
            return this->neighbors_num < rhs.neighbors_num;
        }
    };
    std::shared_ptr<std::vector<std::vector<uint32_t>>> in_adj_list_;
    std::string in_encode_path_;
};

#endif  // SRC_INCLUDE_VEND_DIRECTED_VEND_H_
