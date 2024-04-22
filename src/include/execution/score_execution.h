//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VEND_SCORE_EXECUTION_H
#define VEND_SCORE_EXECUTION_H

#include <algorithm>
#include <unordered_set>

#include "execution/pair_list.h"
#include "graph/graph.h"
#include "omp.h"
#include "vend/bf_bit_vend.h"
#include "vend/bf_int_vend.h"
#include "vend/bit_hash_vend.h"
#include "vend/hybrid_vend.h"
#include "vend/range_vend.h"

static constexpr uint32_t VERTETX_TEST_SIZE1 = 100000;
static constexpr uint32_t VERTETX_TEST_SIZE2 = 10000;
static constexpr uint32_t PAIR_SIZE = VERTETX_TEST_SIZE1 * VERTETX_TEST_SIZE2;
static constexpr uint32_t BATH_SIZE = 10000000;
static constexpr uint32_t SCORE_PAIR_SIZE = 100000000;
static constexpr int PLAN_NUMS = 9;
static VendType Vends[9] = {VendType::SIMDDeltaV3,  VendType::BBloomFilter,   VendType::Hybrid,
                            VendType::CBloomFilter, VendType::BloomFilterBit, VendType::BloomFilterInt,
                            VendType::Range,        VendType::Bloom,          VendType::HashBit};

class ScoreExecution : public PairList {
   public:
    ScoreExecution(std::string pair_path, std::string vend_prefix, std::string output_path, std::string db_path)
        : PairList(pair_path, output_path, SCORE_PAIR_SIZE) {
        vend_paths_[0] = VendFactory::GetVendPath(vend_prefix, Vends[0]);
        graphs_[0] = std::make_shared<Graph>(db_path, vend_paths_[0], Vends[0]);
        graphs_[0]->Init();
        for (int i = 1; i < PLAN_NUMS; ++i) {
            vend_paths_[i] = VendFactory::GetVendPath(vend_prefix, Vends[i]);
            graphs_[i] = std::make_shared<Graph>("", vend_paths_[i], Vends[i]);
            graphs_[i]->Init();
        }
    };

    void Execute() override;

    void CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) override;

    void RandomCreate(uint32_t vertex_size1, std::unordered_set<uint32_t> *vertex_list1, uint32_t vertex_size2,
                      std::unordered_set<uint32_t> *vertex_list2);

   private:
    std::shared_ptr<Graph> graphs_[PLAN_NUMS];
    std::string vend_paths_[PLAN_NUMS];
};

#endif  // VEND_SCORE_EXECUTION_H
