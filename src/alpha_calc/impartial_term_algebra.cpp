#include "impartial_term_algebra.hpp"
#include "important_funcs.hpp"
#include "../number_theory/nt_funcs.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <set>
#include <ctime>
#include <thread>
#include <chrono>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>
#include <boost/math/special_functions/log1p.hpp>

using std::size_t;
using std::vector;
using std::set;
using std::cout;
using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using boost::multiprecision::bit_test;
using boost::multiprecision::cpp_dec_float_100;
using boost::multiprecision::log;
using namespace nt_funcs;

constexpr unsigned PUSH_INTERVAL = 6;

uint32_t term_count_calc(const vector<uint16_t>& q_components_) {
    vector<uint16_t> q_components = q_components_;
    sort(q_components.begin(), q_components.end(), [](uint16_t a, uint16_t b)
                                        {
                                            return prime_pow(a) < prime_pow(b);
                                        });
    q_components.erase(unique(q_components.begin(), q_components.end()), q_components.end());
    uint32_t term_count = 1;
    for (size_t i = 0; i < q_components.size(); i++) {
        term_count *= prime_pow(q_components[i]).first;
    }
    return term_count;
}

impartial_term_algebra::impartial_term_algebra(ring_buffer_calculation_queue& log_queue, std::atomic<bool>& calculation_done,
    vector<uint16_t>& q_components_): log_queue_(log_queue), calculation_done_(calculation_done),
    q_components(q_components_), q_degrees(new uint16_t[q_components.size()]),
    basis(new uint32_t[q_components.size() + 1]), accumulator(), kappa_table(new term_array[q_components.size()]) {
    
    sort(q_components.begin(), q_components.end(), [](uint16_t a, uint16_t b)
                                        {
                                            return prime_pow(a) < prime_pow(b);
                                        });
    q_components.erase(unique(q_components.begin(), q_components.end()), q_components.end());
    
    basis[0] = 1;
    for (size_t i = 0; i < q_components.size(); i++) {
        q_degrees[i] = prime_pow(q_components[i]).first;
        basis[i + 1] = basis[i] * q_degrees[i];
    }
    term_count = basis[q_components.size()];
    accumulator = term_array(term_count);

    for (size_t i = 0; i < q_components.size(); i++) {
        if (q_degrees[i] == 2) {
            kappa_table[i] = term_array(term_count);
            kappa_table[i].set(basis[i] - 1);
            kappa_table[i].set(basis[i]);
        } else if (q_components[i] == q_degrees[i]) {
            const uint16_t p = q_degrees[i];
            const auto q_set_r = important_funcs::q_set(p);
            const excess_return exr = important_funcs::excess(p);
            if (q_set_r.first != 0 || exr.failed) {
                cout << "constructing algebra failed\n";
                if (basis != nullptr) delete[] basis;
                basis = nullptr;
                if (basis_search != nullptr) delete[] basis_search;
                basis_search = nullptr;
                if (kappa_table != nullptr) delete[] kappa_table;
                kappa_table = nullptr;
                if (q_degrees != nullptr) delete[] q_degrees;
                q_degrees = nullptr;
                cout << "term_count was " << term_count << "\n";
                term_count = 0;
                return;
            }
            const vector<uint16_t> q_set = q_set_r.second;
            const uint16_t excess = exr.result;

            vector<uint32_t> kappa_blocks(vector<uint32_t>(q_set.size()));
            for (size_t j = 0; j < q_set.size(); j++) {
                kappa_blocks[j] = basis[find(q_components.begin(), q_components.end(), q_set[j]) - q_components.begin()];
            }
            if (excess != 0) {
                kappa_blocks.push_back(msb(excess));
                sort(kappa_blocks.begin(), kappa_blocks.end()); // this sorting can be done smarter I think
            }
            kappa_table[i] = term_array(term_count);
            for (uint32_t j = 0; j < kappa_blocks.size(); j++) {
                kappa_table[i].set(kappa_blocks[j]);
            }
        } else {
            kappa_table[i] = term_array(term_count);
            kappa_table[i].set(basis[i - 1]);
        }
    }

    q_power_times_term_table.component_offsets = new size_t[q_components.size()];
    q_power_times_term_table.term_count = term_count;
    q_power_times_term_table.component_offsets[0] = 0;
    size_t q_power_times_term_table_size = 0;
    for (size_t q_index = 0; q_index < q_components.size()-1; q_index++) {
        q_power_times_term_table.component_offsets[q_index+1] = q_power_times_term_table.component_offsets[q_index];
        for (size_t q_exp = 1; q_exp < q_degrees[q_index]; q_exp++) {
            for (size_t term = 0; term < term_count; term++) {
                q_power_times_term_table.component_offsets[q_index+1]++;
                q_power_times_term_table_size++;
            }
        }
    }
    for (size_t q_exp = 1; q_exp < q_degrees[q_components.size()-1]; q_exp++) {
        for (size_t term = 0; term < term_count; term++) {
            q_power_times_term_table_size++;
        }
    }
    q_power_times_term_table.data = new term_array[q_power_times_term_table_size];

    for (size_t q_index = 0; q_index < q_components.size(); q_index++) {
        for (size_t q_exp = 1; q_exp < q_degrees[q_index]; q_exp++) {
            for (size_t term = 0; term < term_count; term++) {
                q_power_times_term(q_index, q_exp, term);
            }
        }
    }

    basis_search = new uint32_t[term_count];
    basis_search[0] = 0; // dummy value
    uint32_t index = 0;
    for (uint32_t term = 1; term < term_count; term++) {
        index = q_components.size();
        while (term < basis[index]) index--;
        basis_search[term] = index;
    }

    square_term_table = flat_term_table(term_count, term_count);
    for (uint32_t term = 0; term < term_count; term++) {
        accumulator.clear_all();
        accumulate_term_product(term, term);
        square_term_table.add_row(accumulator);
    }
}

