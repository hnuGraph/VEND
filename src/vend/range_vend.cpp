
// //===----------------------------------------------------------------------===//
// //
// //
// //
// //
// //
// //
// //
// //===----------------------------------------------------------------------===//

// #include "vend/range_vend.h"

// #include <iostream>
// #include <map>

// #include "omp.h"

// void RangeVend::BuildEncoding() {
//     uint32_t id = 1, max_degree = 0, degree, edge_filtered = 0, total_edge = 0;
//     // represent the time that a vertex was cut

//     // vert array ordered the vertex by degree , bin  keeps tracks of first vertex index which degree equals i  in vert
//     // array pos array  mapping vertex id to index of vert array
//     std::vector<uint32_t> degrees(VERTEX_SIZE + 1), pos(VERTEX_SIZE + 1), vert(VERTEX_SIZE + 1);

//     // get degrees
//     if (adjacency_list_) {
//         for (uint32_t id = 1; id < adjacency_list_->size(); ++id) {
//             degree = adjacency_list_->at(id).size();
//             vert[id] = id;
//             max_degree = std::max(degree, max_degree);
//             degrees[id] = degree;
//             total_edge += degree;
//         }
//     } else {
//         data_db_->InitIter();
//         uint32_t vertex1;
//         std::vector<uint32_t> neighbors;
//         while (data_db_->Next(&vertex1, &neighbors)) {
//             degree = neighbors.size();
//             vert[vertex1] = vertex1;
//             max_degree = std::max(degree, max_degree);
//             degrees[vertex1] = degree;
//             total_edge += degree;
//         }
//     }

//     std::sort(vert.begin() + 1, vert.end(), [&](int lhs, int rhs) { return degrees[lhs] < degrees[rhs]; });
//     std::vector<uint32_t> bin(max_degree + 1, 0);

//     int j = 0;
//     for (uint32_t i = 1; i < VERTEX_SIZE + 1; ++i) {
//         degree = degrees[vert[i]];
//         pos[vert[i]] = i;
//         if (degree > j) {
//             j = degree;
//             bin[j] = i;
//         }
//     }
//     j = 1;
//     for (; j <= max_degree; ++j) {
//         if (bin[j] == 0) {
//             bin[j] = bin[j - 1] + 1;
//             while (bin[++j] == 0) bin[j] = bin[j - 1];
//         }
//     }

//     std::vector<bool> filtered(VERTEX_SIZE + 1, false);
//     // k core
//     int idx = 0, max_kcore = encodes_->GetIntSize();

//     for (; idx < bin[max_kcore + 1]; ++idx) {
//         std::vector<uint32_t> neighbors;
//         uint32_t vertex = vert[idx], neighbor_degree, neighbor_pos, first_bin_pos, first_bin;
//         uint32_t vertex_degree = degrees[vertex];

//         std::vector<uint32_t> adj;
//         if (adjacency_list_) {
//             adj = adjacency_list_->at(vertex);
//         } else {
//             data_db_->Get(vertex, adj);
//         }

//         for (auto neighbor : adj) {
//             if (filtered[neighbor]) continue;
//             neighbors.push_back(neighbor);
//             neighbor_degree = degrees[neighbor];
//             neighbor_pos = pos[neighbor];
//             first_bin_pos = bin[neighbor_degree];
//             first_bin = vert[first_bin_pos];
//             if (vertex_degree >= neighbor_degree) continue;
//             if (first_bin_pos != neighbor_pos) {
//                 // swap two elements in the pos and vert
//                 pos[neighbor] = first_bin_pos;
//                 pos[first_bin] = neighbor_pos;
//                 vert[neighbor_pos] = first_bin;
//                 vert[first_bin_pos] = neighbor;
//             }
//             bin[neighbor_degree] += 1;
//             degrees[neighbor] -= 1;
//         }
//         filtered[vertex] = true;
//         //encodes_->EncodeVertex(vertex, neighbors);
//         edge_filtered += neighbors.size();
//     }

//     std::cout << "k core finished : kcore size:" << idx << "\n";
//     std::cout << "edge filtered : " << edge_filtered << " total edge: " << total_edge << std::endl;



// #if THRESHOLD
//     threshold = 0.1 * max_degree;
//     std::cout << " threshold: " << threshold << std::endl;
// #endif

// #ifdef VARY_K
//     return;
// #endif


//     omp_set_num_threads(10);
// #pragma omp parallel for
//     for (uint32_t i = idx; i < VERTEX_SIZE + 1; ++i) {
//         uint32_t vertex = vert[i];
//         std::vector<uint32_t> neighbors;

