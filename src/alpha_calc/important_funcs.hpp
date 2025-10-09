#ifndef IMPORTANT_FUNCS_HPP
#define IMPORTANT_FUNCS_HPP

#include <cstdint>
#include <utility>
#include <vector>
#include <map>

extern uint32_t MAX_TERM_COUNT;

typedef struct {
    bool failed;
    union {
        uint8_t result;
        uint32_t term_count;
    };
    bool used_cache;
} excess_return;

namespace important_funcs {
    void init();

    const std::map<uint16_t, std::vector<uint16_t>> get_q_set_cache();
    std::pair<uint32_t, std::vector<uint16_t>> q_set(uint16_t p);

    const std::map<uint16_t, uint8_t> get_excess_cache();
    excess_return excess(uint16_t p); // every excess for primes p <= 257 is at most 4
}

#endif