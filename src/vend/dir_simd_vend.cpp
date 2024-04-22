
#include "vend/dir_simd_vend.h"

#include <iostream>
#include <map>

#include "omp.h"
#include "util/timer.h"
void DirSIMDVend::BuildEncoding() {
    Timer t1, t2;
    t1.StartTimer();
    uint32_t edge_filtered = 0, vertices_filtered = 0, total_edge = 0;
    std::vector<bool> filtered(VERTEX_SIZE + 1, false);
    std::vector<uint32_t> encode_list;
    DeltaSIMDCodec *codec = reinterpret_cast<DeltaSIMDEncode *>(encodes_.get())->GetCodec();
    uint32_t max_bytes_len = encodes_->GetBytesSize();
    for (uint32_t id = 1; id < adjacency_list_->size(); ++id) {
        int len = codec->GetRequiredBytes(adjacency_list_->at(id));
        len += codec->GetRequiredBytes(in_adj_list_->at(id));
        if (len <= max_bytes_len) {
            encode_list.push_back(id);
        }
#ifdef METRIC
        total_edge += adjacency_list_->at(id).size() + in_adj_list_->at(id).size();
#endif
    }
    // first round
    std::unordered_set<uint32_t> vertices_related;
    for (int idx = 0; idx < encode_list.size(); ++idx) {
        uint32_t vertex = encode_list[idx];
        vertices_related.insert(adjacency_list_->at(vertex).begin(), adjacency_list_->at(vertex).end());
        vertices_related.insert(in_adj_list_->at(vertex).begin(), in_adj_list_->at(vertex).end());

        encodes_->EncodeVertex(vertex, adjacency_list_->at(vertex), in_adj_list_->at(vertex));
        filtered[vertex] = true;
#ifdef METRIC
        edge_filtered += adjacency_list_->at(vertex).size();
        for (auto v : adjacency_list_->at(vertex)) {
            if (filtered[v]) edge_filtered--;
        }
        vertices_filtered += 1;
#endif
    }

    while (!vertices_related.empty()) {
        std::unordered_set<uint32_t> next_list;
        for (uint32_t vertex : vertices_related) {
            std::vector<uint32_t> neighbors, in_neighs;
            if (filtered[vertex]) continue;
            for (auto neighbor : adjacency_list_->at(vertex)) {
                if (filtered[neighbor]) continue;
                neighbors.push_back(neighbor);
            }
            for (auto in_neigh : in_adj_list_->at(vertex)) {
                if (filtered[in_neigh]) continue;
                in_neighs.push_back(in_neigh);
            }

            int len = codec->GetRequiredBytes(neighbors) + codec->GetRequiredBytes(in_neighs);
            if (len <= max_bytes_len) {
                encodes_->EncodeVertex(vertex, neighbors, in_neighs);
                filtered[vertex] = true;
#ifdef METRIC
                edge_filtered += neighbors.size();
                vertices_filtered += 1;
#endif
                next_list.insert(neighbors.begin(), neighbors.end());
                next_list.insert(in_neighs.begin(), in_neighs.end());
            }
        }
        vertices_related = next_list;
    }

#ifdef METRIC
    std::cout << "k core finished : kcore size:" << vertices_filtered << "\n";
    std::cout << "edge filtered : " << edge_filtered << " total edge: " << total_edge / 2
              << " ratio: " << ((double)edge_filtered / total_edge * 2) << std::endl;
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

        std::vector<uint32_t> neighbors, in_neighs;
        for (auto &neighbor : adjacency_list_->at(vertex)) {
            if (!filtered[neighbor]) neighbors.push_back(neighbor);
        }
        for (auto &in_neigh : in_adj_list_->at(vertex)) {
            if (!filtered[in_neigh]) in_neighs.push_back(in_neigh);
        }

        encodes_->EncodeVertex(vertex, neighbors, in_neighs);
    }
    std::cout << " encode finished"
              << "\n";

    t2.StopTimer();
    std::cout << " after k-core time :" << t2.CountSeconds() << " s" << std::endl;
}