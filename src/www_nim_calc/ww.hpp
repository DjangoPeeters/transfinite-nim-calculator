#ifndef WW_HPP
#define WW_HPP

#include <cstdint>
#include <string>
#include <ostream>
#include <list>

using std::string;
using std::ostream;
using std::list;

class ww {
    private:
        list<std::pair<uint16_t, uint16_t>> terms;
    public:
        ww();
        ww(uint16_t a);
        ww(const list<std::pair<uint16_t, uint16_t>>& terms);
        ww(const ww& other) = default;
        ~ww() = default;

        static list<std::pair<uint16_t, uint16_t>> simp_terms(const list<std::pair<uint16_t, uint16_t>>& terms);
        static string string_of_term(uint16_t a, uint16_t b);
        
        string to_string() const;
        
        const list<std::pair<uint16_t, uint16_t>>& get_terms() const;

        ww& operator=(const ww& other);
        ww operator+(const ww& other) const;
        ww operator*(const ww& other) const;
        ww& operator+=(const ww& other);
        friend bool operator==(const ww& me, const ww& other);
        friend bool operator!=(const ww& me, const ww& other);
        friend bool operator<(const ww& me, const ww& other);
        friend bool operator>(const ww& me, const ww& other);
        friend bool operator<=(const ww& me, const ww& other);
        friend bool operator>=(const ww& me, const ww& other);
        friend ostream& operator<<(ostream& os, const ww& me);

        void print() const;
};

#endif