

#ifndef VEND_CODEC_H
#define VEND_CODEC_H

#include "util/bitset.h"
#include "include/common/config.h"
#include <vector>

struct DeltaInfo {
    size_t delta_bits;
    size_t val_bits;
    size_t total_bits;
};

class Codec {

public:
    enum  FindRes:char {
        EXIST = 0,
        LESS_THAN_MIN = 2,
        GRATER_THAN_MAX =4,
        NOT_EXIST =15
    };

    /**
     * @param in : encode array
     * @param in_size : encode array size
     * @param out : array in bytes
     * @param out_length : output length
     */
    virtual void
    Encode(const uint32_t *__restrict__ in, size_t in_size, uint32_t *__restrict__ out, uint32_t &out_bytes_length) {
        EncodeArray(in, in_size, reinterpret_cast<uint8_t *>(out), out_bytes_length);
    };

    virtual void EncodeArray(const uint32_t *__restrict__ in, size_t in_size, uint8_t *__restrict__ out,
                             uint32_t &out_bytes_length) = 0;


    /**
     * @return : how many bytes for encode
     * @param in
     * @param in_size
     * @return
     */
    virtual size_t GetEncodeLength(const uint32_t *in, size_t in_size) = 0;


    /**
     *
     * @param integer
     * @return  the bytes integer takes
     */
    virtual size_t GetOneEncodeLength(uint32_t integer) = 0;


    virtual FindRes Find(uint8_t **&inbyte, size_t in_bytes_length, uint32_t element) {};

    virtual FindRes FindWithCount(uint8_t **__restrict__&inbyte, uint32_t count, uint32_t element) {};


    virtual void
    Decode(const uint8_t *__restrict__ in, size_t in_bytes_length, uint32_t *__restrict__ out, uint32_t &out_size) = 0;


    /**
     *
     * @param in   : encode data
     * @param in_size
     * @param delta : delta[i] = how many bits for in[i]-in[i-1];
     * @param bytes_length : bytes_length[i]: how many bytes takes for encoding in[0]、in[1]...in[i]
     * notice : delta and bytes_length must be empty
     */
    virtual void ParseEncode(const uint32_t *in, size_t in_size, std::vector<DeltaInfo> &delta_infos) = 0;


private:


};


#endif //VEND_CODEC_H
