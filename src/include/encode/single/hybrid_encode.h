//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_DYNAMIC_ENCODE_H
#define VEND_DYNAMIC_ENCODE_H


#include "math.h"
#include "assert.h"
#include "algorithm"

#include "encode/hash_count.h"
#include "util/bitset.h"
#include "common/config.h"
#include "encode/single/encode_bitset.h"
#include "dbengine/dbengine.h"
#include "common/config.h"
#include "util/utils.h"
#include <cstring>
#include <fstream>
#include <sstream>
#include <set>

/**
 *  encode format (bit)
 *  ------------------------------------------------------------------------------------------------------------------
 *  | FULL FLAG (1) | RANGE MODEL (2) | RANGE SIZE (log(k))|  RANGE ENCODE PART (RANGE SIZE * VERTEX BIT)| HASH ENCODE PART
 *  -------------------------------------------------------------------------------------------------------------------
 *
 *
 *  FULL FLAG (1 bit) :  1 represent fully encode, 0 for partially encode
 *  RANGE MODEL(2 bits) :  00  ——  range part contains minimal id (0)
 *                   01  ——  range part contains neither minial id or maximal id
 *                   11  ——  range part contains  maximal id (VERTEX_SIZE + 1)
 *  RANGE SIZE (log(k) bits) : represent how many integers for range encode
 *  VERTEX BIT  :  how many bits one vertex id takes      ( log(|V|))
 * */



static uint32_t BLOCK_BEGIN_INDEX = 3 + LOG_K;

class HybridEncode : public EncodeBitSet {
public:


    HybridEncode() : log_k_(LOG_K), v_bits_size_(VERTEX_BIT_SIZE), max_integer_size_(MAX_INTEGER_SIZE),
                     EncodeBitSet() {

    }

    PairType
    NEpairTest(uint32_t vertex1, const DecodeInfo &decode_info1, uint32_t vertex2, const DecodeInfo &decode_info2) {

        if (decode_info1.decodable && !decode_info2.decodable)
            return NonNeighborTest(vertex1, decode_info1, vertex2);
        else if (!decode_info1.decodable && decode_info2.decodable)
            return NonNeighborTest(vertex2, decode_info2, vertex1);
        else if(decode_info1.decodable && decode_info2.decodable){
            PairType type1 = NonNeighborTest(vertex1, decode_info1, vertex2);
            //Neighbor=1,NonNeighbor=2,Uncertain=4
            //type1==PairType::Neighbor
            if(type1&0x01)
                return PairType::Neighbor;
            else
                return NonNeighborTest(vertex2, decode_info2, vertex1);
        }else{
            PairType type1 = NonNeighborTest(vertex1, decode_info1, vertex2);
            //type1!=PairType::Uncertain
            if(type1&0x03)
                return type1;
            else
                return NonNeighborTest(vertex2, decode_info2, vertex1);
        }
    }

    /** F()
     *  @return true, if both hybrid functions is true
     * */
    PairType NEpairTest(uint32_t vertex1, uint32_t vertex2) override {
        bool encode_type1 = IsDecodable(vertex1), encode_type2 = IsDecodable(vertex2);
        if (encode_type1 < encode_type2) {
            std::swap(encode_type1, encode_type2);
            std::swap(vertex1, vertex2);
        }
        PairType type1 = NonNeighborTest(vertex1, vertex2);
        if (encode_type1 && !encode_type2) {
            return type1;
        } else {
            // both are Partical Encode or decodable
            PairType type2 = NonNeighborTest(vertex2, vertex1);
            return std::min(type1, type2);
        }
    };

    /**
     *  find vertex2 in the encode of vertex1
     * */
    PairType NonNeighborTest(uint32_t vertex1, const DecodeInfo &decode_info1, uint32_t vertex2);

