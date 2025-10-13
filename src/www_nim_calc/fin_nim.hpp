#ifndef FIN_NIM_HPP
#define FIN_NIM_HPP

#include <cstdint>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using std::vector;
using uint256_t = boost::multiprecision::uint256_t;

namespace fin_nim {
    uint256_t fin_nim_add(uint256_t a, uint256_t b);
    uint256_t fin_nim_mul(uint256_t a, uint256_t b);
    vector<uint8_t> fin_to_2_pow(uint256_t n);
}

#endif