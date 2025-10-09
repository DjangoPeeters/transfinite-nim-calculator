#include "nt_funcs.hpp"
#include "prime_generator.hpp"

#include <cstdint>
#include <utility>
#include <iostream>

using std::cout;

namespace nt_funcs {
    uint16_t f(uint16_t p) {
        uint16_t h = 1;
        uint16_t v = 2 % p;
        while (v != 1) {
            h++;
            v = (v << 1) % p;
        }
        return h;
    }

    // the same `uint32_t` as the one from `impartial_term_algebra::term_count`
    bool div_2_pow_min_1(uint16_t p, uint32_t pow2) {
        uint16_t v = 1;
        for (uint32_t i = 0; i < pow2; i++) v = (v << 1) % p;
        return v == 1;
    }

    std::pair<uint16_t, uint16_t> max_pow_min_prime_div(uint16_t n) {
        if (n == 0) {cout << "Error: 0 is divisible by every prime power"; fflush(stdout); while(1);}
        if (n == 1) return {2, 1};

        uint16_t p = 1;
        for (const auto p1 : prime_generator::primes(2, n + 1)) {
            if (n % p1 == 0) {
                p = p1;
                break;
            }
        }

        uint16_t q = p;
        while (n % (q * p) == 0) q *= p;
        return {p, q};
    }

    // q should be a non-trivial prime power
    std::pair<uint16_t, uint16_t> prime_pow(uint16_t q) {
        uint16_t p = 1;
        for (const auto p1 : prime_generator::primes(2, q + 1)) {
            if (q % p1 == 0) {
                p = p1;
                break;
            }
        }
        if (p == 1) {cout << "Error: q is not a non-trivial prime power"; fflush(stdout); while(1);}

        uint16_t e = 1;
        uint16_t pe = p;
        while (pe < q) {
            e++;
            pe *= p;
        }
        if (pe != q) {cout << "Error: q is not a non-trivial prime power"; fflush(stdout); while(1);}
        return {p, e};
    }

    bool is_3_pow(uint16_t n) {
        if (n == 0) return false;
        uint16_t curpow = 1;
        while (curpow < n) curpow += (curpow << 1);
        return curpow == n;
    }
};