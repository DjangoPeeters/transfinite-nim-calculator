#ifndef WWW_NIM_HPP
#define WWW_NIM_HPP

#include "www.hpp"

#include <cstdint>
#include <boost/multiprecision/cpp_int.hpp>

using std::vector;
using boost::multiprecision::cpp_int;

typedef struct alpha_return {
    bool failed;
    union {
        www result;
        uint32_t term_count;
    };

    alpha_return(bool f, www r, uint32_t t): failed(f) {
        if (f) {
            term_count = t;
        } else {
            new (&result) www(r);
        }
    }

    alpha_return(const alpha_return& other): failed(other.failed) {
        if (failed) {
            term_count = other.term_count;
        } else {
            new (&result) www(other.result);
        }
    }
    
    ~alpha_return() {
        if (!failed) result.~www();
    }

    alpha_return& operator=(const alpha_return& other) {
        if (this == &other) return *this;

        if (!failed) result.~www();
        failed = other.failed;
        if (failed) {
            term_count = other.term_count;
        } else {
            new (&result) www(other.result);
        }
        return *this;
    }
} alpha_return;

namespace www_nim {
    inline www www_nim_add(const www& a, const www& b);
    www www_nim_mul(const www& a, const www& b);
    www www_nim_square(const www& a);
    www www_nim_pow(const www& a, const cpp_int& n);

    www kappa(uint16_t q);
    alpha_return alpha(uint16_t p);
}

#endif