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
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

using std::size_t;
using std::vector;
using std::set;
using std::cout;
using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using boost::multiprecision::bit_test;
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
    basis(new uint32_t[q_components.size() + 1]), accumulator(), kappa_table(new term_array[q_components.size()]),
    component_offsets(nullptr) {
    
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

    component_offsets = new size_t[q_components.size()];
    component_offsets[0] = 0;
    size_t q_power_times_term_table_size = 0;
    for (size_t q_index = 0; q_index < q_components.size()-1; q_index++) {
        const size_t component_size = (size_t)(q_degrees[q_index] - 1) * term_count;
        component_offsets[q_index+1] = component_offsets[q_index] + component_size;
        q_power_times_term_table_size += component_size;
    }
    q_power_times_term_table_size += (size_t)(q_degrees[q_components.size()-1] - 1) * term_count;

    // Entries here are as sparse as square_term_table's rows (see its density diagnostic) —
    // a dense term_count-sized bitset per entry would need tens of GB for realistic algebras.
    // index_estimate is just a reserve() hint (observed average ~1.3 set bits/entry; 2x for
    // headroom) — a wrong guess costs reallocations, not correctness.
    q_power_times_term_table = flat_term_table((uint32_t)q_power_times_term_table_size, term_count,
        q_power_times_term_table_size * 2);
    for (size_t q_index = 0; q_index < q_components.size(); q_index++) {
        for (uint16_t q_exp = 1; q_exp < q_degrees[q_index]; q_exp++) {
            for (uint32_t term = 0; term < term_count; term++) {
                q_power_times_term_table.add_row(q_power_times_term_calc(q_index, q_exp, term));
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

    // Same reserve()-hint reasoning as above (observed average ~2 set bits/row; 4x for headroom).
    square_term_table = flat_term_table(term_count, term_count, (size_t)term_count * 4);
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
    if (component_offsets != nullptr) delete[] component_offsets;
    component_offsets = nullptr;
}

// XOR (symmetric-difference) merges the source's set-bit indices into sorted, duplicate-free
// `dst`. `scratch` is caller-owned so repeated calls in a loop don't reallocate.
// These per-term intermediate results (unlike `accumulator`/`result` in excess_power's hot
// loop) stay sparse regardless of term_count, so tracking them as index lists rather than
// dense term_count-sized bitsets avoids doing O(term_count) work for O(1)-ish content.

// Source is one row of a flat_term_table (the row-index-keyed q_power_times_term_table).
static void xor_merge_row_into(vector<uint32_t>& dst, const flat_term_table& table, uint32_t row, vector<uint32_t>& scratch) {
    scratch.clear();
    scratch.reserve(dst.size() + table.row_bit_count(row));
    size_t a = 0;
    table.for_each_set_bit_in_row(row, [&](uint32_t idx) {
        while (a < dst.size() && dst[a] < idx) scratch.push_back(dst[a++]);
        if (a < dst.size() && dst[a] == idx) a++; // cancels
        else scratch.push_back(idx);
    });
    while (a < dst.size()) scratch.push_back(dst[a++]);
    dst.swap(scratch);
}

// Source is another already-sorted, duplicate-free index vector.
static void xor_merge_vec_into(vector<uint32_t>& dst, const vector<uint32_t>& src, vector<uint32_t>& scratch) {
    scratch.clear();
    scratch.reserve(dst.size() + src.size());
    size_t a = 0, b = 0;
    while (a < dst.size() && b < src.size()) {
        if (dst[a] < src[b]) scratch.push_back(dst[a++]);
        else if (dst[a] > src[b]) scratch.push_back(src[b++]);
        else { a++; b++; } // cancels
    }
    while (a < dst.size()) scratch.push_back(dst[a++]);
    while (b < src.size()) scratch.push_back(src[b++]);
    dst.swap(scratch);
}

inline size_t impartial_term_algebra::q_power_times_term_row(size_t q_index, uint16_t q_exponent, uint32_t term) const {
    return component_offsets[q_index] +
           (size_t)term_count * (q_exponent - 1) + // we ignore q_exponent=0, which is the trivial case result = 1 = term_array({0})
           term;
}

// Returns a reference to qptc_result_, valid until the next call. Only ever called from the
// constructor's fill loop (never reentrant), so a persistent scratch buffer is safe — and
// entries here average ~1-2 set bits, so building the result as a sparse vector end-to-end
// (rather than through a dense term_count-sized term_array) avoids O(term_count) work for
// O(1)-ish content, same reasoning as square_term_table's rows.
const vector<uint32_t>& impartial_term_algebra::q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term) {
    const uint16_t p = q_degrees[q_index];
    const uint16_t q_exponent_in_term = (uint16_t)((term % basis[q_index + 1]) / basis[q_index]); // see headerfile why I'm casting
    const uint16_t q_exponent_new = q_exponent + q_exponent_in_term;
    qptc_result_.clear();
    if (q_exponent_new < p) {
        qptc_result_.push_back(term + (uint32_t)q_exponent * basis[q_index]);
    } else {
        const uint32_t high_order_part = (term / basis[q_index + 1]) * basis[q_index + 1] + (q_exponent_new % p) * basis[q_index];
        const uint32_t low_order_part = term % basis[q_index];
        const term_array& kappa_expansion = kappa_table[q_index];

        qptc_merged_.clear();
        kappa_expansion.for_each_set_bit([&](uint32_t i) {
            xor_merge_vec_into(qptc_merged_, term_times_term(low_order_part, i), qptc_scratch_);
        });

        for (uint32_t i : qptc_merged_) qptc_result_.push_back(high_order_part + i);
    }
    return qptc_result_;
}

const vector<uint32_t>& impartial_term_algebra::term_times_term(uint32_t x, uint32_t y) {
    ttt_result_.clear();
    ttt_result_.push_back(y);
    for (size_t xip1 = q_components.size(); xip1 > 0; xip1--) { // `xip1` is `xi + 1` because `0 - 1` will cause overflow
        const uint16_t x_exp = (uint16_t)((x % basis[xip1]) / basis[xip1 - 1]); // see headerfile why I'm casting
        if (x_exp > 0) {
            ttt_next_.clear();
            for (uint32_t i : ttt_result_) {
                const uint32_t row = (uint32_t)q_power_times_term_row(xip1 - 1, x_exp, i);
                xor_merge_row_into(ttt_next_, q_power_times_term_table, row, ttt_scratch_);
            }
            ttt_result_.swap(ttt_next_);
        }
    }
    return ttt_result_;
}

void impartial_term_algebra::accumulate_term_product(uint32_t x, uint32_t y) {
    if (y == 0) {
        accumulator.flip_no_count(x);
        return;
    } else {
        const uint32_t bi = basis[basis_search[y]];
        // 0 <= `y / bi` < some prime from `q_degrees`
        const uint32_t row = (uint32_t)q_power_times_term_row(basis_search[y], (uint16_t)(y / bi), x);
        q_power_times_term_table.for_each_set_bit_in_row(row, [&](uint32_t i) {
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
    accumulator.bit_count = accumulator.recompute_bit_count();

    term_array result = accumulator;
    return result;
}

// a must have enough allocated memory for the result
void impartial_term_algebra::square_with_table(term_array& a) {
    accumulator.clear_all();
    a.for_each_set_bit([&](uint32_t i) {
        square_term_table.for_each_set_bit_in_row(i, [&](uint32_t idx) {
            accumulator.flip_no_count(idx);
        });
    });
    accumulator.bit_count = accumulator.recompute_bit_count();
    a = accumulator;
    return;
}

// a must already be sorted
term_array impartial_term_algebra::square(const term_array& a) {
    accumulator.clear_all();
    a.for_each_set_bit([&](uint32_t i) {
        square_term_table.for_each_set_bit_in_row(i, [&](uint32_t idx) {
            accumulator.flip_no_count(idx);
        });
    });
    accumulator.bit_count = accumulator.recompute_bit_count();
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
    
    unsigned index = 0;
    const unsigned msbnp1 = msb(n) + 1;
    constexpr unsigned MASK = ((unsigned)1 << PUSH_INTERVAL) - 1; // only log when first PUSH_INTERVAL bits are off

    vector<bool> vn = vector<bool>(msbnp1); // less overhead
    for (unsigned i = 0; i < msbnp1; i++) {
        vn[i] = bit_test(n, i);
    }

    // Same reserve()-hint reasoning as square_term_table's — rows here scale with a.bit_count
    // rather than a fixed constant, so the estimate follows suit (2x headroom).
    flat_term_table term_times_a_table(term_count, term_count,
        (size_t)term_count * std::max<uint32_t>(a.bit_count, 1) * 2);
    term_array term_as_array(term_count);
    for (uint32_t term = 0; term < term_count; term++) {
        term_as_array.set(term);
        term_times_a_table.add_row(multiply(term_as_array, a));
        term_as_array.clear_bit(term);
    }
    cout << "Precomputed values done." << '\n';

    while (!log_queue_.push({0, msbnp1, a.bit_count, result.bit_count})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    size_t ip1 = (size_t)msbnp1;
    while (ip1 > 0) {
        square_with_table(result);
        if (vn[ip1-1]) {
            accumulator.clear_all();
            result.for_each_set_bit([&](uint32_t i) {
                term_times_a_table.for_each_set_bit_in_row(i, [&](uint32_t idx) {
                    accumulator.flip_no_count(idx);
                });
            });
            accumulator.bit_count = accumulator.recompute_bit_count();
            result = accumulator;
        }
        index++;
        if (!(index & MASK)) { // Send progress update
            while (!log_queue_.push({index, msbnp1, a.bit_count, result.bit_count})) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
        ip1--;
    }
    while (!log_queue_.push({msbnp1, msbnp1, a.bit_count, result.bit_count})) {
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    calculation_done_ = true;
    while (!log_queue_.push({UNSIGNED_MAX, 0, 0, 0})) { // Signal completion to logger
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

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