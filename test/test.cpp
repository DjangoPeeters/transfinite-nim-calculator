#include "acutest.h"
#include "../src/alpha_calc/calculation_logger.hpp"
#include "../src/alpha_calc/constants.hpp"
#include "../src/alpha_calc/impartial_term_algebra.hpp"
#include "../src/alpha_calc/important_funcs.hpp"
#include "../src/alpha_calc/ring_buffer_queue.hpp"
#include "../src/number_theory/nt_funcs.hpp"
#include "../src/number_theory/prime_generator.hpp"
#include "../src/www_nim_calc/fin_nim.hpp"
#include "../src/www_nim_calc/kappa_component.hpp"
#include "../src/www_nim_calc/ww.hpp"
#include "../src/www_nim_calc/www.hpp"
#include "../src/www_nim_calc/www_nim.hpp"
#include "../src/misc.hpp"

#include <utility>

using namespace prime_generator;

void test_miscelaneous(void) {
    TEST_CHECK(strtou16("123") == (uint16_t)123);
    TEST_CHECK(strtou32("0123") == (uint32_t)123);
    TEST_CHECK(strtosize("00123") == (size_t)123);
}

void test_prime_generator(void) {
    TEST_CHECK(nth_prime(1) == 2);
    TEST_CHECK(nth_prime(2) == 3);
    TEST_CHECK(nth_prime(3) == 5);

    TEST_CHECK(prime_pi(100) == 25);

    for (const uint16_t p : primes(10, 100)) {
        TEST_CHECK(10 < p && p < 100);
    }
}

void test_nt_funcs(void) {
    TEST_CHECK(nt_funcs::f(3) == 2);
    TEST_CHECK(nt_funcs::f(5) == 4);
    TEST_CHECK(nt_funcs::f(7) == 3);
    TEST_CHECK(nt_funcs::f(9) == 6);

    TEST_CHECK(!nt_funcs::div_2_pow_min_1(11,19));
    TEST_CHECK(nt_funcs::div_2_pow_min_1(11,20));
    TEST_CHECK(!nt_funcs::div_2_pow_min_1(11,21));

    TEST_CHECK(!nt_funcs::is_3_pow(242));
    TEST_CHECK(nt_funcs::is_3_pow(243));
    TEST_CHECK(!nt_funcs::is_3_pow(244));

    TEST_CHECK(nt_funcs::max_pow_min_prime_div(2*2*3) == (std::pair<uint16_t, uint16_t>{2, 2*2}));
    TEST_CHECK(nt_funcs::max_pow_min_prime_div(3*3*7) == (std::pair<uint16_t, uint16_t>{3, 3*3}));
    TEST_CHECK(nt_funcs::max_pow_min_prime_div(5*5*13) == (std::pair<uint16_t, uint16_t>{5, 5*5}));

    TEST_CHECK(nt_funcs::prime_pow(2) == (std::pair<uint16_t, uint16_t>{2, 1}));
    TEST_CHECK(nt_funcs::prime_pow(3*3) == (std::pair<uint16_t, uint16_t>{3, 2}));
    TEST_CHECK(nt_funcs::prime_pow(5*5*5) == (std::pair<uint16_t, uint16_t>{5, 3}));
}

TEST_LIST = {
    { "test miscelaneous", test_miscelaneous },
    { "test prime_generator", test_prime_generator },
    { "test nt_funcs", test_nt_funcs },
    { NULL, NULL }     /* zeroed record marking the end of the list */
};