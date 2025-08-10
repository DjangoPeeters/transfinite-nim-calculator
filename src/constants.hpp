#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>
#include <vector>
#include <map>

using std::vector;
using std::map;

namespace test_values {
    extern const map<uint16_t, vector<uint16_t>> q_set_cache;
    extern const map<uint16_t, uint8_t> excess_cache;
};

namespace previously_known_values { // known at 6 august 2025
    extern const map<uint16_t, vector<uint16_t>> q_set_cache;
    extern const map<uint16_t, uint8_t> excess_cache;
};

map<uint16_t, vector<uint16_t>> q_set_records();
map<uint16_t, uint8_t> excess_records();

namespace record_values {
    extern map<uint16_t, vector<uint16_t>> q_set_cache;
    extern map<uint16_t, uint8_t> excess_cache;
};

void cache_q_set(uint16_t p, vector<uint16_t> q_set_p);
void cache_excess(uint16_t p, uint8_t excess_p);

#endif