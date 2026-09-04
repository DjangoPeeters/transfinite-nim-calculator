#ifndef IMPARTIAL_TERM_ALGEBRA_HPP
#define IMPARTIAL_TERM_ALGEBRA_HPP

#include "ring_buffer_queue.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <ostream>
#include <utility>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using std::size_t;
using std::vector;
using boost::multiprecision::cpp_int;

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

    // Same as flip(), but skips the bit_count update. Whether a flip sets or clears a bit is
    // data-dependent and effectively unpredictable in a hot scatter-XOR loop (e.g.
    // square_with_table), so flip()'s branch there is a real per-call misprediction cost at
    // scale. Use this in such loops and call recompute_bit_count() once afterward instead.
    inline void flip_no_count(uint32_t idx) {
        uint64_t mask = uint64_t(1) << (idx & 63);
        words[idx >> 6] ^= mask;
    }

    inline void clear_all() {
        if (word_count) std::memset(words, 0, word_count * sizeof(uint64_t));
        bit_count = 0;
    }

    // Swaps buffers instead of copying word_count words. Requires same capacity_bits (both sides
    // of the swaps this is used for are always sized to the same algebra's term_count) — for use
    // where the source is about to be cleared/overwritten anyway (e.g. accumulator right after
    // its contents are moved out in square_with_table), so there's no need for the old `a =
    // accumulator` full-array memcpy.
    inline void swap(term_array& other) {
        assert(capacity_bits == other.capacity_bits);
        std::swap(word_count, other.word_count);
        std::swap(bit_count, other.bit_count);
        std::swap(words, other.words);
    }

    // Recomputes bit_count from scratch (branchless popcount scan) — for use after a run of
    // flip_no_count() calls.
    inline uint32_t recompute_bit_count() const {
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

// Offset is templated so tables whose total index count is provably bounded (e.g.
// square_term_table, capped at roughly 2*term_count — see small_flat_term_table below) can use
// a 4-byte offset instead of 8. That directly shrinks row_offsets, which is read once per set
// bit of the array being processed — profiling on real hardware (doduo) showed the stall in
// square_with_table's hot loop sitting right on this table's row_offsets/index_data reads, so
// this isn't a shot in the dark: it's the concrete lever that data pointed at.
template <typename Offset>
struct flat_term_table_base {
    // Row `term`'s set-bit indices live in index_data[row_offsets[term] .. row_offsets[term+1]),
    // in increasing order (guaranteed by how add_row() walks a bitset's set bits).
    std::vector<uint32_t> index_data;
    std::vector<Offset> row_offsets;     // size term_count+1
    uint32_t term_count;
    uint32_t capacity_bits;              // domain the indices are valid within (== term_count for this algebra)

    flat_term_table_base() : term_count(0), capacity_bits(0) {
        row_offsets.push_back(0);
    }

    // `index_estimate` is a rough guess at total set bits across all rows —
    // used only to size the initial reserve() and avoid reallocation churn
    // during the build loop. Getting it wrong just costs some reallocations,
    // not correctness. Pass 0 if you have no idea.
    flat_term_table_base(uint32_t term_count_, uint32_t capacity_bits_, size_t index_estimate = 0)
        : term_count(term_count_), capacity_bits(capacity_bits_) {
        row_offsets.reserve((size_t)term_count_ + 1);
        row_offsets.push_back(0);
        if (index_estimate > 0) index_data.reserve(index_estimate);
    }

    // No implicit deep copies of a structure this size.
    flat_term_table_base(const flat_term_table_base&) = delete;
    flat_term_table_base& operator=(const flat_term_table_base&) = delete;
    flat_term_table_base(flat_term_table_base&&) noexcept = default;
    flat_term_table_base& operator=(flat_term_table_base&&) noexcept = default;

    // Append one row's worth of set-bit indices, taken from any bitset type
    // that exposes for_each_set_bit (term_array, or another bitset view).
    // Must be called exactly once per term, in order 0..term_count-1 —
    // row identity comes purely from call order, not an explicit index,
    // so calling out of order or skipping a term will silently misalign
    // every row after it.
    template <typename Bitset>
    void add_row(const Bitset& bits) {
        bits.for_each_set_bit([&](uint32_t idx) {
            assert(idx < capacity_bits);
            index_data.push_back(idx);
        });
        row_offsets.push_back((Offset)index_data.size());
    }

    // Same as above, but for a row already held as a sorted, duplicate-free index list —
    // skips going through a term_array entirely when the caller already has one.
    void add_row(const std::vector<uint32_t>& sorted_indices) {
        for (uint32_t idx : sorted_indices) {
            assert(idx < capacity_bits);
            index_data.push_back(idx);
        }
        row_offsets.push_back((Offset)index_data.size());
    }

    // Visit set-bit indices of row `term`, in increasing order.
    template <typename Fn>
    inline void for_each_set_bit_in_row(uint32_t term, Fn&& fn) const {
        const Offset begin = row_offsets[term];
        const Offset end = row_offsets[term + 1];
        const uint32_t* data = index_data.data();
        for (Offset k = begin; k < end; k++) {
            fn(data[k]);
        }
    }

    inline uint32_t row_bit_count(uint32_t term) const {
        return (uint32_t)(row_offsets[term + 1] - row_offsets[term]);
    }

    // Diagnostics — e.g. the density check that confirmed this rewrite was worth doing.
    size_t total_index_count() const { return index_data.size(); }
    double average_row_density() const {
        if (term_count == 0 || capacity_bits == 0) return 0.0;
        return (double)index_data.size() / term_count / capacity_bits;
    }
    size_t bytes_used() const {
        return index_data.size() * sizeof(uint32_t) + row_offsets.size() * sizeof(Offset);
    }
};

// q_power_times_term_table's total index count is term_count * sum(degree-1), which for
// extreme algebras (many large-degree components) can exceed 4B — keep the safe, wide offset.
using flat_term_table = flat_term_table_base<uint64_t>;
// square_term_table's total index count is bounded by roughly 2*term_count (observed average
// row density), which stays well within 4B for any realistic term_count — safe to use the
// smaller offset, and it's the table actually sitting in excess_power's hot loop.
using small_flat_term_table = flat_term_table_base<uint32_t>;

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
        size_t* component_offsets; // flattens (q_index, q_exponent, term) into a single row index below
        flat_term_table q_power_times_term_table; // ditto; entries are as sparse as square_term_table's rows
        uint32_t* basis_search;
        small_flat_term_table square_term_table;

        inline size_t q_power_times_term_row(size_t q_index, uint16_t q_exponent, uint32_t term) const;
        // Scratch buffers for the two functions below — persistent (per-caller) to avoid a heap
        // alloc per call across the tens of millions of calls a large algebra's construction
        // makes. Grouped into a struct, one instance per worker thread, so
        // build_q_power_times_term_table() (see the .cpp) can parallelize row computation within
        // a q_index level: each level's rows only ever read *already-finalized* lower-q_index
        // rows of q_power_times_term_table (see that function's comment for why), so concurrent
        // calls are safe as long as each thread has its own scratch instance.
        struct calc_scratch {
            vector<uint32_t> qptc_result_, qptc_merged_, qptc_scratch_;
            vector<uint32_t> ttt_result_, ttt_next_, ttt_scratch_;
        };
        // Both of these return a reference to a field of `scratch` (qptc_result_ / ttt_result_
        // respectively), valid until the next call made with that same scratch instance.
        const vector<uint32_t>& q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term, calc_scratch& scratch) const;
        const vector<uint32_t>& term_times_term(uint32_t x, uint32_t y, calc_scratch& scratch) const;
        void build_q_power_times_term_table();

        void accumulate_term_product(uint32_t x, uint32_t y);
        void square_with_table(term_array& a, bool need_bit_count);
    public:
        impartial_term_algebra(ring_buffer_calculation_queue& log_queue, std::atomic<bool>& calculation_done,
            vector<uint16_t>& q_components);
        ~impartial_term_algebra();

        const vector<uint16_t>& get_q_components() const;
        uint32_t get_term_count() const;
        uint32_t* get_basis() const;

        term_array multiply(const term_array& a, const term_array& b);
        term_array square(const term_array& a, bool need_bit_count);
        term_array power(const term_array& a, const cpp_int& n);
        void excess_power(const term_array&a, const cpp_int& n, term_array& res);
        uint32_t degree(const term_array& a); // not sure how much space is adequate for the result
        void q_set_degree(const term_array& a, uint32_t& res);
};

#endif