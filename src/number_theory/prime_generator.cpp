#include "prime_generator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>
#include <mutex>

using std::size_t;
using std::vector;

namespace prime_generator {
    namespace {
        // initial few primes
        vector<uint16_t> primes_cache = {2};
        std::mutex primes_cache_mutex;

        uint16_t next_prime() { // private method so no mutex lock
            uint16_t nextp = primes_cache.back();
            bool is_prime = false;
            while (!is_prime) {
                nextp++;
                is_prime = true;
                for (const uint16_t p : primes_cache) {
                    if (p * p > nextp) break;
                    if (nextp % p == 0) {
                        is_prime = false;
                        break;
                    }
                }
            }
            primes_cache.push_back(nextp);
            return nextp;
        }
    }

    uint16_t nth_prime(size_t n) {
        size_t index = n - 1;
        std::lock_guard<std::mutex> lock(primes_cache_mutex);
        if (index < primes_cache.size()) return primes_cache[index];

        for (size_t N = primes_cache.size(); N < n; N++) next_prime();
        return primes_cache[index];
    }

    size_t prime_pi(uint16_t n) {
        std::lock_guard<std::mutex> lock(primes_cache_mutex);
        while (primes_cache.back() < n) next_prime();

        size_t result = 0;
        for (uint16_t p : primes_cache) { // only works because we store the primes in ascending order
            if (n < p) break;
            else result++;
        }
        return result;
    }

    vector<uint16_t> primes(uint16_t start, uint16_t end) {
        std::lock_guard<std::mutex> lock(primes_cache_mutex);
        while (primes_cache.back() < end) next_prime();

        auto start_it = lower_bound(primes_cache.begin(), primes_cache.end(), start);
        auto end_it = lower_bound(start_it, primes_cache.end(), end);
        
        return vector<uint16_t>(start_it, end_it);
    }
};