impartial_term_algebra::~impartial_term_algebra() {
    if (kappa_table != nullptr) delete[] kappa_table;
    kappa_table = nullptr;
    if (basis != nullptr) delete[] basis;
    basis = nullptr;
    if (q_degrees != nullptr) delete[] q_degrees;
    q_degrees = nullptr;
    if (basis_search != nullptr) delete[] basis_search;
    basis_search = nullptr;
}

term_array impartial_term_algebra::q_power_times_term(size_t q_index, uint16_t q_exponent, uint32_t term) {
    if (q_power_times_term_table.get(q_index, q_exponent, term).words != nullptr) {
        return q_power_times_term_table.get(q_index, q_exponent, term);
    } else {
        const term_array result = q_power_times_term_calc(q_index, q_exponent, term);
        q_power_times_term_table.get(q_index, q_exponent, term) = result;
        return result;
    }
}

term_array impartial_term_algebra::q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term) {
    const uint16_t p = q_degrees[q_index];
    const uint16_t q_exponent_in_term = (uint16_t)((term % basis[q_index + 1]) / basis[q_index]); // see headerfile why I'm casting
    const uint16_t q_exponent_new = q_exponent + q_exponent_in_term;
    if (q_exponent_new < p) {
        term_array result = term_array(term_count);
        result.set(term + (uint32_t)q_exponent * basis[q_index]);
        return result;
    } else {
        const uint32_t high_order_part = (term / basis[q_index + 1]) * basis[q_index + 1] + (q_exponent_new % p) * basis[q_index];
        const uint32_t low_order_part = term % basis[q_index];
        const term_array kappa_expansion(kappa_table[q_index]);

        term_array terms(term_count);
        
        kappa_expansion.for_each_set_bit([&](uint32_t i) {
            terms ^= term_times_term(low_order_part, i);
        });

        term_array result(term_count);
        terms.for_each_set_bit([&](uint32_t i) {
            result.set(high_order_part + i);
        });
        return result;
    }
}

term_array impartial_term_algebra::term_times_term(uint32_t x, uint32_t y) {
    term_array result(term_count);
    result.set(y);
    term_array product(term_count);
    uint16_t x_exp;
    for (size_t xip1 = q_components.size(); xip1 > 0; xip1--) { // `xip1` is `xi + 1` because `0 - 1` will cause overflow
        x_exp = (uint16_t)((x % basis[xip1]) / basis[xip1 - 1]); // see headerfile why I'm casting
        if (x_exp > 0) {
            term_array new_terms(term_count);
            result.for_each_set_bit([&](uint32_t i) {
                product = q_power_times_term(xip1 - 1, x_exp, i);
                new_terms ^= product;
            });
            result = new_terms;
        }
    }
    
    return result;
}

