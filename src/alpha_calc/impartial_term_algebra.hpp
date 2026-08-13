#ifndef IMPARTIAL_TERM_ALGEBRA_HPP
#define IMPARTIAL_TERM_ALGEBRA_HPP

#include "ring_buffer_queue.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <ostream>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using std::size_t;
using std::vector;
using boost::multiprecision::cpp_int;

struct tmp_term_array;

struct term_array {
    uint32_t capacity_bits;   // domain size (== term_count for this algebra)
    uint32_t word_count;      // ceil(capacity_bits / 64)
    uint32_t bit_count;       // number of set bits (replaces old terms_size)
    uint64_t* words;

    static uint32_t words_for(uint32_t bits) {
        return (bits + 63u) / 64u;
    }

    term_array() : capacity_bits(0), word_count(0), bit_count(0), words(nullptr) {}

    // Allocates a zero-initialized bitset over `capacity_bits` terms.
    explicit term_array(uint32_t capacity_bits_)
        : capacity_bits(capacity_bits_),
          word_count(words_for(capacity_bits_)),
          bit_count(0),
          words(word_count ? new uint64_t[word_count]() : nullptr) {} // value-init zeroes

    term_array(const term_array& other)
        : capacity_bits(other.capacity_bits),
          word_count(other.word_count),
          bit_count(other.bit_count),
          words(nullptr) {
        if (word_count > 0) {
            words = new uint64_t[word_count];
            std::memcpy(words, other.words, word_count * sizeof(uint64_t));
        }
    }

    term_array(term_array&& other) noexcept
        : capacity_bits(other.capacity_bits),
          word_count(other.word_count),
          bit_count(other.bit_count),
          words(other.words) {
        other.words = nullptr;
        other.capacity_bits = 0;
        other.word_count = 0;
        other.bit_count = 0;
    }

    ~term_array() {
        delete[] words;
        words = nullptr;
    }

    term_array& operator=(const term_array& other) {
        if (this != &other) {
            if (word_count != other.word_count) {
                delete[] words;
                word_count = other.word_count;
                words = word_count ? new uint64_t[word_count] : nullptr;
            }
            capacity_bits = other.capacity_bits;
            bit_count = other.bit_count;
            if (word_count > 0) {
                std::memcpy(words, other.words, word_count * sizeof(uint64_t));
            }
        }
        return *this;
    }

    term_array& operator=(term_array&& other) noexcept {
        if (this != &other) {
            delete[] words;
            capacity_bits = other.capacity_bits;
            word_count = other.word_count;
            bit_count = other.bit_count;
            words = other.words;
            other.words = nullptr;
            other.capacity_bits = 0;
            other.word_count = 0;
            other.bit_count = 0;
        }
        return *this;
    }

    // --- bit ops ---

    inline bool test(uint32_t idx) const {
        return (words[idx >> 6] & (uint64_t(1) << (idx & 63))) != 0;
    }

    inline void set(uint32_t idx) {
        uint64_t mask = uint64_t(1) << (idx & 63);
        uint64_t& w = words[idx >> 6];
        if (!(w & mask)) { w |= mask; bit_count++; }
    }

    inline void clear_bit(uint32_t idx) {
        uint64_t mask = uint64_t(1) << (idx & 63);
        uint64_t& w = words[idx >> 6];
        if (w & mask) { w &= ~mask; bit_count--; }
    }

    // Same job as the old flip_accumulator_term, single load instead of load+reload.
    inline void flip(uint32_t idx) {
        uint64_t mask = uint64_t(1) << (idx & 63);
        uint64_t& w = words[idx >> 6];
        uint64_t before = w;
        w = before ^ mask;
        bit_count += (before & mask) ? -1 : 1;
    }

    inline void clear_all() {
        if (word_count) std::memset(words, 0, word_count * sizeof(uint64_t));
        bit_count = 0;
    }

    // Merge another bitset into this one via XOR (word-wise; auto-vectorizes).
    // Both must share the same capacity/word_count.
    term_array& operator^=(const term_array& other) {
        assert(word_count == other.word_count);
        int64_t delta = 0;
        for (uint32_t w = 0; w < word_count; w++) {
            uint64_t before = words[w];
            uint64_t after = before ^ other.words[w];
            words[w] = after;
            delta += __builtin_popcountll(after) - __builtin_popcountll(before);
        }
        bit_count = (uint32_t)((int64_t)bit_count + delta);
        return *this;
    }

    term_array& operator^=(const tmp_term_array& other);

    // Optional cross-check / recovery if you ever suspect incremental drift.
    uint32_t recompute_bit_count() const {
        uint32_t c = 0;
        for (uint32_t w = 0; w < word_count; w++) c += __builtin_popcountll(words[w]);
        return c;
    }

    friend bool operator!=(const term_array& me, const term_array& other) {
        if (me.word_count != other.word_count) return true;
        return std::memcmp(me.words, other.words, me.word_count * sizeof(uint64_t)) != 0;
    }

