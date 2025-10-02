#ifndef IMPORTANT_FUNCS_HPP
#define IMPORTANT_FUNCS_HPP

#include <cstdint>
#include <vector>
#include <map>

typedef struct {
    bool failed;
    union {
        uint8_t result;
        uint32_t term_count;
    };
} excess_return;

namespace important_funcs {
    const std::map<uint16_t, std::vector<uint16_t>>& get_q_set_cache();
    std::vector<uint16_t> q_set(uint16_t p);

    const std::map<uint16_t, uint8_t>& get_excess_cache();
    excess_return excess(uint16_t p); // every excess for primes p <= 257 is at most 4
}

#endif