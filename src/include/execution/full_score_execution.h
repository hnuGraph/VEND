//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VEND_FULL_SCORE_EXECUTION_H1
#define VEND_FULL_SCORE_EXECUTION_H1

static constexpr uint32_t SCORE_BATCH = 1000000;
static constexpr uint64_t TOTAL_PAIRS_SIZE = 100000000;

class FullScoreExecution {

public:
    FullScoreExecution(std::string db_path, std::string vend_prefix, std::string output_path) : output_path_(
            output_path + ".score") {

        vend_paths_[0] = VendFactory::GetVendPath(vend_prefix, Vends[0]);
        graphs_[0] = std::make_shared<Graph>("", vend_paths_[0], Vends[0]);
        graphs_[0]->Init();
        for (int i = 1; i < PLAN_NUMS; ++i) {
            vend_paths_[i] = VendFactory::GetVendPath(vend_prefix, Vends[i]);
            graphs_[i] = std::make_shared<Graph>("", vend_paths_[i], Vends[i]);
            graphs_[i]->Init();
        }
    }

    void Execute() {
        uint64_t scores[PLAN_NUMS];
        double percent[PLAN_NUMS];
        memset(scores, 0, sizeof(uint64_t) * PLAN_NUMS);
        memset(percent, 0, sizeof(double) * PLAN_NUMS);

        std::ofstream output(output_path_, std::ios::app | std::ios::out);
        uint64_t times = 0;
        std::vector<uint32_t> random_array(VERTEX_SIZE);
        for (uint32_t i = 1; i <= VERTEX_SIZE; ++i)
            random_array[i - 1] = i;
        std::uniform_int_distribution<unsigned> u;
        std::default_random_engine e(time(NULL));
        std::shuffle(random_array.begin(), random_array.end(), e);
        while (times <= TOTAL_PAIRS_SIZE) {

            int random1 = u(e) % VERTEX_SIZE + 1;
            int random2 = u(e) % VERTEX_SIZE + 1;
            if (random1 == random2)
                continue;
            ++times;
            for (int n = 0; n < PLAN_NUMS; ++n) {
                if (graphs_[n]->GetEdge(random1, random2) != PairType::Uncertain)
                    ++scores[n];
            }
            if (times % SCORE_BATCH == 0) {
                srand((unsigned int) time(NULL));
                std::cout << "after " << times << " times" << std::endl;
                for (int n = 0; n < PLAN_NUMS; ++n) {
                    percent[n] = (double) scores[n] / times * 100;
                    std::cout << "  Plan " << n << " : " << VEND_STRING[Vends[n]] << "  score : " << scores[n]
                              << "("
                              << percent[n] << "%)" << std::endl;
                    output << scores[n] << ',' << percent[n] << ',';
                }
                output << "\n";
                if (times >= TOTAL_PAIRS_SIZE) {
                    output.close();
                    return;
                }
            }
        }

    }


private:

    std::shared_ptr<Graph> graphs_[PLAN_NUMS];
    std::string vend_paths_[PLAN_NUMS];
    std::string output_path_;


};


#endif //VEND_FULL_SCORE_EXECUTION_H1
