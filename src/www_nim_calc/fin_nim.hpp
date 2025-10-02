#ifndef FIN_NIM_HPP
#define FIN_NIM_HPP

#include <boost/multiprecision/cpp_int.hpp>

using uint256_t = boost::multiprecision::uint256_t;

namespace fin_nim {
    inline uint256_t fin_nim_add(uint256_t a, uint256_t b);
    uint256_t fin_nim_mul(uint256_t a, uint256_t b);
}

#endif