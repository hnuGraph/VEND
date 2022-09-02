//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//



#include "execution/score_execution.h"

#include <random>


void
ScoreExecution::RandomCreate(uint32_t vertex_size1, std::unordered_set<uint32_t> *vertex_list1, uint32_t vertex_size2,
                             std::unordered_set<uint32_t> *vertex_list2) {
    std::uniform_int_distribution<unsigned> u(1, VERTEX_SIZE + 1);
    std::default_random_engine e;
    e.seed(time(NULL));
    while (vertex_list1->size() < vertex_size1) {
        vertex_list1->insert(u(e) % VERTEX_SIZE + 1);
    }
    e.seed(time(NULL));
    while (vertex_list2->size() < vertex_size2) {
        uint32_t id = u(e) % VERTEX_SIZE + 1;
        if (vertex_list1->find(id) == vertex_list1->end())
            vertex_list2->insert(id);
    }
}


void ScoreExecution::Execute() {
    Init();
    uint32_t scores[PLAN_NUMS];
    double percent[PLAN_NUMS];
    memset(scores, 0, sizeof(uint32_t) * PLAN_NUMS);
    memset(percent, 0, sizeof(double) * PLAN_NUMS);

    std::ofstream output(output_path_, std::ios::app | std::ios::out);
    for (auto &pair:pair_list_) {
        for (int i = 0; i < PLAN_NUMS; ++i) {
            if (graphs_[i]->GetEdge(pair.first, pair.second)!=PairType::Uncertain)
                ++scores[i];
        }

    }
    for (int i = 0; i < PLAN_NUMS; ++i) {
        percent[i] = (double) scores[i] / list_size_ * 100;
        std::cout << "  Plan " << i << " : " << VEND_STRING[Vends[i]] << "  score : " << scores[i] << "("
                  << percent[i] << "%)" << "\n";
        output << scores[i] << ',' << percent[i] << ',';
    }
    output << "\n";
    output.close();
}


void ScoreExecution::CreateList(uint32_t vertex_size, std::vector<std::pair<uint32_t, uint32_t>> *vertex_list) {
    std::vector<uint32_t> random_array;
    for (int i = 1; i <= VERTEX_SIZE; ++i)
        random_array.push_back(i);

    std::shuffle(random_array.begin(), random_array.end(), std::mt19937(std::random_device()()));
    uint32_t vertex1, vertex2;
    for (auto v:random_array) {

        std::vector<uint32_t> neighbors;
        graphs_[0]->GetNeighbors(v, &neighbors);
        int neighbor_nums = neighbors.size();
        if (neighbor_nums <= 1)
            continue;
        for (int i = 0; i < neighbor_nums - 1; ++i) {
            if(rand()%2==0)
                continue;
            for (int j = i + 1; j < neighbor_nums; j+=2) {
                if(rand()%2==0)
                    continue;
                vertex_list->push_back({neighbors[i], neighbors[j]});
                if (vertex_list->size() >= vertex_size)
                    return;
            }
        }
    }
}

