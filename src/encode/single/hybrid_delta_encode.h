//#ifndef VEND_HYBRID_DELTA_ENCODE_H
//#define VEND_HYBRID_DELTA_ENCODE_H
//
//#include "encode/single/hybrid_encode.h"
//#include "encode/codec/varint_codec.h"
//#include <unordered_set>
//
//class HybridDeltaEncode : public HybridEncode {
//
//public:
//    HybridDeltaEncode() {
//        log_k_ = 5;
//        info_bytes_length_ = 1;
//        max_bytes_left_ = PER_ENCODE_BIT_SIZE / 8 - 1;
//        this->codec_ = std::make_unique<VarintCodec>(true);
//    }
//
//    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override;
//
//    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override;
//
//
//    void
//    DynamicChoose(BitSet<PER_ENCODE_BIT_SIZE> &encode,std::vector<uint32_t> &neighbors,std::vector<DeltaInfo> &delta_infos, uint32_t &score);
//
//
//
//private:
//    struct BlockInfo {
//
//        int begin;
//        // how many integer to write
//        int block_size;
//        BlockType block_type;
//    };
//
//    // return true if update score
//    bool ScoreCount(BlockInfo &block_info, std::vector<uint32_t> &neighbors,
//                    uint32_t hash_bits, uint32_t &score);
//
//    bool FullHashScoreCount(std::vector<uint32_t> &neighbors,
//                            uint32_t hash_bits, uint32_t &score);
//
//    void
//    ConstructEncode(const BlockInfo &block_info,std::vector<uint32_t> &neighbors,
//                    BitSet<PER_ENCODE_BIT_SIZE> &encode);
//
//    PairType FindDecodable(BitSet<PER_ENCODE_BIT_SIZE> &encode,uint32_t vertex);
//
//    PairType FindNonDecodable(BitSet<PER_ENCODE_BIT_SIZE> &encode,uint32_t vertex);
//
//    uint32_t max_bytes_left_;
//    size_t info_bytes_length_;
//    std::unique_ptr<VarintCodec> codec_;
//
//
//};
//
//
//#endif //VEND_HYBRID_DELTA_ENCODE_H
