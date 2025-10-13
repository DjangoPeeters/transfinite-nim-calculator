#include "fin_nim.hpp"

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::msb;
using uint256_t = boost::multiprecision::uint256_t;

namespace fin_nim {
    namespace {
        uint256_t fin_nim_mul_pows(unsigned a, unsigned b) {
            if (a == 0) return ((uint256_t)1) << b;
            if (b == 0) return ((uint256_t)1) << a;
            if (a > b) return fin_nim_mul_pows(b, a);

            unsigned ah = 1U << msb(a);
            unsigned bh = 1U << msb(b);
            unsigned al = a - ah;
            unsigned bl = b - bh;

            if (ah == bh) {
                return fin_nim_add(fin_nim_mul_pows(a, bl), fin_nim_mul(fin_nim_mul_pows(al, bl), (((uint256_t)1) << (bh - 1))));
            } else {
                return fin_nim_mul_pows(a, bl) << bh;
            }
        }
    }

    uint256_t fin_nim_add(uint256_t a, uint256_t b) {
        return a ^ b;
    }

    uint256_t fin_nim_mul(uint256_t a, uint256_t b) {
        if (a == 0 || b == 0) return 0;
        if (a == 1) return b;
        if (b == 1) return a;

        unsigned ahb = msb(a); // highest bit of a
        unsigned bhb = msb(b);
        uint256_t ah = ((uint256_t)1) << ahb; // high part of a
        uint256_t bh = ((uint256_t)1) << bhb;
        uint256_t al = a - ah; // low part of a
        uint256_t bl = b - bh;

        return fin_nim_add(fin_nim_add(fin_nim_mul_pows(ahb, bhb), fin_nim_mul(al, bh)),
            fin_nim_add(fin_nim_mul(ah, bl), fin_nim_mul(al, bl)));
    }

    vector<uint8_t> fin_to_2_pow(uint256_t n) {
        if (n == 0) return {};
        
        vector<uint8_t> result{};
        for (uint8_t i = msb(n); i > 0; i--) {
            if ((n & (((uint256_t)1) << i)) != 0) {
                result.push_back(i);
            }
        }
        if ((n & 1) != 0) result.push_back(0);
        return result;
    }
};