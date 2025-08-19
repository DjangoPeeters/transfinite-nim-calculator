#ifndef PERIODIC_BOOLS_HPP
#define PERIODIC_BOOLS_HPP

#include <vector>

using std::vector;

namespace periodic_bools {
    // pad right with `false` as much as original has max streak of `false`
    vector<bool> pad_right(const vector<bool>& bools);

    vector<bool> strip_right(const vector<bool>& bools);
    
    // returns how many times some big part repeats
    std::pair<std::pair<std::size_t, vector<bool>>, vector<bool>> vb_euclid(const vector<bool>& bools);
}

#endif