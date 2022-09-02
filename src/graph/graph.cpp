//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//



#include <iostream>

#include "graph/graph.h"

void Graph::Init() {
    // load or build encoding
    adjacency_list_ = std::make_shared<std::vector<std::vector<uint32_t >>>();
    if (!db_path_.empty()) {

        switch (db_type_) {
            case 0:
                graph_db_ = std::make_shared<RocksDb>(db_path_);
                break;
            case 1:
                graph_db_ = std::make_shared<Neo4j>(URL);
                break;
            case 2:
                graph_db_ = std::make_shared<DGraph>(DGRAPH_RPC_URL);
                break;
            case 3:
                graph_db_ = std::make_shared<Gstore>(db_path_);
        }

    }
    if (vend_type_ != VendType::NoVend) {
        if (access(vend_path_.c_str(), F_OK) != 0 && !data_path_.empty()) {
            // encode file when is not exists ,load data
            LoadData();
            VendFactory::GetEncode(vend_type_, adjacency_list_, vend_path_, graph_db_, vend);
            BuildEncode();

        } else {
            VendFactory::GetEncode(vend_type_, adjacency_list_, vend_path_, graph_db_, vend);
            vend->LoadEncode();

        }
    }
    std::cout << "graph initial finished" << std::endl;
}

void Graph::LoadData() {
    assert(!data_path_.empty());
    adjacency_list_->resize(VERTEX_SIZE + 1);

    int fd = open(data_path_.c_str(), O_RDWR, 00666);
    if (fd == -1) {
        printf("file doesn't exist \n");
        exit(-1);
    }
    struct stat st;
    fstat(fd, &st);
    uint64_t len = st.st_size;

    char *p = (char *) mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    uint64_t i = 0;
    uint32_t vertex1, vertex2;
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

        adjacency_list_->at(vertex1).push_back(vertex2);
        adjacency_list_->at(vertex2).push_back(vertex1);

    }
    // id begins with 1
    munmap(p, len);
    close(fd);
}

void Graph::SaveData() {
    assert(!adjacency_list_->empty());
    assert(graph_db_ != nullptr);
    for (uint32_t id = 1; id <= VERTEX_SIZE; ++id) {
        if(adjacency_list_->at(id).size()==0)
            continue;
        std::vector<uint32_t> neighbors;
        graph_db_->Get(id,&neighbors);
        neighbors.insert(neighbors.end(),adjacency_list_->at(id).begin(),adjacency_list_->at(id).end());
        std::sort(neighbors.begin(),neighbors.end());
        //std::vector<uint32_t> neighbors(adjacency_list_->at(id).begin(), adjacency_list_->at(id).end());
        //graph_db_->Put(id, adjacency_list_->at(id));
        graph_db_->Put(id, neighbors);
    }
}

void Graph::BuildEncode() {
    Timer t;
    t.StartTimer();
    vend->BuildEncoding();
    t.StopTimer();
    std::cout<<VEND_STRING[vend_type_]<<" build time : "<<t.CountTime()/1000000<<"s"<<std::endl;


    vend->EncodePersistent();
    std::cout<<" encode persistent "<<std::endl;
}

void Graph::DbInsert(uint32_t vertex1, uint32_t vertex2) {
    std::vector<uint32_t> neighbors;

    graph_db_->Get(vertex1, &neighbors);
    neighbors.push_back(vertex2);
    graph_db_->Put(vertex1, neighbors);

    graph_db_->Get(vertex2, &neighbors);
    neighbors.push_back(vertex1);
    graph_db_->Put(vertex2, neighbors);

}

void Graph::DbDelete(uint32_t vertex1, uint32_t vertex2) {

    std::vector<uint32_t> neighbors;

    graph_db_->Get(vertex1, &neighbors);
    auto iter = std::find(neighbors.begin(), neighbors.end(), vertex2);
    if (iter != neighbors.end()) {
        neighbors.erase(iter);
        graph_db_->Put(vertex1, neighbors);
    }


    std::vector<uint32_t>().swap(neighbors);

    graph_db_->Get(vertex2, &neighbors);
    iter = std::find(neighbors.begin(), neighbors.end(), vertex1);
    if (iter != neighbors.end()) {
        neighbors.erase(iter);
        graph_db_->Put(vertex2, neighbors);
    }


}

bool Graph::DbQuery(uint32_t vertex1, uint32_t vertex2) {

    if (db_type_)
        return graph_db_->QueryEdge(vertex1, vertex2);
    else {
        std::vector<uint32_t> neighbors;
        graph_db_->Get(vertex1, &neighbors);
        return std::find(neighbors.begin(), neighbors.end(), vertex2) != neighbors.end();
    }
}


void Graph::BackUpDb() {
//    std::uniform_int_distribution<unsigned> u;
//    std::default_random_engine e;
//    e.seed(time(NULL));
//    std::string bak_path = db_path_ + VEND_STRING[vend_type_] + std::to_string(u(e));
//    std::string command = "cp -r " + db_path_ + " " + bak_path;
//    system(command.c_str());
//    db_path_ = bak_path;

}

void Graph::DestoryDb() {
//    if (graph_db_ != nullptr) {
//        graph_db_->Close();
//        graph_db_ = nullptr;
//    }
//    std::string command = "rm -rf " + db_path_;
//    system(command.c_str());
//    db_path_ = "";

}




