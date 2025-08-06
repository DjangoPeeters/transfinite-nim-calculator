#ifndef WWW_HPP
#define WWW_HPP

#include "ww.hpp"

#include <cstdint>
#include <string>
#include <ostream>
#include <list>

using std::string;
using std::ostream;
using std::list;

class www {
    private:
        // alpha(193) already has a finite summand of `2 << (2 << 4)`
        list<std::pair<ww, uint64_t>> terms; // `uint64_t` because we want to be able to represent the natural `2 << (2 << 5)`
    public:
        www();
        www(uint64_t a);
        www(ww a);
        www(const list<std::pair<ww, uint64_t>>& terms);
        www(const www& other) = default;
        ~www() = default;

        static list<std::pair<ww, uint64_t>> simp_terms(const list<std::pair<ww, uint64_t>>& terms);
        static string string_of_term(ww a, uint64_t b);

        string to_string() const;
        
        const list<std::pair<ww, uint64_t>>& get_terms() const;

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