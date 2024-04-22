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
#include "assert.h"
#include "hash_config.h"


#define DB_QUERY 1

#define VARY_K 0

#define IN_MEMORY 1
#define SAVE_LIST 1
#define THRESHOLD 0
#define FALSE_OPSITIVE 1

#define UNDIRECTED 1
#define SAVE_ENCODE 1

#define MULTI_THREAD 0 


#define VERIFY 0

// hybrid +simd
//#define HSIMD 0


extern bool HSIMD;

extern int HITED;

extern int K_SIZE;
// 1696415 1791489 3072441 en:6253897 uk:39454463 ar:22743881   gsh:988490691 cf:65608366
extern uint32_t VERTEX_SIZE;        // how many vertices
// 13 28 76  en:43 uk:40  ar:49  gsh:52 cf:55
extern uint32_t DEGREE;
// how many bits bloom filter for the graph takes
extern uint32_t B_FILTER_ENCODE_BITS;
extern int PER_ENCODE_BIT_SIZE;
// how many bits one vertex takes
extern uint32_t VERTEX_BIT_SIZE;
extern uint32_t MAX_K_CORE_SIZE;
// how many bits range size takes
extern uint32_t LOG_K;
// max integer number that fully encoding contains
extern uint32_t MAX_INTEGER_SIZE;
extern int THREAD_NUMS;
extern uint32_t BLOCK_BEGIN_INDEX;
/*
 *  configurations for blocked bloom filter
 * */


extern uint32_t BLOCK_SIZE;
extern uint32_t BLOCK_NUMS;
/*
 *
 *  configurations for counting bloom filter
 *
 * */
extern uint32_t ELEMENT_SIZE;
/*
 *  configurations for large graphs
 * */
extern bool IS_LARGE;
// write 500W vertices each batch
extern int WriteBatchSize;
/*
 *  configurations for neo4j
 *
 * */
//#define  ROCKSDB
//#define NEO4J

extern uint32_t threshold;

#define DGRAPH
extern std::string URL;
extern std::string USER_NAME;
extern std::string PASSWORD;
extern std::string DGRAPH_RPC_URL;
enum PairType {
    Neighbor = 1, NonNeighbor = 2, Uncertain = 4

};
void InitCommonVariables();
#endif //VEND_CONFIG_H
