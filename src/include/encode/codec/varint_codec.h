#ifndef VEND_VARINT_CODEC_H
#define VEND_VARINT_CODEC_H


#include "codec.h"

/**
 * 0 indicates the integer is finished
 * fork from https://github.com/lemire/SIMDCompressionAndIntersection
 */
class VarintCodec : public Codec {

public:



    VarintCodec() {
        this->is_delta_ = true;
    }

    VarintCodec(bool is_delta) : is_delta_(is_delta) {}


    void EncodeArray(const uint32_t *__restrict__ in, size_t in_size, uint8_t *__restrict__ out, uint32_t &out_bytes_length) override {
        uint32_t prev = 0;
        const uint8_t *const initout = out;
        for (size_t k = 0; k < in_size; ++k) {
            const uint32_t val = is_delta_ ? (in[k] - prev) : in[k];
            if (is_delta_)
                prev = in[k];
            /**
             * Code below could be shorter. Whether it could be faster
             * depends on your compiler and machine.
             */
            if (val < bound[0]) {
                *out = val & 0x7F;
                ++out;
            } else if (val < bound[1]) {
                *out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(val >> 7);
                ++out;
            } else if (val < bound[2]) {
                *out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(val >> 14);
                ++out;
            } else if (val < bound[3]) {
                *out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 14) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(val >> 21);
                ++out;
            } else {
                *out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 14) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(((val >> 21) & 0x7F) | mask_7_zero);
                ++out;
                *out = static_cast<uint8_t>(val >> 28);
                ++out;
            }
        }
        out_bytes_length = out - initout;
    }


    /**
     *  encode with logical address rather than driectly address
     * @param in
     * @param in_size
     * @param out
     * @param out_bytes_length
     */
    void EncodeUnDirectArray(const uint32_t *in, size_t in_size, uint8_t **&out, uint32_t &out_bytes_length) {
        uint32_t prev = 0;
        out_bytes_length = 0;
        uint8_t **const initout = out;
        for (size_t k = 0; k < in_size; ++k) {
            const uint32_t val = is_delta_ ? (in[k] - prev) : in[k];
            if (is_delta_)
                prev = in[k];
            /**
             * Code below could be shorter. Whether it could be faster
             * depends on your compiler and machine.
             */
            if (val < bound[0]) {
                **out = val & 0x7F;
                ++out;
            } else if (val < bound[1]) {
                **out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(val >> 7);
                ++out;
            } else if (val < bound[2]) {
                **out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(val >> 14);
                ++out;
            } else if (val < bound[3]) {
                **out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 14) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(val >> 21);
                ++out;
            } else {
                **out = static_cast<uint8_t>((val & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 7) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 14) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(((val >> 21) & 0x7F) | mask_7_zero);
                ++out;
                **out = static_cast<uint8_t>(val >> 28);
                ++out;
            }
        }
        out_bytes_length = out - initout;
    }

    FindRes Find(uint8_t **&inbyte, size_t in_bytes_length, uint32_t element) override {
        uint32_t prev = 0;
        uint8_t **const endbyte = inbyte + in_bytes_length;
        if (is_delta_) {
            uint8_t c;
            uint32_t v;

            c = *inbyte[0];
            v = c & 0x7F;
            if (c < 128) {
                inbyte += 1;
                prev = v + prev;
            } else {
                c = *inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    prev = v + prev;
                } else {
                    c = *inbyte[2];
                    v |= (c & 0x7F) << 14;
                    if (c < 128) {
                        inbyte += 3;
                        prev = v + prev;
                    } else {
                        c = *inbyte[3];
                        v |= (c & 0x7F) << 21;
                        if (c < 128) {
                            inbyte += 4;
                            prev = v + prev;
                        } else {
                            c = *inbyte[4];
                            inbyte += 5;
                            v |= (c & 0x0F) << 28;
                            prev = v + prev;
                        }
                    }
                }
            }
            if (element == prev)
                return FindRes::EXIST;
            else if (element < prev)
                return FindRes::LESS_THAN_MIN;
        }
        while (endbyte > inbyte + 5) {
            if (is_delta_) {
                uint8_t c;
                uint32_t v;

                c = *inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    prev = v + prev;
                    if (element == prev)
                        return FindRes::EXIST;
                    else if(element<prev)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    prev = v + prev;
                    if (element == prev)
                        return FindRes::EXIST;
                    else if(element<prev)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    prev = v + prev;
                    if (element == prev)
                        return FindRes::EXIST;
                    else if(element<prev)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    prev = v + prev;
                    if (element == prev)
                        return FindRes::EXIST;
                    else if(element<prev)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                prev = v + prev;
                if (element == prev)
                    return FindRes::EXIST;
                else if(element<prev)
                    return FindRes::NOT_EXIST;
            } else {
                uint8_t c;
                uint32_t v;

                c = *inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    if (element == v)
                        return FindRes::EXIST;
                    else if(element<v)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    if (element == v)
                        return FindRes::EXIST;
                    else if(element<v)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    if (element == v)
                        return FindRes::EXIST;
                    else if(element<v)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    if (element == v)
                        return FindRes::EXIST;
                    else if(element<v)
                        return FindRes::NOT_EXIST;
                    continue;
                }

                c = *inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                if (element == v)
                    return FindRes::EXIST;
                else if(element<v)
                    return FindRes::NOT_EXIST;
            }
        }
        while (endbyte > inbyte) {
            unsigned int shift = 0;
            for (uint32_t v = 0; endbyte > inbyte; shift += 7) {
                uint8_t c = **inbyte++;
                v += ((c & 127) << shift);
                if ((c < 128)) {
                    if (is_delta_) {
                        prev = v + prev;
                        if (element == prev)
                            return FindRes::EXIST;
                        else if(element<v)
                            return FindRes::NOT_EXIST;
                    } else {
                        if (element == v)
                            return FindRes::EXIST;
                        else if(element<v)
                            return FindRes::NOT_EXIST;
                    }
                    break;
                }
            }
        }
        if (prev < element) {
            return FindRes::GRATER_THAN_MAX;
        }
        return FindRes::NOT_EXIST;
    }

    void Decode(const uint8_t *__restrict__ inbyte, size_t in_bytes_length, uint32_t *__restrict__ out, uint32_t &out_size) {
        // only support differential encoding now
        uint32_t prev = 0;
        const uint8_t *const endbyte = inbyte + in_bytes_length;
        const uint32_t *const initout(out);
        // this assumes that there is a value to be read

        while (endbyte > inbyte + 5) {
            if (is_delta_) {
                uint8_t c;
                uint32_t v;

                c = inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                *out++ = (prev = v + prev);
            } else {
                uint8_t c;
                uint32_t v;

                c = inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    *out++ = v;
                    continue;
                }

                c = inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    *out++ = v;
                    continue;
                }

                c = inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    *out++ = v;
                    continue;
                }

                c = inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    *out++ = v;
                    continue;
                }

                c = inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                *out++ = v;
            }
        }
        while (endbyte > inbyte) {
            unsigned int shift = 0;
            for (uint32_t v = 0; endbyte > inbyte; shift += 7) {
                uint8_t c = *inbyte++;
                v += ((c & 127) << shift);
                if ((c < 128)) {
                    *out++ = is_delta_ ? (prev = v + prev) : v;
                    break;
                }
            }
        }
        out_size = out - initout;
    }

    void DecodeUnDirectArray(uint8_t **&inbyte, size_t in_bytes_length, uint32_t *out, uint32_t &out_size) {
        uint32_t prev = 0;
        uint8_t **const endbyte = inbyte + in_bytes_length;
        const uint32_t *const initout(out);
        // this assumes that there is a value to be read

        while (endbyte > inbyte + 5) {
            if (is_delta_) {
                uint8_t c;
                uint32_t v;

                c = *inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = *inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = *inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = *inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    *out++ = (prev = v + prev);
                    continue;
                }

                c = *inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                *out++ = (prev = v + prev);
            } else {
                uint8_t c;
                uint32_t v;

                c = *inbyte[0];
                v = c & 0x7F;
                if (c < 128) {
                    inbyte += 1;
                    *out++ = v;
                    continue;
                }

                c = *inbyte[1];
                v |= (c & 0x7F) << 7;
                if (c < 128) {
                    inbyte += 2;
                    *out++ = v;
                    continue;
                }

                c = *inbyte[2];
                v |= (c & 0x7F) << 14;
                if (c < 128) {
                    inbyte += 3;
                    *out++ = v;
                    continue;
                }

                c = *inbyte[3];
                v |= (c & 0x7F) << 21;
                if (c < 128) {
                    inbyte += 4;
                    *out++ = v;
                    continue;
                }

                c = *inbyte[4];
                inbyte += 5;
                v |= (c & 0x0F) << 28;
                *out++ = v;
            }
        }
        while (endbyte > inbyte) {
            unsigned int shift = 0;
            for (uint32_t v = 0; endbyte > inbyte; shift += 7) {
                uint8_t c = **inbyte++;
                v += ((c & 127) << shift);
                if ((c < 128)) {
                    *out++ = is_delta_ ? (prev = v + prev) : v;
                    break;
                }
            }
        }
        out_size = out - initout;
    }

    size_t GetOneEncodeLength(uint32_t integer) override {
        if (integer < bound[0])
            return 1;
        else if (integer < bound[1])
            return 2;
        else if (integer < bound[2])
            return 3;
        else if (integer < bound[3])
            return 4;
        else
            return 5;
    }


    size_t GetEncodeLength(const uint32_t *in, size_t in_size) override {
        size_t size = 0;
        uint32_t pre = 0;
        for (int i = 0; i < in_size; ++i) {
            const uint32_t val = is_delta_ ? in[i] - pre : in[i];
            if (is_delta_)
                pre = in[i];
            size += GetOneEncodeLength(val);
        }
        return size;
    }

    virtual void ParseEncode(const uint32_t *in, size_t in_size, std::vector<DeltaInfo> &delta_infos) override {
        size_t size = 0;
        uint32_t pre = 0;
        for (int i = 0; i < in_size; ++i) {
            const uint32_t val = is_delta_ ? in[i] - pre : in[i];
            if (is_delta_)
                pre = in[i];
            uint32_t val_bits = GetOneEncodeLength(val);
            size += val_bits;
            if (i == 0)
                delta_infos.push_back({0, GetOneEncodeLength(in[i]), size});
            else
                delta_infos.push_back({val_bits, GetOneEncodeLength(in[i]), size});
        }
    };

private:
    const static unsigned int mask_7_zero = (1U << 7);
    constexpr static uint32_t bound[4] = {1U << 7, 1U << 14, 1U << 21, 1U << 28};
    bool is_delta_;


};


#endif //VEND_VARINT_CODEC_H