void impartial_term_algebra::accumulate_term_product(uint32_t x, uint32_t y) {
    if (y == 0) {
        accumulator.flip(x);
        return;
    } else {
        const uint32_t bi = basis[basis_search[y]];
        // 0 <= `y / bi` < some prime from `q_degrees`
        const tmp_term_array product = tmp_term_array(q_power_times_term_table.get(basis_search[y], (uint16_t)(y / bi), x));
        product.for_each_set_bit([&](uint32_t i) {
            accumulate_term_product(i, y % bi);
        });
        return;
    }
}

const vector<uint16_t>& impartial_term_algebra::get_q_components() const {
    return q_components;
}

uint32_t impartial_term_algebra::get_term_count() const {
    return term_count;
}

uint32_t* impartial_term_algebra::get_basis() const {
    return basis;
}

// a and b must already be sorted
term_array impartial_term_algebra::multiply(const term_array& a, const term_array& b) {
    accumulator.clear_all();
    a.for_each_set_bit([&](uint32_t i) {
        b.for_each_set_bit([&](uint32_t j) {
            accumulate_term_product(i, j);
        });
    });

    term_array result = accumulator;
    return result;
}

term_array impartial_term_algebra::square_term_calc(uint32_t x) {
    accumulator.clear_all();
    accumulate_term_product(x, x);

    term_array result = accumulator;
    return result;
}

// a must have enough allocated memory for the result
void impartial_term_algebra::square_with_table(term_array& a) {
    accumulator.clear_all();
    a.for_each_set_bit([&](uint32_t i) {
        square_term_table.for_each_set_bit_in_row(i, [&](uint32_t idx) {
            accumulator.flip(idx);
        });
    });
    a = accumulator;
    return;
}

// a must already be sorted
term_array impartial_term_algebra::square(const term_array& a) {
    accumulator.clear_all();
    a.for_each_set_bit([&](uint32_t i) {
        square_term_table.for_each_set_bit_in_row(i, [&](uint32_t idx) {
            accumulator.flip(idx);
        });
    });
    term_array result = accumulator;
    return result;
}

// a must already be sorted
term_array impartial_term_algebra::power(const term_array& a, const cpp_int& n) {
    term_array result(term_count);
    result.set(0);
    if(n.is_zero()) return result;
    
    term_array curpow(a);
    unsigned index = 0;
    const unsigned msbnp1 = msb(n) + 1;
    
    while (index < msbnp1) {
        if (bit_test(n, index)) {
            result = multiply(result, curpow);
        }
        curpow = square(curpow);
        index++;
    }
    return result;
}

