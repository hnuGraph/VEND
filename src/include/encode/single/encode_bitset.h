

#ifndef VEND_ENCODE_BITSET_H
#define VEND_ENCODE_BITSET_H

#include <fstream>
#include <sstream>

#include "encode/encode.h"
#include "util/bitset.h"
#include "util/bitset_simd.h"

/**
 *  super class for hash encode / range encode /  hybrid encode
 *
 * */
class EncodeBitSet : public Encode {
   public:
    enum class EncodeType { UnFull = 0, Full = 1, NonDecodable = 2 };

    EncodeBitSet() : Encode(), encode_size_(VERTEX_SIZE + 1) {
        this->encode_bitset_ = new BitSetSIMD *[VERTEX_SIZE + 1];
        for (uint32_t i = 0; i < VERTEX_SIZE + 1; ++i) {
            encode_bitset_[i] = new BitSetSIMD(PER_ENCODE_BIT_SIZE);
        }
    }

    EncodeBitSet(size_t bit_size, size_t vertex_size) : Encode(), encode_size_(vertex_size + 1) {
        this->encode_bitset_ = new BitSetSIMD *[vertex_size + 1];
        for (uint32_t i = 0; i < vertex_size + 1; ++i) {
            encode_bitset_[i] = new BitSetSIMD(bit_size);
        }
    }

    ~EncodeBitSet() {
        for (uint32_t i = 0; i < encode_size_; ++i) delete encode_bitset_[i];
        delete[] encode_bitset_;
    }

    virtual PairType NonNeighborTest(uint32_t vertex1, uint32_t vertex2){};

    void EncodePersistent(DbEngine *encode_db) override{};

    void EncodePersistent(std::string file_path) override {
        std::ofstream output(file_path);
        for (uint32_t i = 1; i <= VERTEX_SIZE; ++i) {
            std::vector<uint32_t> encode = encode_bitset_[i]->Dump();
            output << i << "\t";
            for (auto &v : encode) output << v << "\t";
            output << "\n";
            if (i % 1000000 == 0) output.flush();
        }
        std::cout << "file output close" << std::endl;
        output.close();
    };

    virtual void EncodePersistent(std::string file_path, bool out) override {
        std::ofstream output(file_path);
        for (uint32_t i = 1; i <= VERTEX_SIZE; ++i) {
            std::vector<uint32_t> encode;
            if (out)
                encode = encode_bitset_[i]->Dump();
            else
                encode = in_bitset_[i]->Dump();
            output << i << "\t";
            for (auto &v : encode) output << v << "\t";
            output << "\n";
            if (i % 1000000 == 0) output.flush();
        }
        std::cout << "file output close" << std::endl;
        output.close();
    };

    virtual void LoadFromDb(std::string file_path, bool out) override{

        std::ifstream infile(file_path);
        std::string line;
        while (std::getline(infile, line)) {
            std::vector<uint32_t> temp;
            uint32_t key, vertex;
            std::istringstream line_string(line);
            line_string >> key;
            while (line_string >> vertex) temp.push_back(vertex);
            if(out)
                encode_bitset_[key]->BlockSet(temp);
            else
                in_bitset_[key]->BlockSet(temp);
        }
        infile.close();

    };

    // load all encode from db
    void LoadFromDb(DbEngine *encode_db) override{};

    void LoadFromDb(std::string file_path) override {
        std::ifstream infile(file_path);
        std::string line;
        while (std::getline(infile, line)) {
            std::vector<uint32_t> temp;
            uint32_t key, vertex;
            std::istringstream line_string(line);
            line_string >> key;
            while (line_string >> vertex) temp.push_back(vertex);
            encode_bitset_[key]->BlockSet(temp);
        }
        infile.close();
    };

   protected:
    uint32_t encode_size_;
    // vertex id starts with 1
    BitSetSIMD **encode_bitset_;
    BitSetSIMD **in_bitset_;
};

#endif  // VEND_ENCODE_BITSET_H