//         if (adjacency_list_) {
//             for (auto &neighbor : adjacency_list_->at(vertex)) {
//                 if (!filtered[neighbor]) neighbors.push_back(neighbor);
//             }
//         } else {
//             data_db_->Get(vertex, neighbors);
//             for (auto neighbor : neighbors) {
//                 if (!filtered[neighbor]) neighbors.push_back(neighbor);
//             }
//         }
//         // encode
//         encodes_->EncodeVertex(vertex, neighbors);
//     }
//     std::cout << " encode finished"
//               << "\n";
// }

//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//


#include "vend/range_vend.h"
#include <iostream>
#include <map>
#include "omp.h"


void RangeVend::BuildEncoding() {

#if UNDIRECTED==1
    uint32_t id = 1, max_degree = 0, degree, edge_filtered = 0, total_edge = 0;
    // represent the time that a vertex was cut

    // vert array ordered the vertex by degree , bin  keeps tracks of first vertex index which degree equals i  in vert array
    // pos array  mapping vertex id to index of vert array
    std::vector<uint32_t> degrees(VERTEX_SIZE + 1), pos(VERTEX_SIZE + 1), vert(VERTEX_SIZE + 1);

    // get degrees
    if (adjacency_list_) {
        for (uint32_t id = 1; id < adjacency_list_->size(); ++id) {
            degree = adjacency_list_->at(id).size();
            vert[id] = id;
            max_degree = std::max(degree, max_degree);
            degrees[id] = degree;
            total_edge += degree;
        }
    } else {
        data_db_->InitIter();
        uint32_t vertex1;
        std::vector<uint32_t> neighbors;
        while (data_db_->Next(&vertex1, &neighbors)) {
            degree = neighbors.size();
            vert[id] = id;
            max_degree = std::max(degree, max_degree);
            degrees[id] = degree;
            total_edge += degree;
        }
    }

    std::sort(vert.begin() + 1, vert.end(), [&](int lhs, int rhs) {
        return degrees[lhs] < degrees[rhs];
    });
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
            while (bin[++j] == 0)
                bin[j] = bin[j - 1];
        }
    }

    std::vector<bool> filtered(VERTEX_SIZE + 1, false);
    // k core
    int idx = 0, max_kcore = encodes_->GetIntSize();
    for (; idx < bin[max_kcore + 1]; ++idx) {

        std::vector<uint32_t> neighbors;
        uint32_t vertex = vert[idx], neighbor_degree, neighbor_pos, first_bin_pos, first_bin;
        uint32_t vertex_degree = degrees[vertex];

        std::vector<uint32_t> adj;
        if (adjacency_list_) {
            adj = adjacency_list_->at(vertex);
        }
        for (auto neighbor: adj) {
            if (filtered[neighbor])
                continue;
            neighbors.push_back(neighbor);
            neighbor_degree = degrees[neighbor];
            neighbor_pos = pos[neighbor];
            first_bin_pos = bin[neighbor_degree];
            first_bin = vert[first_bin_pos];
            if (vertex_degree >= neighbor_degree)
                continue;
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
        filtered[vertex] = true;
        encodes_->EncodeVertex(vertex, neighbors);
        edge_filtered += neighbors.size();
    }

    std::cout << "k core finished : kcore size:" << idx << "\n";
    std::cout << "edge filtered : " << edge_filtered << " total edge: " << total_edge/2<<" ratio:"<<((double)edge_filtered/total_edge *2 ) << std::endl;

#if VARY_K==1
    return;
#endif



//    omp_set_num_threads(30);
//
//    #pragma omp parallel for
    for (; idx < VERTEX_SIZE + 1; ++idx) {
        uint32_t vertex = vert[idx];
        std::vector<uint32_t> neighbors;
        if (adjacency_list_) {
            for (auto &neighbor: adjacency_list_->at(vertex)) {
                if (!filtered[neighbor])
                    neighbors.push_back(neighbor);
            }
        }
        // encode
        encodes_->EncodeVertex(vertex, neighbors);
    }
    std::cout << " encode finished" << "\n";

#endif 

#if UNDIRECTED==0
    for (int idx=0; idx < VERTEX_SIZE + 1; ++idx) {
        // encode
        encodes_->EncodeVertex(idx, adjacency_list_->at(idx));
    }
    std::cout << " encode finished" << "\n";

#endif 
}