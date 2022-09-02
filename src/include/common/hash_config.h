//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_HASH_CONFIG_H
#define VEND_HASH_CONFIG_H

#include "common/config.h"
#include <vector>

#define UP_TO_ONE(x) x<=1?1:x
const uint32_t BIT_HASH_BEST_NUMS = log(2) * 64 * K_SIZE / DEGREE;
const uint32_t INT_HASH_BEST_NUMS = UP_TO_ONE(log(2) * 2 * K_SIZE / DEGREE);
const uint32_t COUNT_HASH_BEST_NUMS=UP_TO_ONE(log(2) * 2 * K_SIZE*(32/ELEMENT_SIZE) / DEGREE);
static const std::vector<uint32_t> HASH_PARAM1 = {
        9191891, 1635947, 6893911, 2908361, 3877817, 5170427,
        2703929, 9961873, 8702593, 6795587, 3838903, 1609511,
        3838903, 3712831, 1609511, 2717593, 1434539, 7085971,
        2339639, 1316767, 2734321, 3274367, 3911749, 1300769,
        9871111, 1141061, 5974999, 2021081, 1123597, 9063707,
        5727739, 5218243, 2209747, 7445869, 7982327, 3739793,
        1803349, 9859697, 4833379, 7049219, 7919623, 7731103,
        4184641, 2627057, 3807691, 3044791, 7665613, 5529673,
        7904591, 9065003, 9416543, 9616279, 8034209, 8383213,
        2609339, 7587001, 2895359, 7402631, 1797161, 7542971
};
static const std::vector<uint32_t> HASH_PARAM2 = {
        1282517, 6659501, 7581941, 4050433, 7283071, 5793467,
        9692027, 4554733, 3448817, 9099731, 2797793, 9255167,
        4282727, 8699567, 9842081, 6976267, 7877509, 2042107,
        4404161, 4295609, 3455113, 2989673, 9410483, 7036109,
        9559609, 5945413, 4152943, 8736919, 8832641, 3201251,
        7251823, 3375391, 8783759, 1203077, 6746209, 8190113,
        3458687, 9673919, 2210567, 2698727, 5789227, 9444763,
        2628107, 4365827, 6898621, 4875253, 9633563, 4943333,
        4214123, 6162077, 6528673, 3763867, 6954799, 3726403,
        4801627, 1597961, 4228381, 9210757, 5584463, 8847067,
};

#endif //VEND_HASH_CONFIG_H
