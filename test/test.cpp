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

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

using namespace prime_generator;
using namespace fin_nim;
using namespace www_nim;
using important_funcs::excess;
using important_funcs::q_set;
using std::cout;
using std::vector;

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

void test_www_nim(void) {
    TEST_CHECK(alpha(2).result == 0);
    TEST_CHECK(alpha(3).result == 2);
    TEST_CHECK(alpha(5).result == 4);
    TEST_CHECK(alpha(7).result == ww() + 1);
    TEST_CHECK(alpha(11).result == www() + 1);
    TEST_CHECK(alpha(47).result == www({{ww({{7,1}}),1}}) + 1);
    
    TEST_CHECK(kappa(4) == 4);
    TEST_CHECK(kappa(8) == 16);
    TEST_CHECK(kappa(3) == ww());
    TEST_CHECK(kappa(9) == ww({{3,1}}));
    TEST_CHECK(kappa(5) == www());

    TEST_CHECK(www_nim_add(1, 2) == 3);
    TEST_CHECK(www_nim_add(ww(), ww()*2 + 1) == ww()*3 + 1);

    TEST_CHECK(www_nim_mul(2, 3) == 1);
    TEST_CHECK(www_nim_mul(ww(), ww()*ww()) == 2);
    TEST_CHECK(www_nim_mul(www(), www()*www()*www()*www()) == 4);

    TEST_CHECK(www_nim_square(2) == 3);
    TEST_CHECK(www_nim_square(4) == 6);
    TEST_CHECK(www_nim_square(ww()) == ww()*ww());
    TEST_CHECK(www_nim_square(ww() + 2) == ww()*ww() + 3);

    TEST_CHECK(www_nim_pow(ww(), 3) == 2);
    TEST_CHECK(www_nim_pow(ww(), 4) == ww()*2);
    TEST_CHECK(www_nim_pow(www(), 5) == 4);
    TEST_CHECK(www_nim_pow(www()*ww(), 15) == 9);
}

void test_important_funcs(void) {
    important_funcs::init();
    TEST_CHECK(excess(2).result == 0);
    TEST_CHECK(excess(3).result == 0);
    TEST_CHECK(excess(5).result == 0);
    TEST_CHECK(excess(7).result == 1);
    TEST_CHECK(excess(11).result == 1);
    TEST_CHECK(excess(13).result == 0);
    TEST_CHECK(excess(17).result == 0);
    TEST_CHECK(excess(19).result == 4);

    TEST_CHECK(q_set(2).second == vector<uint16_t>{});
    TEST_CHECK(q_set(3).second == vector<uint16_t>{2});
    TEST_CHECK(q_set(5).second == vector<uint16_t>{4});
    TEST_CHECK(q_set(7).second == vector<uint16_t>{3});
    TEST_CHECK(q_set(11).second == vector<uint16_t>{5});
    TEST_CHECK(q_set(13).second == vector<uint16_t>({4, 3}));
    TEST_CHECK(q_set(17).second == vector<uint16_t>{8});
    TEST_CHECK(q_set(19).second == vector<uint16_t>{9});
}

void test_impartial_term_algebra(void) {
    important_funcs::init();
    vector<uint16_t> q_components{2, 4, 3, 5}; // closed under primitive_components

    // Shared resources
    ring_buffer_calculation_queue log_queue;
    std::atomic<bool> calculation_done{false};

    // Create objects
    impartial_term_algebra algebra(log_queue, calculation_done, q_components); //TODO make constructor faster?
    calculation_logger logger(log_queue, calculation_done, logs_dir + "/calculation.log");

    cpp_int poww = cpp_int(1) << algebra.get_term_count();
    term_array random(3);
    for (int i = 0; i < 8; i++) {
        random.terms[0] = 1+i;
        random.terms[1] = 2+i;
        random.terms[2] = 3+i;

        TEST_CHECK(!(algebra.power(random, poww) != random));
    }
}

// should we constants.hpp, calculation_logger.hpp, ring_buffer_queue.hpp?

TEST_LIST = {
    { "test miscelaneous", test_miscelaneous },
    { "test prime generator", test_prime_generator },
    { "test number theoretic functions", test_nt_funcs },
    { "test finite nimbers", test_fin_nim },
    { "test kappa components", test_kappa_component },
    { "test ordinals below w^w", test_ww },
    { "test ordinals below w^(w^w)", test_www },
    { "test nimbers below w^(w^w)", test_fin_nim },
    { "test important functions", test_important_funcs },
    { "test impartial term algebra", test_impartial_term_algebra },
    { NULL, NULL }     /* zeroed record marking the end of the list */
};