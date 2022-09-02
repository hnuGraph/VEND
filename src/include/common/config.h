//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_CONFIG_H
#define VEND_CONFIG_H

#include "math.h"
#include "stdint.h"
#include <string>

static constexpr int K_SIZE =8;
// 1696415 1791489 3072441 en:6253897 uk:39454463 ar:22743881   gsh:988490691 cf:65608366
static constexpr uint32_t VERTEX_SIZE = 1696415;        // how many vertices
// 13 28 76  en:43 uk:40  ar:49  gsh:52 cf:55
static constexpr uint32_t DEGREE =13;

// how many bits bloom filter for the graph takes
static constexpr uint32_t B_FILTER_ENCODE_BITS = VERTEX_SIZE * K_SIZE;
static constexpr int PER_ENCODE_BIT_SIZE = K_SIZE * 32;
// how many bits one vertex takes
static uint32_t VERTEX_BIT_SIZE = static_cast<uint32_t>(ceil(log2((double) VERTEX_SIZE)));


static uint32_t MAX_K_CORE_SIZE = static_cast<uint32_t >((PER_ENCODE_BIT_SIZE - 1) / VERTEX_BIT_SIZE);
// how many bits range size takes
static uint32_t LOG_K = static_cast<uint32_t>(ceil(log2((double) MAX_K_CORE_SIZE)));

// max integer number that fully encoding contains
static uint32_t MAX_INTEGER_SIZE = static_cast<uint32_t >((PER_ENCODE_BIT_SIZE - 3 - LOG_K) / VERTEX_BIT_SIZE);

static constexpr int THREAD_NUMS = 1;
enum PairType {
    Neighbor = 1, NonNeighbor = 2, Uncertain = 4

};

/*
 *  configurations for blocked bloom filter
 * */

static constexpr uint32_t BLOCK_SIZE = 512;
static uint32_t BLOCK_NUMS = (uint64_t)B_FILTER_ENCODE_BITS *32/ 512;




/*
 *
 *  configurations for counting bloom filter
 *
 * */
static uint32_t ELEMENT_SIZE = static_cast<uint32_t>(ceil(log2((double) (DEGREE * VERTEX_SIZE / 2))));



/*
 *  configurations for large graphs
 * */
static bool IS_LARGE=false;
// write 500W vertices each batch
static constexpr int WriteBatchSize = 1000000000;



/*
 *  configurations for neo4j
 *
 * */
//#define  ROCKSDB
//#define NEO4J
#define DGRAPH
static std::string URL="neo4j://neo4j:123456@localhost:7687";
static std::string USER_NAME="neo4j";
static std::string PASSWORD="123456";
static std::string DGRAPH_RPC_URL="localhost:9080";
#endif //VEND_CONFIG_H
