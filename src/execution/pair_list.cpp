//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#include "execution/pair_list.h"

void PairList::Init() {
    if (access(pair_path_.c_str(), F_OK) != 0) {
        // pair list  file doesn't exists ,create pair
        CreateList(list_size_, &pair_list_);
#if SAVE_LIST == 1
        ListPersistent();
#endif
    } else {
        LoadPairList(pair_path_);
    }
}

void PairList::LoadPairList(std::string pair_path) {
    uint32_t vertex1, vertex2;
    std::ifstream infile(pair_path);
    std::string line;
    std::istringstream line_s;
    while (std::getline(infile, line)) {
        line_s.str(line);
        line_s >> vertex1 >> vertex2;
        pair_list_.push_back({vertex1, vertex2});
        line_s.clear();
    }
    infile.close();
}

void PairList::ListPersistent() {
    assert(!pair_list_.empty());

    std::ofstream output(pair_path_, std::ios::out);
    for (auto &pair : pair_list_) {
        output << pair.first << "\t" << pair.second << "\n";
    }
    output.close();
}
