#include "ww.hpp"

#include <cstdint>
#include <string>
#include <iostream>
#include <list>

using std::string;
using std::ostream;
using std::list;

ww::ww(): terms({{1, 1}}) {}

ww::ww(uint16_t a): terms({{0, a}}) {
    if (a == 0) { // we can simplify more
        terms = {};
    }
}

ww::ww(const list<std::pair<uint16_t, uint16_t>>& terms): terms(ww::simp_terms(terms)) {}

list<std::pair<uint16_t, uint16_t>> ww::simp_terms(const list<std::pair<uint16_t, uint16_t>>& terms) {
    if (terms.empty()) {
        return {};
    } else if (terms.size() == 1) {
        if (terms.front().second == 0) {
            return {}; // easy simplification
        } else {
            return terms;
        }
    } else {
        auto result0 = terms.front();
        auto it = terms.begin();
        it++;
        if (result0.second == 0) {
            return simp_terms(list<std::pair<uint16_t, uint16_t>>(it, terms.end()));
        }

        // recursively let smaller values get gobbled up by bigger ones on the right
        auto tail = simp_terms(list<std::pair<uint16_t, uint16_t>>(it, terms.end()));
        auto tail_start = tail.begin();
        while (tail_start != tail.end()) {
            if (result0.first < tail_start->first) { // absorbing
                result0 = *tail_start;
                tail_start++;
            } else if (result0.first == tail_start->first) { // left-distributivity
                result0 = {result0.first, result0.second + tail_start->second};
                tail_start++;
            } else {
                break;
            }
        }
        std::list<std::pair<uint16_t, uint16_t>> result = {result0};
        result.insert(result.end(), tail_start, tail.end());
        return result;
    }
}

string ww::string_of_term(uint16_t a, uint16_t b) {
    if (a == 0 || b == 0) {
        return "" + std::to_string(b);
    } else if (a == 1) {
        if (b == 1) {
            return "w";
        } else {
            return "w*" + std::to_string(b);
        }
    } else {
        if (b == 1) {
            return "w^" + std::to_string(a);
        } else {
            return "w^" + std::to_string(a) + "*" + std::to_string(b);
        }
    }
}

string ww::to_string() const {
    if (terms.empty()) {
        return "0";
    } else {
        auto it = terms.begin();
        auto result = ww::string_of_term(it->first, it->second);
        it++;
        while (it != terms.end()) {
            result +=  " + " + ww::string_of_term(it->first, it->second);
            it++;
        }
        return result;
    }
}

const list<std::pair<uint16_t, uint16_t>>& ww::get_terms() const {
    return terms;
}

ww& ww::operator=(const ww& other) {
    terms = other.get_terms();
    return *this;
}

ww ww::operator+(const ww& other) const {
    auto result = list<std::pair<uint16_t, uint16_t>>(this->terms);
    result.insert(result.end(), other.get_terms().begin(), other.get_terms().end());
    return result;
}

ww ww::operator*(const ww& other) const {
    if (terms.empty() || other.get_terms().empty()) {
        return ww({{0, 0}});
    }
    if (terms == list<std::pair<uint16_t, uint16_t>>{{0, 1}}) {
        return ww(other);
    }
    if (other.get_terms() == list<std::pair<uint16_t, uint16_t>>{{0, 1}}) {
        return ww(*this);
    }

    auto oterms = other.get_terms();
    if (oterms.size() > 1) { // left-distributivity
        auto result = ww(0);
        for (const std::pair<uint16_t, uint16_t>& term : oterms) {
            result += *this * ww({term});
        }
        return result;
    } else { // more absorbing
        auto oterm0 = oterms.front();
        if (oterm0.first == 0) {
            auto rterms = list<std::pair<uint16_t, uint16_t>>(terms);
            rterms.front().second *= oterm0.second;
            return ww(rterms);
        } else {
            return ww({{terms.front().first + oterm0.first, oterm0.second}});
        }
    }
}

ww& ww::operator+=(const ww& other) {
    this->terms.insert(this->terms.end(), other.get_terms().begin(), other.get_terms().end());
    return *this;
}

bool operator==(const ww& me, const ww& other) {
    return me.get_terms() == other.get_terms();
}

bool operator!=(const ww& me, const ww& other) {
    return !(me.get_terms() == other.get_terms());
}

bool operator<(const ww& me, const ww& other) {
    return me.get_terms() < other.get_terms(); // list and pair compare lexicographically, exactly how we need it!
}

bool operator>(const ww& me, const ww& other) {
    return other < me;
}

bool operator<=(const ww& me, const ww& other) {
    return me == other || me < other;
}

bool operator>=(const ww& me, const ww& other) {
    return me == other || other < me;
}

ostream& operator<<(ostream& os, const ww& me) {
    return os << me.to_string(); // pretty printing
}

void ww::print() const {
    std::cout << *this << std::endl;
}