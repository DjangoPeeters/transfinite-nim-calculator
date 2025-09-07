#include "fin_nim.hpp"

#include <cstdint>
#include <boost/multiprecision/integer.hpp>

using boost::multiprecision::msb;

namespace fin_nim {
    namespace {
        uint64_t fin_nim_mul_pows(uint8_t a, uint8_t b) {
            if (a == 0) return ((uint64_t)1) << b;
            if (b == 0) return ((uint64_t)1) << a;
            if (a > b) return fin_nim_mul_pows(b, a);

            uint8_t ah = ((uint8_t)1) << msb(a);
            uint8_t bh = ((uint8_t)1) << msb(b);
            uint8_t al = a - ah;
            uint8_t bl = b - bh;

            if (ah == bh) {
                return fin_nim_add(fin_nim_mul_pows(a, bl), fin_nim_mul(fin_nim_mul_pows(al, bl), (((uint64_t)1) << (bh - 1))));
            } else {
                return fin_nim_mul_pows(a, bl) << bh;
            }
        }
    }

    inline uint64_t fin_nim_add(uint64_t a, uint64_t b) {
        return a ^ b;
    }

    uint64_t fin_nim_mul(uint64_t a, uint64_t b) {
        if (a == 0 || b == 0) return 0;
        if (a == 1) return b;
        if (b == 1) return a;

        uint8_t ahb = msb(a); // highest bit of a
        uint8_t bhb = msb(b);
        uint64_t ah = ((uint64_t)1) << ahb; // high part of a
        uint64_t bh = ((uint64_t)1) << bhb;
        uint64_t al = a - ah; // low part of a
        uint64_t bl = b - bh;

        return fin_nim_add(fin_nim_add(fin_nim_mul_pows(ahb, bhb), fin_nim_mul(al, bh)),
            fin_nim_add(fin_nim_mul(ah, bl), fin_nim_mul(al, bl)));
    }
};