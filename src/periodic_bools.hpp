#ifndef PERIODIC_BOOLS_HPP
#define PERIODIC_BOOLS_HPP

#include <vector>

namespace periodic_bools {
    std::pair<unsigned, std::vector<bool>> vb_euclid(std::vector<bool> bools); // returns how many times some big part repeats
}

#endif