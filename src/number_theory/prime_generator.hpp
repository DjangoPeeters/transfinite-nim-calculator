#ifndef PRIME_GENERATOR_HPP
#define PRIME_GENERATOR_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>

using std::size_t;
using std::vector;

// if you're gonna work with primes greater than 65535, then use an unsigned int type that uses more than 16 bits
namespace prime_generator {
    uint16_t nth_prime(size_t n); // 1-indexed for historical reasons
    size_t prime_pi(uint16_t n);
        
    vector<uint16_t> primes(uint16_t start, uint16_t end);
}

#endif