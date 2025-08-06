#ifndef IMPORTANT_FUNCS_HPP
#define IMPORTANT_FUNCS_HPP

#include <cstdint>
#include <vector>
#include <map>

using std::vector;
using std::map;

namespace important_funcs {
    map<uint16_t, vector<uint16_t>> get_q_set_cache();
    vector<uint16_t> q_set(uint16_t p);

    map<uint16_t, uint8_t> get_excess_cache();
    uint8_t excess(uint16_t p); // every excess for primes p <= 257 is at most 4
}

#endif