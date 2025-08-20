#ifndef PERIODIC_BOOLS_HPP
#define PERIODIC_BOOLS_HPP

#include <vector>

using std::vector;
using std::size_t;

namespace periodic_bools {
    // (index of the most significant bit)+1
    size_t vb_msbp1(const vector<bool>& bools, size_t end);

    // returns how many times some big part repeats
    // ((how many times, until which index goes repeating part), start index of residu)
    std::pair<std::pair<size_t, size_t>, size_t> find_rep(const vector<bool>& bools);
}

#endif