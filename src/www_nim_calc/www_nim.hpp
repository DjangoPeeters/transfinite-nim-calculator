#ifndef WWW_NIM_HPP
#define WWW_NIM_HPP

#include "www.hpp"

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

using std::vector;
using boost::multiprecision::cpp_int;

namespace www_nim {
    inline www www_nim_add(const www& a, const www& b);
    www www_nim_mul(const www& a, const www& b);
    www www_nim_square(const www& a);
    www www_nim_pow(const www& a, const cpp_int& n);

    www kappa(uint16_t q);
    www alpha(uint16_t p);
}

#endif