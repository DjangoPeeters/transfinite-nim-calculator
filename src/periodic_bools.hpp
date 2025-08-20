#ifndef PERIODIC_BOOLS_HPP
#define PERIODIC_BOOLS_HPP

#include <vector>

using std::vector;

namespace periodic_bools {
    // returns how many times some big part repeats
    // (how many times, until which index goes repeating part)
    std::pair<std::size_t, std::size_t> find_rep(const vector<bool>& bools);
}

#endif