#include "vend/range_com_vend.h"

#include <unordered_set>

#define METRIC
void RangeCompVend::BuildEncoding() {
    Timer t1, t2;
    t1.StartTimer();
#if UNDIRECTED == 1
    uint32_t edge_filtered = 0, max_degree = 0, vertices_filtered = 0, total_edge = 0;
    std::vector<bool> filtered(VERTEX_SIZE + 1, false);
    std::vector<uint32_t> encode_list;
    auto codec = reinterpret_cast<RangeCompEncode *>(encodes_.get())->GetCodec();
    // DeltaSIMDCodec *codec = reinterpret_cast<DeltaSIMDEncode *>(encodes_.get())->GetCodec();
    uint32_t max_bits_len = encodes_->GetBytesSize();
    for (uint32_t id = 1; id < adjacency_list_->size(); ++id) {
        int len = codec->GetRequiredBits(adjacency_list_->at(id));
        if (len <= max_bits_len) {
            encode_list.push_back(id);
        }
        max_degree = std::max(max_degree, (uint32_t)adjacency_list_->at(id).size());
#ifdef METRIC
        total_edge += adjacency_list_->at(id).size();
#endif
    }
    // first round
    std::unordered_set<uint32_t> vertices_related;
    for (int idx = 0; idx < encode_list.size(); ++idx) {
        uint32_t vertex = encode_list[idx];
        vertices_related.insert(adjacency_list_->at(vertex).begin(), adjacency_list_->at(vertex).end());
        if (vertex == 1616642 || vertex == 1225887) {
            std::cout << "line 33: " << vertex << " neighbors:";
            for (auto v : adjacency_list_->at(vertex)) {
                std::cout << v << ",";
            }
            std::cout << std::endl;
        }
        encodes_->EncodeVertex(vertex, adjacency_list_->at(vertex));

        filtered[vertex] = true;
#ifdef METRIC
        edge_filtered += adjacency_list_->at(vertex).size();
        vertices_filtered += 1;
#endif
    }

    while (!vertices_related.empty()) {
        std::unordered_set<uint32_t> next_list;
        for (uint32_t vertex : vertices_related) {
            std::vector<uint32_t> neighbors;
            if (filtered[vertex]) continue;
            for (auto neighbor : adjacency_list_->at(vertex)) {
                if (filtered[neighbor]) continue;
                neighbors.push_back(neighbor);
            }
            if (codec->GetRequiredBits(neighbors) <= max_bits_len) {
                if (vertex == 1616642 || vertex == 1225887) {
                    std::cout << "line 59: " << vertex << " neighbors:";
                    for (auto v : neighbors) {
                        std::cout << v << ",";
                    }
                    std::cout << std::endl;
                }
                encodes_->EncodeVertex(vertex, neighbors);
                filtered[vertex] = true;
#ifdef METRIC
                edge_filtered += neighbors.size();
                vertices_filtered += 1;
#endif
                next_list.insert(neighbors.begin(), neighbors.end());
            }
        }
        vertices_related = next_list;
    }

#ifdef METRIC
    std::cout << "k core finished : kcore size:" << vertices_filtered << "\n";
    std::cout << "edge filtered : " << edge_filtered << " total edge: " << total_edge / 2 << std::endl;
#endif
    t1.StopTimer();
    std::cout << " k-core time :" << t1.CountSeconds() << " s" << std::endl;
    t2.StartTimer();
#if VARY_K == 1
    return;
#endif

#if THRESHOLD
    threshold = 0.1 * max_degree;
    std::cout << " threshold: " << threshold << std::endl;
#endif

#if MULTI_THREAD
    omp_set_num_threads(10);
#pragma omp parallel for
#endif
    for (int vertex = 1; vertex < VERTEX_SIZE + 1; ++vertex) {
        if (filtered[vertex]) continue;

        std::vector<uint32_t> neighbors;
        for (auto &neighbor : adjacency_list_->at(vertex)) {
            if (!filtered[neighbor]) neighbors.push_back(neighbor);
        }
        encodes_->EncodeVertex(vertex, neighbors);
    }
    std::cout << " encode finished"
              << "\n";

#endif

#if UNDIRECTED == 0
    for (int idx = 0; idx < VERTEX_SIZE + 1; ++idx) {
        // encode
        encodes_->EncodeVertex(idx, adjacency_list_->at(idx));
    }
    std::cout << " encode finished"
              << "\n";

#endif

    t2.StopTimer();
    std::cout << " after k-core time :" << t2.CountSeconds() << " s" << std::endl;
}
