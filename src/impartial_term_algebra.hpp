#ifndef IMPARTIAL_TERM_ALGEBRA_HPP
#define IMPARTIAL_TERM_ALGEBRA_HPP

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

using std::size_t;
using std::vector;
using boost::multiprecision::cpp_int;

struct term_array {
    uint32_t terms_size;
    uint32_t* terms;
};

/* q is used when something is related to non-trivial prime powers */
class impartial_term_algebra {
    private:
        vector<uint16_t> q_components; // 16 bits will suffice here for now, also see "prime_generator.hpp"
        uint16_t* q_degrees; // ditto
        uint32_t* basis; // multiplying everything together from `q_degrees` might need more than 16 bits
        uint32_t term_count;
        uint64_t* accumulator; // using uint64_t to represent 64 contiguous bits
        term_array* kappa_table; // some entries come from `basis`
        term_array*** q_power_times_term_table; // ditto
        // maybe give q_power_times_term_table type uint32_t**** and keep sizes in a new member

        term_array q_power_times_term(size_t q_index, uint16_t q_exponent, uint32_t term); // same `uint16_t` as for `q_degrees`
        term_array q_power_times_term_calc(size_t q_index, uint16_t q_exponent, uint32_t term); // ditto
        term_array term_times_term(uint32_t x, uint32_t y);

        inline void flip_accumulator_term(uint32_t x);
        inline bool accumulator_contains(uint32_t x);
        inline void clear_accumulator();
        void accumulate_term_product(uint32_t x, uint32_t y);
    public:
        impartial_term_algebra(vector<uint16_t>& q_components);
        ~impartial_term_algebra();

        const vector<uint16_t>& get_q_components() const;
        const uint32_t get_term_count() const;
        uint32_t* const get_basis() const;

        vector<uint32_t> multiply(const vector<uint32_t>& aterms, const vector<uint32_t>& bterms);
        vector<uint32_t> square(const vector<uint32_t>& terms);
        vector<uint32_t> power(const vector<uint32_t>& terms, const cpp_int& n);
};

#endif