//TODO optimize
// a must already be sorted
void impartial_term_algebra::excess_power(const term_array&a, const cpp_int& n, term_array& res) {
    term_array result(term_count);
    result.set(0);
    if(n.is_zero()) {
        res = result;
        return;
    }
    
    term_array curpow(a);
    unsigned index = 0;
    const unsigned msbnp1 = msb(n) + 1;
    constexpr unsigned MASK = ((unsigned)1 << PUSH_INTERVAL) - 1; // only log when first PUSH_INTERVAL bits are off

    vector<bool> vn = vector<bool>(msbnp1); // less overhead
    for (unsigned i = 0; i < msbnp1; i++) {
        vn[i] = bit_test(n, i);
    }
    
    //TODO optimize (maybe multithreading the multiplication of powers of `tmp`)
    /* test: sliding-window; doesn't really help nor hurt...
    cpp_dec_float_100 lnnm1 = log(cpp_dec_float_100(n)) - 1;
    cpp_int twotokp1 = 4, fourtok = 4;
    size_t k = 1;
    while (lnnm1 >= cpp_dec_float_100(k * (k+1) * fourtok) / cpp_dec_float_100(twotokp1 - k - 2)) {
        k++;
        twotokp1 << 1;
        fourtok << 2;
    }
    cout << "Sliding-window with k = " << k << '\n';

    size_t odd_powers_size = (size_t)((1 << (k-1)) - 1);
    term_array* odd_powers = new term_array[odd_powers_size];
    term_array sqa = square(a);
    if (odd_powers_size != 0) {
        odd_powers[0] = multiply(a, sqa);
        for (size_t i = 1; i < odd_powers_size; i++) {
            odd_powers[i] = multiply(odd_powers[i-1], sqa);
        }
    }
    cout << "Precomputed values done." << '\n';

    while (!log_queue_.push({0, msbnp1, curpow.terms_size, result.terms_size})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    size_t ip1 = (size_t)msbnp1, s = 0, u = 0, pow2 = 0;
    while (ip1 > 0) {
        if (!vn[ip1-1]) {
            square_with_table(result);
            index++;
            if (!(index & MASK)) { // Send progress update
                while (!log_queue_.push({index, msbnp1, curpow.terms_size, result.terms_size})) {
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
            ip1--;
        } else {
            if (ip1 > k) {
                s = ip1 - k;
            } else {
                s = 0;
            }
            while (!vn[s]) s++;
            for (size_t h = s; h < ip1; h++) {
                square_with_table(result);
                index++;
                if (!(index & MASK)) { // Send progress update
                    while (!log_queue_.push({index, msbnp1, curpow.terms_size, result.terms_size})) {
                        std::this_thread::sleep_for(std::chrono::microseconds(10));
                    }
                }
            }
            u = 0;
            pow2 = 1;
            for (size_t h = s; h < ip1; h++) {
                if (vn[h]) {
                    u += pow2;
                }
                pow2 <<= 1;
            }
            if (u == 1) {
                result = multiply(result, a);
            } else {
                result = multiply(result, odd_powers[(u-3) >> 1]);
            }
            ip1 = s;
        }
    }
    while (!log_queue_.push({msbnp1, msbnp1, curpow.terms_size, result.terms_size})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    calculation_done_ = true;
    while (!log_queue_.push({UNSIGNED_MAX, 0, 0, 0})) { // Signal completion to logger
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    delete[] odd_powers;
    odd_powers = nullptr;
    */

    term_array* term_times_a = new term_array[term_count];
    term_array term_as_array(term_count);
    for (uint32_t term = 0; term < term_count; term++) {
        term_as_array.set(term);
        term_times_a[term] = multiply(term_as_array, a);
        term_as_array.clear_bit(term);
    }
    cout << "Precomputed values done." << '\n';

    while (!log_queue_.push({0, msbnp1, curpow.bit_count, result.bit_count})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    size_t ip1 = (size_t)msbnp1;
    tmp_term_array tmp;
    while (ip1 > 0) {
        square_with_table(result);
        if (vn[ip1-1]) {
            accumulator.clear_all();
            result.for_each_set_bit([&](uint32_t i) {
                tmp = tmp_term_array(term_times_a[i]);
                accumulator.merge_xor_no_count(tmp);
            });
            accumulator.bit_count = accumulator.recompute_bit_count();
            result = accumulator;
        }
        index++;
        if (!(index & MASK)) { // Send progress update
            while (!log_queue_.push({index, msbnp1, curpow.bit_count, result.bit_count})) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
        ip1--;
    }
    while (!log_queue_.push({msbnp1, msbnp1, curpow.bit_count, result.bit_count})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    calculation_done_ = true;
    while (!log_queue_.push({UNSIGNED_MAX, 0, 0, 0})) { // Signal completion to logger
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    delete[] term_times_a;
    term_times_a = nullptr;

    res = result;
    return;
}

// a must already be sorted
uint32_t impartial_term_algebra::degree(const term_array& a) {
    term_array respow = square(a);
    uint32_t result = 1;
    while (respow != a) {
        respow = square(respow);
        result++;
    }
    return result;
}

// a must already be sorted
void impartial_term_algebra::q_set_degree(const term_array& a, uint32_t& res) {
    term_array respow = square(a);
    uint32_t result = 1;
    constexpr unsigned MASK = ((unsigned)1 << PUSH_INTERVAL) - 1; // only log when first PUSH_INTERVAL bits are off

    while (!log_queue_.push({0, 0, respow.bit_count, 0})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    while (respow != a) {
        respow = square(respow);
        result++;
        if (!(result & MASK)) { // Send progress update
            while (!log_queue_.push({result, 0, respow.bit_count, 0})) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
    while (!log_queue_.push({result, result, respow.bit_count, 0})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    calculation_done_ = true;
    while (!log_queue_.push({UNSIGNED_MAX, 0, 0, 0})) { // Signal completion to logger
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    res = result; return;
}