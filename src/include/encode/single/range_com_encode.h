#ifndef SRC_INCLUDE_ENCODE_SINGLE_RANGE_COM_ENCODE_H_
#define SRC_INCLUDE_ENCODE_SINGLE_RANGE_COM_ENCODE_H_

#include "compression.h"
#include "range_encode.h"

class RangeCompEncode : public RangeEncode {
   public:
    RangeCompEncode(int method) : RangeEncode() ,shift(32),max_bits(PER_ENCODE_BIT_SIZE-32){
        if (method == 0) {
            codec = std::make_shared<BitsComp>(PER_ENCODE_BIT_SIZE, VERTEX_BIT_SIZE,
                                               (PER_ENCODE_BIT_SIZE - 32) / VERTEX_BIT_SIZE);
        } else if (method == 1) {
            codec = std::make_shared<VbyteComp>(PER_ENCODE_BIT_SIZE);
        } else if (method == 2) {
            codec = std::make_shared<VbyteDeltaComp>(PER_ENCODE_BIT_SIZE);
        }
    }

    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override {
        auto &encode = encode_bitset_[vertex_id];
        encode->Clear();
        uint32_t bits_needed = codec->GetRequiredBits(neighbors);
        if (bits_needed <= max_bits) {
            encode->BlockSet(0, shift, true);
            codec->Encode(neighbors, encode, shift);
        } else {
            encode->BlockSet(0, shift, false);
            std::vector<uint32_t> nei_back = neighbors;
            nei_back.insert(nei_back.begin(), 0);
            nei_back.push_back(vertex_id_upper_);
            codec->DynamicEncode(nei_back, encode, shift);
        }
    };

    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override {
        auto &encode = encode_bitset_[vertex1];

        return codec->Find(vertex2, encode, shift, GetFlag(vertex1));
    };

    bool GetFlag(uint32_t vertex) override {
        return encode_bitset_[vertex]->BlockGet(0, shift);
    };

    // flag == true means fully encode
    void SetFlag(uint32_t vertex, bool flag) override{
        encode_bitset_[vertex]->BlockSet(0, shift, flag);
    };
    uint32_t GetBytesSize() {return max_bits; }


    std::shared_ptr<Compression> GetCodec() { return codec; }

    
   private:
    std::shared_ptr<Compression> codec;
    uint32_t max_bits;
    uint32_t shift;
};

#endif  // SRC_INCLUDE_ENCODE_SINGLE_RANGE_COM_ENCODE_H_