    // Visit set bit indices in increasing order — for code that still needs
    // the sorted-index view (e.g. final output, or interfacing with
    // q_power_times_term_table which is keyed by term index).
    template <typename F>
    void for_each_set_bit(F&& fn) const {
        for (uint32_t w = 0; w < word_count; w++) {
            uint64_t word = words[w];
            uint32_t base = w * 64u;
            while (word) {
                unsigned b = __builtin_ctzll(word);
                fn(base + b);
                word &= word - 1; // clear lowest set bit
            }
        }
    }
};

struct tmp_term_array { // only for transferring `term_array`'s
    uint32_t capacity_bits;
    uint32_t word_count;
    uint32_t bit_count;
    uint64_t* words;

    tmp_term_array() : capacity_bits(0), word_count(0), bit_count(0), words(nullptr) {}

    tmp_term_array(const term_array& other)
        : capacity_bits(other.capacity_bits),
          word_count(other.word_count),
          bit_count(other.bit_count),
          words(other.words) {
    }

    ~tmp_term_array() {
        capacity_bits = 0;
        word_count = 0;
        bit_count = 0;
        words = nullptr;
    }

    tmp_term_array& operator=(const tmp_term_array& other) {
        if (this != &other) {
            capacity_bits = other.capacity_bits;
            word_count = other.word_count;
            bit_count = other.bit_count;
            words = other.words;
        }
        return *this;
    }

    // Visit set bit indices in increasing order — for code that still needs
    // the sorted-index view (e.g. final output, or interfacing with
    // q_power_times_term_table which is keyed by term index).
    template <typename F>
    void for_each_set_bit(F&& fn) const {
        for (uint32_t w = 0; w < word_count; w++) {
            uint64_t word = words[w];
            uint32_t base = w * 64u;
            while (word) {
                unsigned b = __builtin_ctzll(word);
                fn(base + b);
                word &= word - 1; // clear lowest set bit
            }
        }
    }
};

inline term_array& term_array::operator^=(const tmp_term_array& other) {
    assert(word_count == other.word_count);
    int64_t delta = 0;
    for (uint32_t w = 0; w < word_count; w++) {
        uint64_t before = words[w];
        uint64_t after = before ^ other.words[w];
        words[w] = after;
        delta += __builtin_popcountll(after) - __builtin_popcountll(before);
    }
    bit_count = (uint32_t)((int64_t)bit_count + delta);
    return *this;
}

struct flattened_table {
    term_array* data;
    size_t* component_offsets;
    uint32_t term_count;

    flattened_table(): data(nullptr), component_offsets(nullptr), term_count(0) {}

    ~flattened_table() {
        if (data != nullptr) delete[] data;
        data = nullptr;
        if (component_offsets != nullptr) delete[] component_offsets;
        component_offsets = nullptr;
        term_count = 0;
    }
    
    term_array& get(size_t component, size_t degree, size_t term_idx) {
        size_t offset = component_offsets[component] + 
                       term_count*(degree-1) + // we ignore degree=0 because this corresponds to the trivial case with result 1 = term_array({0})
                       term_idx;
        return data[offset];
    }
};

uint32_t term_count_calc(const vector<uint16_t>& q_components);

/* q is used when something is related to non-trivial prime powers */
class impartial_term_algebra {
    private:
        ring_buffer_calculation_queue& log_queue_;
        std::atomic<bool>& calculation_done_;

        /** WARNING: q_components is supposed to represent prime powers such that
         * the corresponding algebra generated by the kappa(q)'s doesn't contain
         * kappa(Q)'s where Q is not present among q_components.
         * Example: impartial_term_algebra(..., ..., {2, 3, 5}) won't work as expected because
         * kappa(5)^5 = 4 which can't be written as a sum of products of kappa(2),kappa(3),kappa(5).
         */
        vector<uint16_t> q_components; // 16 bits will suffice here for now, also see "prime_generator.hpp"
        uint16_t* q_degrees; // ditto
        uint32_t* basis; // multiplying everything together from `q_degrees` might need more than 16 bits
        uint32_t term_count;
        term_array accumulator;
        term_array* kappa_table; // some entries come from `basis`
        flattened_table q_power_times_term_table; // ditto
        uint32_t* basis_search;
        term_array* square_term_table;

        term_array q_power_times_term(size_t q_index, uint16_t q_exponent, uint32_t term); // same `uint16_t` as for `q_degrees`
        term_array q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term); // ditto
        term_array term_times_term(uint32_t x, uint32_t y);

        inline void flip_accumulator_term(uint32_t x);
        inline bool accumulator_contains(uint32_t x);
        inline void clear_accumulator();
        void accumulate_term_product(uint32_t x, uint32_t y);
        term_array square_term_calc(uint32_t x);
        void square_with_table(term_array& a);
    public:
        impartial_term_algebra(ring_buffer_calculation_queue& log_queue, std::atomic<bool>& calculation_done,
            vector<uint16_t>& q_components);
        ~impartial_term_algebra();

        const vector<uint16_t>& get_q_components() const;
        uint32_t get_term_count() const;
        uint32_t* get_basis() const;

        term_array multiply(const term_array& a, const term_array& b);
        term_array square(const term_array& a);
        term_array power(const term_array& a, const cpp_int& n);
        void excess_power(const term_array&a, const cpp_int& n, term_array& res);
        uint32_t degree(const term_array& a); // not sure how much space is adequate for the result
        void q_set_degree(const term_array& a, uint32_t& res);
};

#endif