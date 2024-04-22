#ifndef VEND_SIMD_ENCODE_H
#define VEND_SIMD_ENCODE_H

#include <fstream>
#include <sstream>

#include "encode/codec/SIMD/delta_simd_codec.h"
#include "encode/encode.h"
#include "util/bitset_simd.h"

class SIMDEncode : public Encode {
   public:
    SIMDEncode() : Encode(), encode_size_(VERTEX_SIZE) {
        this->encode_bitset_ = new BitSetSIMD *[VERTEX_SIZE + 1];
        for (int i = 0; i < VERTEX_SIZE + 1; ++i) {
            encode_bitset_[i] = new BitSetSIMD(PER_ENCODE_BIT_SIZE);
        }
    }

    SIMDEncode(size_t bit_size, size_t vertex_size) : Encode(), encode_size_(vertex_size + 1) {
        this->encode_bitset_ = new BitSetSIMD *[vertex_size + 1];
        for (int i = 0; i < vertex_size + 1; ++i) {
            encode_bitset_[i] = new BitSetSIMD(bit_size);
        }
    }

    ~SIMDEncode() {
        for (uint32_t i = 0; i <= encode_size_; ++i) delete encode_bitset_[i];
        delete[] encode_bitset_;
        encode_bitset_ = nullptr;
    }

    uint32_t GetBytesSize() override { return encode_bitset_[0]->GetBitSize() - 1; }

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

    void LoadFromDb(std::string file_path) override {
        std::ifstream infile(file_path);
        std::string line;
        uint32_t vertex = 1;
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
    
   protected:
    uint32_t encode_size_;
    BitSetSIMD **encode_bitset_;
    BitSetSIMD **in_bitset_;


};

#endif  // VEND_SIMD_ENCODE_H
