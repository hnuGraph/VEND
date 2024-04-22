
#include "vend/dir_hyb_vend.h"

#include <iostream>
#include <map>

#include "omp.h"

void Directed_Hyb_Vend::BuildEncoding() {
    uint32_t id = 1, max_degree = 0, degree, edge_filtered = 0, total_edge = 0;
    // represent the time that a vertex was cut

    // vert array ordered the vertex by degree , bin  keeps tracks of first vertex index which degree equals i  in vert
    // array pos array  mapping vertex id to index of vert array
    std::vector<uint32_t> degrees(VERTEX_SIZE + 1), pos(VERTEX_SIZE + 1), vert(VERTEX_SIZE + 1);

    // get degrees

    for (uint32_t id = 1; id < adjacency_list_->size(); ++id) {
        degree = adjacency_list_->at(id).size() + in_adj_list_->at(id).size();
        vert[id] = id;
        max_degree = std::max(degree, max_degree);
        degrees[id] = degree;
        total_edge += degree;
    }

    std::sort(vert.begin() + 1, vert.end(), [&](int lhs, int rhs) { return degrees[lhs] < degrees[rhs]; });
    std::vector<uint32_t> bin(max_degree + 1, 0);

    int j = 0;
    for (uint32_t i = 1; i < VERTEX_SIZE + 1; ++i) {
        degree = degrees[vert[i]];
        pos[vert[i]] = i;
        if (degree > j) {
            j = degree;
            bin[j] = i;
        }
    }
    j = 1;
    for (; j <= max_degree; ++j) {
        if (bin[j] == 0) {
            bin[j] = bin[j - 1] + 1;
            while (bin[++j] == 0) bin[j] = bin[j - 1];
        }
    }

    std::vector<bool> filtered(VERTEX_SIZE + 1, false);
    // k core
    int idx = 0, max_kcore = encodes_->GetIntSize();
    for (; idx < bin[max_kcore + 1]; ++idx) {
        std::vector<uint32_t> neighbors, in_neighbors;
        uint32_t vertex = vert[idx], neighbor_degree, neighbor_pos, first_bin_pos, first_bin;
        uint32_t vertex_degree = degrees[vertex];

        std::vector<uint32_t> adj;
        if (adjacency_list_) {
            adj = adjacency_list_->at(vertex);
        }
        for (auto neighbor : adj) {
            if (filtered[neighbor]) continue;
            neighbors.push_back(neighbor);
            neighbor_degree = degrees[neighbor];
            neighbor_pos = pos[neighbor];
            first_bin_pos = bin[neighbor_degree];
            first_bin = vert[first_bin_pos];
            if (vertex_degree >= neighbor_degree) continue;
            if (first_bin_pos != neighbor_pos) {
                // swap two elements in the pos and vert
                pos[neighbor] = first_bin_pos;
                pos[first_bin] = neighbor_pos;
                vert[neighbor_pos] = first_bin;
                vert[first_bin_pos] = neighbor;
            }
            bin[neighbor_degree] += 1;
            degrees[neighbor] -= 1;
        }
        assert(in_adj_list_ != nullptr);
        for (auto neighbor : in_adj_list_->at(vertex)) {
            if (filtered[neighbor]) continue;
            in_neighbors.push_back(neighbor);
            neighbor_degree = degrees[neighbor];
            neighbor_pos = pos[neighbor];
            first_bin_pos = bin[neighbor_degree];
            first_bin = vert[first_bin_pos];
            if (vertex_degree >= neighbor_degree) continue;
            if (first_bin_pos != neighbor_pos) {
                // swap two elements in the pos and vert
                pos[neighbor] = first_bin_pos;
                pos[first_bin] = neighbor_pos;
                vert[neighbor_pos] = first_bin;
                vert[first_bin_pos] = neighbor;
            }
            bin[neighbor_degree] += 1;
            degrees[neighbor] -= 1;
        }
#if VERIFY == 1
        if (vertex == 107158 || vertex == 311754) {
            std::cout << vertex << std::endl;
            for (auto v : neighbors) std::cout << v << ",";
            std::cout << std::endl;
            for (auto v : in_neighbors) std::cout << v << ",";
            std::cout << std::endl;
        }
#endif
        filtered[vertex] = true;
        encodes_->EncodeVertex(vertex, neighbors, in_neighbors);
        edge_filtered += neighbors.size();
    }

    std::cout << "k core finished : kcore size:" << idx << "\n";
    std::cout << "edge filtered : " << edge_filtered << " total edge: " << total_edge / 2
              << " ratio:" << ((double)edge_filtered / total_edge * 2) << std::endl;

#if VARY_K == 1
    return;
#endif

    //    omp_set_num_threads(30);
    //
    //    #pragma omp parallel for
    for (; idx < VERTEX_SIZE + 1; ++idx) {
        uint32_t vertex = vert[idx];
        std::vector<uint32_t> neighbors, in_neigh;

        for (auto &neighbor : adjacency_list_->at(vertex)) {
            if (!filtered[neighbor]) neighbors.push_back(neighbor);
        }
        for (auto &neighbor : in_adj_list_->at(vertex)) {
            if (!filtered[neighbor]) in_neigh.push_back(neighbor);
        }
#if VERIFY == 1
        if (vertex == 107158 || vertex == 311754) {
            std::cout << vertex << std::endl;
            for (auto v : neighbors) std::cout << v << ",";
            std::cout << std::endl;
            for (auto v : in_neigh) std::cout << v << ",";
            std::cout << std::endl;
        }
#endif
        // encode
        encodes_->EncodeVertex(vertex, neighbors, in_neigh);
    }
    std::cout << " encode finished"
              << "\n";
}