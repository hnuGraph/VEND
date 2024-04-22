//===----------------------------------------------------------------------===//
//
//
//
// l
//
//
//
//===----------------------------------------------------------------------===//
#ifndef MAIN
#define MAIN

#include <iostream>

#include "CLI11.hpp"
#include "common/config.h"
#include "execution/build_execution.h"
#include "execution/delete_execution.h"
#include "execution/full_score_execution.h"
#include "execution/insert_execution.h"
#include "execution/memory_test.h"
#include "execution/query_execution.h"
#include "execution/random_query_execution.h"
#include "execution/random_score_execution.h"
#include "execution/score_execution.h"
#include "util/utils.h"
#include "vend/vend_factory.h"

int main(int argc, char **argv) {
    CLI::App app{"VEND"};
    std::string data_path, db_path, vend_prefix, output_path, pair_path_prefix;

    int db_type = 0;
    std::vector<int> is_skip;

    bool is_build = false, is_insert = false, is_delete = false, is_triangle = false, is_walk_query = false,
         is_random_query = false, is_full_score = false, is_random_score = false, is_walk_score = false,
         is_memory = false, test_hsimd = false, is_directed = false;

    //    0:NoVend 1:BF-int 2:BBF 3:CBF 4:Hash 5:Hybrid 6:BF-bit 7:Range 8:LBF 9:Count 14:HybridOpt

    std::vector<VendType> types{
        VendType::NoVend,      VendType::BloomFilterInt, VendType::BBloomFilter,   VendType::CBloomFilter,
        VendType::HashBit,     VendType::Hybrid,         VendType::BloomFilterBit, VendType::Range,
        VendType::Bloom,       VendType::Count,          VendType::HybridDelta,    VendType::SIMDDelta,
        VendType::SIMDDeltaV1, VendType::SIMDDeltaV2,    VendType::SIMDDeltaV3,    VendType::RangeBits,
        VendType::RangeVbyte,  VendType::RangeDelta,     VendType::DirectHybrid, VendType::DirectSIMD};

    int plan_nums = types.size();

    app.add_option("-d", data_path, " origin data which is stored in txt format ");
    app.add_option("-b", db_path, "database path for  data graph")->required();
    app.add_option("-v", vend_prefix, "prefix of path for encode")->required();
    app.add_option("-o", output_path, "output path ")->required();
    app.add_option("-p", pair_path_prefix, "query edge path")->required();
    app.add_option("-t", db_type, "db type")->required();
    app.add_option("-k", K_SIZE, "k size ")->required();
    app.add_option("--vs", VERTEX_SIZE, "vertex size")->required();
    app.add_option("--ds", DEGREE, "degree size")->required();
    app.add_option("--th", test_hsimd, "test hybrid+simd");
    /*
     *  0: rocksdb 1:neo4j 2:dgraph 3:gstore
     * */
    app.add_option("--is", is_skip, "skip vend ");
    app.add_option("--ib", is_build, "build  encode for vend");
    app.add_option("--ii", is_insert, "insert experiment");
    app.add_option("--id", is_delete, "delete experiment");
    app.add_option("--iwq", is_walk_query, "query (random walk) experiment");
    app.add_option("--irq", is_random_query, "query (randomly) experiment");
    app.add_option("--ifs", is_full_score, "full score experiment");
    app.add_option("--irs", is_random_score, "score (choose edges randomly) experiment");
    app.add_option("--iws", is_walk_score, "score (choose edges by random walk) experiment");
    app.add_option("--it", is_triangle, "triangle  experiment");
    app.add_option("--im", is_memory, "memory  experiment");
    app.add_option("--idr", is_directed, "directed  experiment");

    CLI11_PARSE(app, argc, argv);

    std::string insert_pair_path = pair_path_prefix + "insert_pair.txt";
    std::string delete_pair_path = pair_path_prefix + "delete_pair.txtnik";
    std::string random_query_pair_path = pair_path_prefix + "random_query_pair.txt";
    std::string walk_query_pair_path = pair_path_prefix + "walk_query_pair_v2.txt";
    std::string random_score_pair_path = pair_path_prefix + "random_score_pair.txt";
    std::string walk_score_pair_path = pair_path_prefix + "walk_score_pair.txt";
    std::string triangle_pair_path = pair_path_prefix + "triangle_pair.txt";

    InitCommonVariables();
    InitHashConfig();

    std::vector<bool> skip(plan_nums, false);
    for (auto v : is_skip) skip[v] = true;



    if (is_build) {
        std::cout << "start building \n";
        for (int i = 0; i < plan_nums; ++i) {
            if (skip[i]) continue;
            BuildExecution *build_execution =
                new BuildExecution(data_path, db_path, vend_prefix, types[i], is_directed);
            build_execution->Execute();
            std::cout << " graph " << VEND_STRING[types[i]] << " build finished " << std::endl;
            delete build_execution;
        }
        std::cout << "build finished \n";
    }

    // insert experiment
    if (is_insert) {
        std::cout << "start inserting \n";
        for (int i = 1; i < plan_nums; ++i) {
            if (skip[i]) continue;
            InsertExecution *insert_execution =
                new InsertExecution(insert_pair_path, vend_prefix, output_path, db_path, types[i]);
            insert_execution->Execute();
            delete insert_execution;
        }
        std::cout << "insert experiment finished \n";
    }

    // delete experiment
    if (is_delete) {
        std::cout << "start deleting \n";
        for (int i = 1; i < plan_nums; ++i) {
            if (skip[i]) continue;
            DeleteExecution *delete_execution =
                new DeleteExecution(delete_pair_path, vend_prefix, output_path, db_path, types[i]);
            delete_execution->Execute();
            delete delete_execution;
        }
        std::cout << "delete experiment finished \n";
    }

    // random query experiment , return query time
    if (is_random_query) {
        std::cout << "start random querying (random ) \n";
        for (int i = 0; i < plan_nums; ++i) {
            if (skip[i]) continue;
            RandomQueryExecution *random_query_execution =
                new RandomQueryExecution(random_query_pair_path, vend_prefix, output_path, db_path, types[i], db_type);
            random_query_execution->Execute();
            delete random_query_execution;
        }
        std::cout << "query time experiment finished \n";
    }

    if (is_walk_query) {
        // walk query experiment , return query time
        if (!is_directed) {
            std::cout << "start querying ( walk ) \n";
            for (int i = 0; i < plan_nums; ++i) {
                if (skip[i]) continue;
                QueryExecution *query_execution =
                    new QueryExecution(walk_query_pair_path, vend_prefix, output_path, db_path, types[i], db_type);
                query_execution->Execute();
                delete query_execution;
            }
            if (test_hsimd) {
                HSIMD = 1;
                QueryExecution *query_execution = new QueryExecution(walk_query_pair_path, vend_prefix, output_path,
                                                                     db_path, VendType::Hybrid, db_type);
                query_execution->Execute();
                delete query_execution;

                query_execution = new QueryExecution(walk_query_pair_path, vend_prefix, output_path, db_path,
                                                     VendType::SIMDDeltaV3, db_type);
                query_execution->Execute();
                delete query_execution;
            }
            std::cout << "query time experiment finished \n";

        } else {
            std::cout << "start querying ( walk ) \n";
            for (int i = 0; i < plan_nums; ++i) {
                if (skip[i]) continue;
                QueryExecution *query_execution =
                    new QueryExecution(walk_query_pair_path, vend_prefix, output_path, db_path, types[i], db_type,is_directed);
                query_execution->Execute();
                delete query_execution;
            }
            std::cout << "query time experiment finished \n";

        }
    }
    if (is_random_score) {
        // random score
        std::cout << "start getting random scores\n";
        RandomScoreExecution *random_score_execution =
            new RandomScoreExecution(random_score_pair_path, vend_prefix, output_path, db_path);
        random_score_execution->Execute();
        delete random_score_execution;
        std::cout << "score experiment finished \n";
    }

    if (is_walk_score) {
        std::cout << "start getting scores (walk)\n";
        ScoreExecution *score_execution = new ScoreExecution(walk_score_pair_path, vend_prefix, output_path, db_path);
        score_execution->Execute();
        delete score_execution;
        std::cout << "score experiment finished \n";
    }

    if (is_full_score) {
        std::cout << "start getting scores\n";
        FullScoreExecution *full_execution = new FullScoreExecution(db_path, vend_prefix, output_path);
        full_execution->Execute();
        delete full_execution;
        std::cout << "score experiment finished \n";
    }

}

#endif  // MAIN
