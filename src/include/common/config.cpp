#include "config.h"

bool HSIMD= 0;
int HITED = 0;

int K_SIZE = 0;
// 1696415 1791489 3072441 en:6253897 uk:39454463 ar:22743881   gsh:988490691 cf:65608366
uint32_t VERTEX_SIZE = 0;  // how many vertices
// 13 28 76  en:43 uk:40  ar:49  gsh:52 cf:55
uint32_t DEGREE = 0;

// how many bits bloom filter for the graph takes
uint32_t B_FILTER_ENCODE_BITS;
int PER_ENCODE_BIT_SIZE;
// how many bits one vertex takes
uint32_t VERTEX_BIT_SIZE;

uint32_t MAX_K_CORE_SIZE;
// how many bits range size takes
uint32_t LOG_K;
uint32_t BLOCK_BEGIN_INDEX;
// max integer number that fully encoding contains
uint32_t MAX_INTEGER_SIZE;
int THREAD_NUMS = 1;
/*
 *  configurations for blocked bloom filter
 * */


uint32_t BLOCK_SIZE = 512;
uint32_t BLOCK_NUMS;
/*
 *
 *  configurations for counting bloom filter
 *
 * */
uint32_t ELEMENT_SIZE;
/*
 *  configurations for large graphs
 * */
bool IS_LARGE = false;
// write 500W vertices each batch
int WriteBatchSize = 1000000000;

uint32_t threshold = 0;
/*
 *  configurations for neo4j
 *
 * */
// #define  ROCKSDB
// #define NEO4J
std::string URL = "neo4j://neo4j:123456@localhost:7687";
std::string USER_NAME = "neo4j";
std::string PASSWORD = "123456";
std::string DGRAPH_RPC_URL = "localhost:9080";

void InitCommonVariables() {
    assert(K_SIZE != 0);
    assert(VERTEX_SIZE != 0);
    assert(DEGREE != 0);

    B_FILTER_ENCODE_BITS = VERTEX_SIZE * K_SIZE;
    PER_ENCODE_BIT_SIZE = K_SIZE * 32;
    VERTEX_BIT_SIZE = static_cast<uint32_t>(ceil(log2((double)VERTEX_SIZE)));
    MAX_K_CORE_SIZE = floor(static_cast<uint32_t>((PER_ENCODE_BIT_SIZE - 1) / VERTEX_BIT_SIZE));
    LOG_K = static_cast<uint32_t>(ceil(log2((double)(MAX_K_CORE_SIZE + 1))));
    MAX_INTEGER_SIZE = static_cast<uint32_t>((PER_ENCODE_BIT_SIZE - 3 - LOG_K) / VERTEX_BIT_SIZE);
    BLOCK_BEGIN_INDEX = 3 + LOG_K;
    BLOCK_NUMS = (uint64_t)B_FILTER_ENCODE_BITS * 32 / 512;
    ELEMENT_SIZE = static_cast<uint32_t>(ceil(log2((double)((double)DEGREE * VERTEX_SIZE / 2))));
}