    // f()
    PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) override;


    inline void Decode(uint32_t vertex, DecodeInfo &decode_info);


    /**
     *  dynamic encode
     *  call function TestNonPair to choose best range size;
     *
     * */
    void EncodeVertex(uint32_t vertex_id, std::vector<uint32_t> &neighbors) override;


    /**
     *  insert function
     *
     * */
    void InsertPair(uint32_t vertex1, uint32_t vertex2) override;

    /**
     *   choose highest score between vertex1 and vertex2
     * */
    void
    RestructChoose(uint32_t vertex1, std::vector<uint32_t> &neighbor1, uint32_t vertex2,
                   std::vector<uint32_t> &neighbor2);


    void DeletePair(uint32_t vertex1, uint32_t vertex2) override;

    // return false if vertex2 is not a neighbor
    bool RemoveNeighbor(uint32_t vertex1, uint32_t vertex2);

    /**
     *  dynamic choose for best range size
     *
     * */

    BitSet<PER_ENCODE_BIT_SIZE>
    DynamicChoose(uint32_t vertex, std::vector<uint32_t> &neighbors, uint32_t *count = nullptr);

    // compare score and choose the highest, and build the encode
    void
    ChooseHighest(const std::vector<uint32_t> &neighbors, uint32_t *max_score,
                  BitSet<PER_ENCODE_BIT_SIZE> *best_bitset,
                  HashCount *hash_count,
                  uint32_t block_size, uint32_t first_idx,
                  const BlockType &type);

    void SetNonDecodable(const std::vector<uint32_t> &neighbor_encode, const std::vector<uint32_t> &neighbor_left,
                         const BlockType &type,
                         BitSet<PER_ENCODE_BIT_SIZE> *bitset);

    /**
    *  count NePair for the encode
    *  @param  vertex_id
    *          encode : dynamic encoding for vertex_id
    *  @return  vertex range from 1 to vertex size, return how many certain non-pair within this encode
    *
    * */

    uint32_t NePairCount(uint32_t vertex_id, const BitSet<PER_ENCODE_BIT_SIZE> &encode);


    /**
     *  hash function for encode
     * @return : bit position to set
     * */
    uint32_t Hash(uint32_t vertex_id, uint32_t hash_begin) {
       return hash_begin == PER_ENCODE_BIT_SIZE ? 0 : vertex_id % (PER_ENCODE_BIT_SIZE - hash_begin) + hash_begin;

    };


    /**
     *  functions to get basic information of encode
     *  true if encode all neighbors by integers
     * */


    inline bool IsDecodable(uint32_t vertex) {
        return encode_bitset_[vertex].IsOne((uint32_t)0);
    }

    /**
     *  return encode integers
     * */
    std::vector<uint32_t> GetBlockInteger(uint32_t vertex);

    inline BlockType GetBlockModel(uint32_t vertex) {
        if (encode_bitset_[vertex].IsOne((uint32_t)1))
            return BlockType::RightMost;
        if (!encode_bitset_[vertex].IsOne((uint32_t)2))
            return BlockType::LeftMost;
        return BlockType::Middle;
    };

    inline BlockType GetBlockModel(BitSet<PER_ENCODE_BIT_SIZE> &encode) {
        if (encode.GetSecondBit())
            return BlockType::RightMost;
        if (encode.GetThridBit())
            return BlockType::Middle;
        return BlockType::LeftMost;
    };


    void SetBlockModel(BitSet<PER_ENCODE_BIT_SIZE> *bitset, BlockType type) {
        switch (type) {
            case BlockType::LeftMost:
                bitset->BlockSet(1, 2, 0);
                break;
            case BlockType::FullHash:
            case BlockType::Middle:
                bitset->BlockSet(1, 2, 1);
                break;
            case BlockType::RightMost:
                bitset->BlockSet(1, 2, 3);
                break;
        }
    }


    inline bool IsFull(uint32_t vertex) {
        return encode_bitset_[vertex].BlockGet(PER_ENCODE_BIT_SIZE - v_bits_size_, v_bits_size_) != 0;
    }

    inline bool IsFull(uint32_t vertex, uint32_t *index_not_full) {
        for (uint32_t index = 1;
             index <= PER_ENCODE_BIT_SIZE - v_bits_size_ + 1; index += v_bits_size_) {
            if (encode_bitset_[vertex].BlockGet(index, v_bits_size_) == 0)
                return false;
        }
        return true;
    }

    inline uint32_t GetBlockSize(uint32_t vertex) {
        return encode_bitset_[vertex].BlockGet(3, log_k_);
    }


    EncodeType GetEncodeType(uint32_t vertex) {
        if (!IsDecodable(vertex))
            return EncodeType::NonDecodable;
        if (IsFull(vertex))
            return EncodeType::Full;
        return EncodeType::UnFull;
    }

    std::vector<uint32_t> GetAllNeighbors(uint32_t vertex) {
        if (encode_bitset_[vertex].IsOne((uint32_t)0))
            return GetBlockInteger(vertex);
        return DbQuery(vertex);
    }

    inline uint32_t GetHashBegin(uint32_t vertex) {
        return GetBlockSize(vertex) * v_bits_size_ + 3 + log_k_;
    }

    uint32_t GetIntSize() { return max_integer_size_; }




    friend class TriangleCount;
    friend class Vend;
protected:


    uint32_t v_bits_size_;  // how many bits one vertex takes
    uint32_t log_k_;    // how many bits range size takes
    uint32_t max_integer_size_; // max integer number that fully encoding contains

};


#endif //VEND_DYNAMIC_ENCODE_H
