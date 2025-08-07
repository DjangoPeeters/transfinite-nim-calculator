#include "impartial_term_algebra.hpp"
#include "nt_funcs.hpp"
#include "important_funcs.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <set>
#include <ctime>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

using std::size_t;
using std::vector;
using std::set;
using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using boost::multiprecision::bit_test;
using namespace nt_funcs;

impartial_term_algebra::impartial_term_algebra(vector<uint16_t>& q_components_):
q_components(q_components_), q_degrees(new uint16_t[q_components.size()]),
basis(new uint32_t[q_components.size() + 1]), kappa_table(new term_array[q_components.size()]),
q_power_times_term_table(new term_array**[q_components.size()]) {
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
    accumulator = new uint64_t[(term_count + 63) >> 6];
    for (uint32_t i = 0; i < ((term_count + 63) >> 6); i++) {
        accumulator[i] = 0;
    }

    for (size_t i = 0; i < q_components.size(); i++) {
        if (q_degrees[i] == 2) {
            kappa_table[i] = {2, new uint32_t[2]{basis[i] - 1, basis[i]}};
        } else if (q_components[i] == q_degrees[i]) {
            uint16_t p = q_degrees[i];
            vector<uint16_t> q_set(important_funcs::q_set(p));
            uint16_t excess = important_funcs::excess(p);

            vector<uint32_t> kappa_blocks(vector<uint32_t>(q_set.size()));
            for (size_t j = 0; j < q_set.size(); j++) {
                kappa_blocks[j] = basis[find(q_components.begin(), q_components.end(), q_set[j]) - q_components.begin()];
            }
            if (excess != 0) {
                kappa_blocks.push_back(msb(excess));
                sort(kappa_blocks.begin(), kappa_blocks.end()); // this sorting can be done smarter I think
            }
            kappa_table[i] = {(uint32_t)(kappa_blocks.size()), new uint32_t[kappa_blocks.size()]};
            for (uint32_t j = 0; j < kappa_blocks.size(); j++) {
                kappa_table[i].terms[j] = kappa_blocks[j];
            }
        } else {
            kappa_table[i] = {1, new uint32_t[1]{basis[i - 1]}};
        }
    }

    for (size_t i = 0; i < q_components.size(); i++) {
        q_power_times_term_table[i] = new term_array*[q_degrees[i]];
        for (size_t j = 0; j < q_degrees[i]; j++) {
            q_power_times_term_table[i][j] = new term_array[term_count];
            for (size_t k = 0; k < term_count; k++) {
                q_power_times_term_table[i][j][k] = {0, nullptr};
            }
        }
    }

    for (size_t q_index = 0; q_index < q_components.size(); q_index++) {
        for (size_t q_exp = 1; q_exp < q_degrees[q_index]; q_exp++) {
            for (size_t term = 0; term < term_count; term++) {
                q_power_times_term(q_index, q_exp, term);
            }
        }
    }
}

impartial_term_algebra::~impartial_term_algebra() {
    for (size_t q_index = 0; q_index < q_components.size(); q_index++) {
        for (size_t q_exp = 0; q_exp < q_degrees[q_index]; q_exp++) {
            for (size_t term = 0; term < term_count; term++) {
                delete q_power_times_term_table[q_index][q_exp][term].terms;
                q_power_times_term_table[q_index][q_exp][term] = {0, nullptr};
            }
            delete[] q_power_times_term_table[q_index][q_exp];
            q_power_times_term_table[q_index][q_exp] = nullptr;
        }
        delete[] q_power_times_term_table[q_index];
        q_power_times_term_table[q_index] = nullptr;
        delete kappa_table[q_index].terms;
        kappa_table[q_index] = {0, nullptr};
    }
    delete[] q_power_times_term_table;
    q_power_times_term_table = nullptr;
    delete[] kappa_table;
    kappa_table = nullptr;

    delete accumulator;
    accumulator = nullptr;
    delete basis;
    basis = nullptr;
    delete q_degrees;
    q_degrees = nullptr;
}

term_array impartial_term_algebra::q_power_times_term(size_t q_index, uint16_t q_exponent, uint32_t term) {
    term_array cached = q_power_times_term_table[q_index][q_exponent][term];
    if (cached.terms != nullptr) {
        return cached;
    } else {
        term_array result = q_power_times_term_calc(q_index, q_exponent, term);
        q_power_times_term_table[q_index][q_exponent][term] = result;
        return result;
    }
}

