

#ifndef VEND_ENCODE_BITSET_H
#define VEND_ENCODE_BITSET_H

#include <fstream>
#include <sstream>
#include "encode/encode.h"
#include "util/bitset.h"

/**
 *  super class for hash encode / range encode /  hybrid encode
 *
 * */
class EncodeBitSet : public Encode {
public:
    enum class EncodeType {
        UnFull = 0, Full = 1, NonDecodable = 2
    };


    EncodeBitSet() : Encode() {

    }

    virtual PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2) {};

    void EncodePersistent(DbEngine *encode_db) override {};

    void EncodePersistent(std::string file_path) override {
        std::ofstream output(file_path);
        for (uint32_t i = 1; i <= VERTEX_SIZE; ++i) {
            std::vector<uint32_t> encode = encode_bitset_[i].Dump();
            output << i << "\t";
            for (auto &v:encode)
                output << v << "\t";
            output << "\n";
            if(i%1000000==0)
                output.flush();
        }
        std::cout<<"file output close"<<std::endl;
        output.close();
    };

    // load all encode from db
    void LoadFromDb(DbEngine *encode_db) override {};

    void LoadFromDb(std::string file_path) override {

        std::ifstream infile(file_path);
        std::string line;
        uint32_t vertex = 1;
        while (std::getline(infile, line)) {
            std::vector<uint32_t> temp;
            uint32_t key, vertex;
            std::istringstream line_string(line);
            line_string >> key;
            while (line_string >> vertex)
                temp.push_back(vertex);
            encode_bitset_[key].BlockSet(temp);
        }
        infile.close();
    };


protected:
    // vertex id starts with 1
    BitSet<PER_ENCODE_BIT_SIZE> encode_bitset_[VERTEX_SIZE + 1];


};


#endif //VEND_ENCODE_BITSET_H
