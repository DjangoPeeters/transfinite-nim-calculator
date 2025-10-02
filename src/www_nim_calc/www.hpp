#ifndef WWW_HPP
#define WWW_HPP

#include "ww.hpp"

#include <cstdint>
#include <string>
#include <ostream>
#include <list>
#include <boost/multiprecision/cpp_int.hpp>

using std::string;
using std::ostream;
using std::list;
using uint256_t = boost::multiprecision::uint256_t;

class www {
    private:
        // alpha(769) already has a finite summand of `2 << (2 << 6)`
        list<std::pair<ww, uint256_t>> terms; // `uint256_t` because we want to be able to represent the natural `2 << (2 << 7)`
    public:
        www();
        www(unsigned int a);
        www(uint256_t a);
        www(ww a);
        www(const list<std::pair<ww, uint256_t>>& terms);
        www(const www& other) = default;
        ~www() = default;

        static list<std::pair<ww, uint256_t>> simp_terms(const list<std::pair<ww, uint256_t>>& terms);
        static string string_of_term(ww a, uint256_t b);

        string to_string() const;
        
        const list<std::pair<ww, uint256_t>>& get_terms() const;

        www& operator=(const www& other);
        www operator+(const www& other) const;
        www operator*(const www& other) const;
        www& operator+=(const www& other);
        friend bool operator==(const www& me, const www& other);
        friend bool operator!=(const www& me, const www& other);
        friend bool operator<(const www& me, const www& other);
        friend bool operator>(const www& me, const www& other);
        friend bool operator<=(const www& me, const www& other);
        friend bool operator>=(const www& me, const www& other);
        friend ostream& operator<<(ostream& os, const www& me);

        void print() const;
};

#endif