term_array impartial_term_algebra::q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term) {
    uint16_t p = q_degrees[q_index];
    uint16_t q_exponent_in_term = (uint16_t)((term % basis[q_index + 1]) / basis[q_index]); // see headerfile why I'm casting
    uint16_t q_exponent_new = q_exponent + q_exponent_in_term;
    if (q_exponent_new < p) {
        return {1, new uint32_t[1]{term + (uint32_t)q_exponent * basis[q_index]}};
    } else {
        uint32_t high_order_part = (term / basis[q_index + 1]) * basis[q_index + 1] + (q_exponent_new % p) * basis[q_index];
        uint32_t low_order_part = term % basis[q_index];
        term_array kappa_expansion = kappa_table[q_index];

        set<uint32_t> terms{};
        term_array product = {0, nullptr};
        for (uint32_t i = 0; i < kappa_expansion.terms_size; i++) {
            product = term_times_term(low_order_part, kappa_expansion.terms[i]);
            for (uint32_t j = 0; j < product.terms_size; j++) {
                if (terms.find(product.terms[j]) != terms.end()) {
                    terms.erase(product.terms[j]);
                } else {
                    terms.insert(product.terms[j]);
                }
            }
        }

        term_array result = {(uint32_t)(terms.size()), new uint32_t[terms.size()]};
        uint32_t i = 0;
        for (uint32_t k : terms) { // set is internally sorted so this for-loop uses the right order
            result.terms[i] = high_order_part + k;
            i++;
        }
        return result;
    }
}

term_array impartial_term_algebra::term_times_term(uint32_t x, uint32_t y) {
    set<uint32_t> terms{y};
    term_array product = {0, nullptr};
    for (size_t xip1 = q_components.size(); xip1 > 0; xip1--) { // `xip1` is `xi + 1` because `0 - 1` will cause overflow
        uint16_t x_exp = (uint16_t)((x % basis[xip1]) / basis[xip1 - 1]); // see headerfile why I'm casting
        if (x_exp > 0) {
            set<uint32_t> new_terms{};
            for (uint32_t term : terms) {
                product = q_power_times_term(xip1 - 1, x_exp, term);
                for (uint32_t i = 0; i < product.terms_size; i++) {
                    if (new_terms.find(product.terms[i]) != new_terms.end()) {
                        new_terms.erase(product.terms[i]);
                    } else {
                        new_terms.insert(product.terms[i]);
                    }
                }
            }
            terms = new_terms;
        }
    }
    
    term_array result = {(uint32_t)(terms.size()), new uint32_t[terms.size()]};
    uint32_t i = 0;
    for (uint32_t k : terms) {
        result.terms[i] = k;
        i++;
    }
    return result;
}

inline void impartial_term_algebra::flip_accumulator_term(uint32_t x) {
    accumulator[x / 64] ^= ((uint64_t)1) << (x & 63);
}

inline bool impartial_term_algebra::accumulator_contains(uint32_t x) {
    return (accumulator[x / 64] & (((uint64_t)1) << (x & 63))) != 0;
}

inline void impartial_term_algebra::clear_accumulator() {
    for (uint32_t i = 0; i < ((term_count + 63) >> 6); i++) {
        accumulator[i] = 0;
    }
}

void impartial_term_algebra::accumulate_term_product(uint32_t x, uint32_t y) {
    if (y == 0) {
        flip_accumulator_term(x);
        return;
    } else {
        size_t index = q_components.size();
        while (y / basis[index] == 0) index--; // be careful for infinite loops
        term_array product = q_power_times_term(index, (uint16_t)(y / basis[index]), x); // 0 <= `y / basis[index]` < some prime from `q_degrees`
        for (uint32_t i = 0; i < product.terms_size; i++) {
            accumulate_term_product(product.terms[i], y % basis[index]);
        }
        return;
    }
}

const vector<uint16_t>& impartial_term_algebra::get_q_components() const {
    return q_components;
}

const uint32_t impartial_term_algebra::get_term_count() const {
    return term_count;
}

uint32_t* const impartial_term_algebra::get_basis() const {
    return basis;
}

// aterms and bterms must already be sorted
vector<uint32_t> impartial_term_algebra::multiply(const vector<uint32_t>& aterms, const vector<uint32_t>& bterms) {
    clear_accumulator();
    for (uint32_t x : aterms) {
        for (uint32_t y : bterms) {
            accumulate_term_product(x, y);
        }
    }

    vector<uint32_t> result;
    for (uint32_t i = 0; i < term_count; i++) {
        if (accumulator_contains(i)) result.push_back(i);
    }
    return result;
}

// terms must already be sorted
vector<uint32_t> impartial_term_algebra::square(const vector<uint32_t>& terms) {
    clear_accumulator();
    for (uint32_t x : terms) {
        accumulate_term_product(x, x);
    }

    vector<uint32_t> result;
    for (uint32_t i = 0; i < term_count; i++) {
        if (accumulator_contains(i)) result.push_back(i);
    }
    return result;
}

//TODO optimize
// terms must already be sorted
vector<uint32_t> impartial_term_algebra::power(const vector<uint32_t>& terms, const cpp_int& n) {
    if(n.is_zero()) return {0};
    vector<uint32_t> curpow(terms);
    vector<uint32_t> result{0};
    unsigned index = 0;
    unsigned msbnp1 = msb(n) + 1;
    std::time_t checkpoint_time = time(nullptr);
    while (index < msbnp1) {
        if (bit_test(n, index)) {
            result = multiply(result, curpow);
        }
        curpow = square(curpow);
        if (time(nullptr) - checkpoint_time >= 60) {
            std::cout << index << "/" << msbnp1 << " bits complete ("
            << (((float)index) / msbnp1) << "); curpow/result has "
            << curpow.size() << "/" << result.size() << " terms" << std::endl;
            checkpoint_time = time(nullptr);
        }
        index++;
    }
    return result;
}