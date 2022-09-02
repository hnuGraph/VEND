#include "graph/large_graph.h"

#include <memory>


void LargeGraph::LoadAndSaveData() {
    assert(!data_path_.empty());

    std::unordered_map<uint32_t, std::vector<uint32_t>> *adj = new std::unordered_map<uint32_t, std::vector<uint32_t>>();
    //adj->reserve(write_batch);
    int batch_size = 0;
    uint32_t vertex1, vertex2, vertex_before = 0;
    // original data file is organized by first vertex id


    int fd = open(data_path_.c_str(), O_RDWR, 00666);
    if (fd == -1) {
        printf("file doesn't exist \n");
        exit(-1);
    }
    struct stat st;
    fstat(fd, &st);
    uint64_t len = st.st_size;

    char *p = (char *) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    uint64_t i = 0;
    int idx = 1;
    while (i < len - 1 && p[i] != EOF) {
        vertex1 = 0;
        while (p[i] >= '0' && p[i] <= '9') {
            vertex1 = vertex1 * 10 + p[i] - '0';
            i++;
        }
        while (p[i] < '0' || p[i] > '9') i++;
        vertex2 = 0;
        while (p[i] >= '0' && p[i] <= '9') {
            vertex2 = vertex2 * 10 + p[i] - '0';
            i++;
        }
        while (i < len - 1 && (p[i] < '0' || p[i] > '9')) i++;
        // update edges
        std::unordered_map<uint32_t, std::vector<uint32_t>>::iterator iter;
        iter = adj->find(vertex1);
        if (iter != adj->end())
            iter->second.push_back(vertex2);
        else
            adj->insert({vertex1, std::vector<uint32_t>({vertex2})});

        iter = adj->find(vertex2);
        if (iter != adj->end())
            iter->second.push_back(vertex1);
        else
            adj->insert({vertex2, std::vector<uint32_t>({vertex1})});

        ++batch_size;
        // write for each batch
        if (batch_size % write_batch == 0) {
            UpdateEdges(adj);
            delete adj;
            malloc_trim(0);
            adj = new std::unordered_map<uint32_t, std::vector<uint32_t>>();
            //adj->reserve(write_batch);
            std::cout << "idx :" << idx++ << "batch size:" << write_batch << " adj:" << adj->size() << " bucket  "
                      << adj->bucket_count() << std::endl;
            batch_size = 0;
        }

    }
    UpdateEdges(adj);
    munmap(p, len);
    close(fd);

}

void LargeGraph::UpdateEdges(std::unordered_map<uint32_t, std::vector<uint32_t>> *adj) {

    std::vector<uint32_t> keys;
    std::vector<std::vector<uint32_t>> values(adj->size());
    uint32_t idx = 0;
    for (auto iter = adj->begin(); iter != adj->end();) {
        if (graph_db_->Get(iter->first, &values[idx]))
            values[idx].insert(values[idx].end(), iter->second.begin(), iter->second.end());
        else
            values[idx] = iter->second;
        std::sort(values[idx].begin(),values[idx].end());
        idx++;
        keys.push_back(iter->first);
        adj->erase(iter++);
    }
    graph_db_->BatchWrite(keys, values);
}

void LargeGraph::Init() {

    if (!db_path_.empty())
        graph_db_ = std::make_shared<RocksDb>(db_path_);
    if (vend_type_ != VendType::NoVend) {
        if (access(vend_path_.c_str(), F_OK) != 0 && !data_path_.empty()) {
            // encode file when is not exists ,load data
            adjacency_list_= nullptr;
            //LoadData();
            VendFactory::GetEncode(vend_type_, vend_path_, graph_db_, vend);
            BuildEncode();

        } else {
            VendFactory::GetEncode(vend_type_, vend_path_, graph_db_, vend);
            vend->LoadEncode();

        }
    }
    std::cout << "graph initial finished" << std::endl;
}



