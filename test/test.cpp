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
using namespace fin_nim;

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

void test_fin_nim(void) {
    for (int i = 0; i < 7; i++) {
        TEST_CHECK(fin_nim_add(1 << i,1 << (i + 1)) == (1 << i) + (1 << (i + 1)));
    }

    TEST_CHECK(fin_nim_mul(2,2) == 3);
    TEST_CHECK(fin_nim_mul(2,3) == 1);
    TEST_CHECK(fin_nim_mul(2,4) == 8);

    TEST_CHECK(fin_to_2_pow(123456) == (std::vector<uint8_t>{16, 15, 14, 13, 9, 6}));
}

void test_kappa_component(void) {
    kappa_component kc(1, 2, 3);
    TEST_CHECK(kc.get_exponent() == 3);
    TEST_CHECK(kc.get_k() == 1);
    TEST_CHECK(kc.get_n() == 2);
    TEST_CHECK(kc.get_p() == 5);

    kappa_component kc1(kc);
    TEST_CHECK(kc1.get_exponent() == 3);
    TEST_CHECK(kc1.get_k() == 1);
    TEST_CHECK(kc1.get_n() == 2);
    TEST_CHECK(kc1.get_p() == 5);

    TEST_CHECK(kc == kc1);

    kappa_component kc2(2, 3, 4);
    TEST_CHECK(kc > kc2);

    kc2 = kc1;
    TEST_CHECK(kc == kc2);
}

void test_ww(void) {
    ww zero = 0;
    ww one(1);
    ww two = ww(2);
    TEST_CHECK(zero < one);
    TEST_CHECK(two > zero);
    TEST_CHECK(one == one);
    TEST_CHECK(one <= two);

    ww result = two * two + two;
    TEST_CHECK(result != two);
    ww result_copy = result;
    TEST_CHECK(result_copy == 6);

    result += one;
    TEST_CHECK(result == 7);

    ww omega({{1, 1}});
    TEST_CHECK(zero < omega);
    TEST_CHECK(one < omega);
    TEST_CHECK(two < omega);
    TEST_CHECK(result < omega);
    TEST_CHECK(omega == ww());

    result += omega;
    TEST_CHECK(result == omega);
    result_copy = result_copy * omega;
    TEST_CHECK(result_copy == omega);
}

void test_www(void) {
    www zero = 0;
    www one(1);
    www two = www(2);
    TEST_CHECK(zero < one);
    TEST_CHECK(two > zero);
    TEST_CHECK(one == one);
    TEST_CHECK(one <= two);

    www result = two * two + two;
    TEST_CHECK(result != two);
    www result_copy = result;
    TEST_CHECK(result_copy == 6);

    result += one;
    TEST_CHECK(result == 7);

    www omega({{ww(1), 1}});
    TEST_CHECK(zero < omega);
    TEST_CHECK(one < omega);
    TEST_CHECK(two < omega);
    TEST_CHECK(result < omega);
    TEST_CHECK(omega == www(ww()));

    result += omega;
    TEST_CHECK(result == omega);
    result_copy = result_copy * omega;
    TEST_CHECK(result_copy == omega);
}

TEST_LIST = {
    { "test miscelaneous", test_miscelaneous },
    { "test prime generator", test_prime_generator },
    { "test number theoretic functions", test_nt_funcs },
    { "test finite nimbers", test_fin_nim },
    { "test kappa components", test_kappa_component },
    { "test nimbers below w^w", test_ww },
    { "test nimbers below w^(w^w)", test_www },
    { NULL, NULL }     /* zeroed record marking the end of the list */
};