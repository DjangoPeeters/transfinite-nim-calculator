#ifndef NT_FUNCS_HPP
#define NT_FUNCS_HPP

#include <cstdint>
#include <utility>

// number theoretic functions
namespace nt_funcs {
    uint16_t f(uint16_t p);
    bool div_2_pow_min_1(uint16_t p, uint32_t pow2); // the same `uint32_t` as the one from `impartial_term_algebra::term_count`
    std::pair<uint16_t, uint16_t> max_pow_min_prime_div(uint16_t n);
    std::pair<uint16_t, uint16_t> prime_pow(uint16_t q); // q should be a non-trivial prime power
    bool is_3_pow(uint16_t n);
}

#endif