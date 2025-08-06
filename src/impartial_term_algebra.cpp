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
q_components(q_components_), q_degrees(vector<uint16_t>(q_components.size())),
basis(vector<uint32_t>(q_components.size() + 1)), kappa_table(vector<vector<uint32_t>>(q_components.size())),
q_power_times_term_table(vector<vector<vector<vector<uint32_t>>>>(q_components.size())) {
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
    accumulator = vector<bool>(term_count, false);

    for (size_t i = 0; i < q_components.size(); i++) {
        if (q_degrees[i] == 2) {
            kappa_table[i] = {basis[i] - 1, basis[i]};
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
            kappa_table[i] = kappa_blocks;
        } else {
            kappa_table[i] = {basis[i - 1]};
        }
    }

    for (size_t i = 0; i < q_components.size(); i++) {
        q_power_times_term_table[i] = vector<vector<vector<uint32_t>>>(q_degrees[i]);
        for (size_t j = 0; j < q_degrees[i]; j++) {
            q_power_times_term_table[i][j] = vector<vector<uint32_t>>(term_count);
            for (size_t k = 0; k < term_count; k++) {
                // can't be `null` or similar so let's make it invalid for our usecase
                // (which demands uniqueness of every value in the vector)
                // lol
                q_power_times_term_table[i][j][k] = {0, 0};
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

vector<uint32_t> impartial_term_algebra::q_power_times_term(size_t q_index, uint16_t q_exponent, uint32_t term) {
    vector<uint32_t> cached(q_power_times_term_table[q_index][q_exponent][term]);
    if (cached != vector<uint32_t>{0, 0}) {
        return cached;
    } else {
        vector<uint32_t> result(q_power_times_term_calc(q_index, q_exponent, term));
        q_power_times_term_table[q_index][q_exponent][term] = result;
        return result;
    }
}

vector<uint32_t> impartial_term_algebra::q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term) {
    uint16_t p = q_degrees[q_index];
    uint16_t q_exponent_in_term = (uint16_t)((term % basis[q_index + 1]) / basis[q_index]); // see headerfile why I'm casting
    uint16_t q_exponent_new = q_exponent + q_exponent_in_term;
    if (q_exponent_new < p) {
        return {term + (uint32_t)q_exponent * basis[q_index]};
    } else {
        uint32_t high_order_part = (term / basis[q_index + 1]) * basis[q_index + 1] + (q_exponent_new % p) * basis[q_index];
        uint32_t low_order_part = term % basis[q_index];
        vector<uint32_t> kappa_expansion(kappa_table[q_index]);

        set<uint32_t> terms{};
        for (uint32_t term : kappa_expansion) {
            for (uint32_t product_term : term_times_term(low_order_part, term)) {
                if (terms.find(product_term) != terms.end()) {
                    terms.erase(product_term);
                } else {
                    terms.insert(product_term);
                }
            }
        }

        vector<uint32_t> result{};
        for (uint32_t k : terms) result.push_back(high_order_part + k); // set is internally sorted so this for-loop uses the right order
        return result;
    }
}

vector<uint32_t> impartial_term_algebra::term_times_term(uint32_t x, uint32_t y) {
    set<uint32_t> terms{y};
    for (size_t xip1 = q_components.size(); xip1 > 0; xip1--) { // `xip1` is `xi + 1` because `0 - 1` will cause overflow
        uint16_t x_exp = (uint16_t)((x % basis[xip1]) / basis[xip1 - 1]); // see headerfile why I'm casting
        if (x_exp > 0) {
            set<uint32_t> new_terms{};
            for (uint32_t term : terms) {
                for (uint32_t product_term : q_power_times_term(xip1 - 1, x_exp, term)) {
                    if (new_terms.find(product_term) != new_terms.end()) {
                        new_terms.erase(product_term);
                    } else {
                        new_terms.insert(product_term);
                    }
                }
            }
            terms = new_terms;
        }
    }
    
    vector<uint32_t> result{};
    for (uint32_t k : terms) result.push_back(k);
    return result;
}

void impartial_term_algebra::accumulate_term_product(uint32_t x, uint32_t y) {
    if (y == 0) {
        accumulator[x].flip();
        return;
    } else {
        size_t index = q_components.size();
        while (y / basis[index] == 0) index--; // be careful for infinite loops
        vector<uint32_t> product(q_power_times_term(index, (uint16_t)(y / basis[index]), x)); // 0 <= `y / basis[index]` < some prime from `q_degrees`
        for (uint32_t term : product) accumulate_term_product(term, y % basis[index]);
        return;
    }
}

const vector<uint16_t>& impartial_term_algebra::get_q_components() const {
    return q_components;
}

const uint32_t impartial_term_algebra::get_term_count() const {
    return term_count;
}

const vector<uint32_t>& impartial_term_algebra::get_basis() const {
    return basis;
}

// aterms and bterms must already be sorted
vector<uint32_t> impartial_term_algebra::multiply(const vector<uint32_t>& aterms, const vector<uint32_t>& bterms) {
    for (uint32_t i = 0; i < term_count; i++) {
        accumulator[i] = false;
    }
    for (uint32_t x : aterms) {
        for (uint32_t y : bterms) {
            accumulate_term_product(x, y);
        }
    }

    vector<uint32_t> result;
    for (uint32_t i = 0; i < term_count; i++) {
        if (accumulator[i]) result.push_back(i);
    }
    return result;
}

// terms must already be sorted
vector<uint32_t> impartial_term_algebra::square(const vector<uint32_t>& terms) {
    for (uint32_t i = 0; i < term_count; i++) {
        accumulator[i] = false;
    }
    for (uint32_t x : terms) {
        accumulate_term_product(x, x);
    }

    vector<uint32_t> result;
    for (uint32_t i = 0; i < term_count; i++) {
        if (accumulator[i]) result.push_back(i);
    }
    return result;
}

// terms must already be sorted
vector<uint32_t> impartial_term_algebra::power(const vector<uint32_t>& terms, const cpp_int& n) {
    vector<uint32_t> curpow = terms;
    vector<uint32_t> result = {0};
    cpp_int i(n);
    std::time_t checkpoint_time = time(nullptr);
    while (!i.is_zero()) {
        if (bit_test(i, 0)) {
            result = multiply(result, curpow);
        }
        curpow = square(curpow);
        if (time(nullptr) - checkpoint_time >= 60) {
            auto bits_complete = msb(n) - msb(i);
            std::cout << bits_complete << "/" << (msb(n) + 1) << " bits complete ("
            << (((float)bits_complete) / (msb(n) + 1)) << "); curpow/result has "
            << curpow.size() << "/" << result.size() << " terms" << std::endl;
            checkpoint_time = time(nullptr);
        }
        i >>= 1;
    }
    return result;
}