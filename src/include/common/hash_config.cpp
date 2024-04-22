#include "hash_config.h"

uint32_t BIT_HASH_BEST_NUMS=0;
uint32_t INT_HASH_BEST_NUMS=0;
uint32_t COUNT_HASH_BEST_NUMS=0;
uint32_t PREFIX_BIT_SIZE=0;
void InitHashConfig(){
    //hash config
    BIT_HASH_BEST_NUMS = log(2) * 64 * K_SIZE / DEGREE;
    INT_HASH_BEST_NUMS = UP_TO_ONE(log(2) * 2 * K_SIZE / DEGREE);
    COUNT_HASH_BEST_NUMS = UP_TO_ONE(log(2) * 2 * K_SIZE * (32 / ELEMENT_SIZE) / DEGREE);
    PREFIX_BIT_SIZE = LOG_K + 3;
    BIT_HASH_BEST_NUMS = 1;
}

