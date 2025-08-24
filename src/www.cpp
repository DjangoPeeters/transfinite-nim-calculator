#include "www.hpp"
#include "ww.hpp"

#include <cstdint>
#include <string>
#include <iostream>
#include <list>

using std::string;
using std::ostream;
using std::list;

www::www(): terms({{ww(), 1}}) {}

www::www(uint64_t a): terms({{0, a}}) {
    if (a == 0) { // we can simplify more
        terms = {};
    }
}

www::www(ww a): terms(list<std::pair<ww, uint64_t>>(a.get_terms().size())) {
    auto ita = a.get_terms().begin();
    for (auto it = terms.begin(); it != terms.end(); it++) {
        *it = {ww(ita->first), ita->second};
        ita++;
    }
}

www::www(const list<std::pair<ww, uint64_t>>& terms): terms(www::simp_terms(terms)) {}

list<std::pair<ww, uint64_t>> www::simp_terms(const list<std::pair<ww, uint64_t>>& terms) {
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
            return simp_terms(list<std::pair<ww, uint64_t>>(it, terms.end()));
        }

        // recursively let smaller values get gobbled up by bigger ones on the right
        auto tail = simp_terms(list<std::pair<ww, uint64_t>>(it, terms.end()));
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
        list<std::pair<ww, uint64_t>> result = {result0};
        result.insert(result.end(), tail_start, tail.end());
        return result;
    }
}

string www::string_of_term(ww a, uint64_t b) {
    if (a == 0 || b == 0) {
        return std::to_string(b);
    } else if (a == 1) {
        if (b == 1) {
            return "w";
        } else {
            return "w*" + std::to_string(b);
        }
    } else {
        string stra = a.to_string();
        if (a.get_terms().size() > 1 || stra.find_first_of('*') != string::npos || stra.find_first_of('^') != string::npos) stra = "(" + stra + ")";
        if (b == 1) {
            return "w^" + stra;
        } else {
            return "w^" + stra + "*" + std::to_string(b);
        }
    }
}

string www::to_string() const {
    if (terms.empty()) {
        return "0";
    } else {
        auto it = terms.begin();
        auto result = www::string_of_term(it->first, it->second);
        it++;
        while (it != terms.end()) {
            result +=  " + " + www::string_of_term(it->first, it->second);
            it++;
        }
        return result;
    }
}

const list<std::pair<ww, uint64_t>>& www::get_terms() const {
    return terms;
}

www& www::operator=(const www& other) {
    terms = other.get_terms();
    return *this;
}

www www::operator+(const www& other) const {
    auto result = list<std::pair<ww, uint64_t>>(this->terms);
    result.insert(result.end(), other.get_terms().begin(), other.get_terms().end());
    return result;
}

www www::operator*(const www& other) const {
    if (terms.empty() || other.get_terms().empty()) return 0;
    if (terms == list<std::pair<ww, uint64_t>>{{0, 1}}) return www(other);
    if (other.get_terms() == list<std::pair<ww, uint64_t>>{{0, 1}}) return www(*this);

    auto oterms = other.get_terms();
    if (oterms.size() > 1) { // left-distributivity
        www result(0);
        for (const std::pair<ww, uint64_t>& term : oterms) {
            result += *this * www({term});
        }
        return result;
    } else { // more absorbing
        auto oterm0 = oterms.front();
        if (oterm0.first == 0) {
            auto rterms = list<std::pair<ww, uint64_t>>(terms);
            rterms.front().second *= oterm0.second;
            return www(rterms);
        } else {
            return www({{terms.front().first + oterm0.first, oterm0.second}});
        }
    }
}

www& www::operator+=(const www& other) {
    this->terms.insert(this->terms.end(), other.get_terms().begin(), other.get_terms().end());
    return *this;
}

bool operator==(const www& me, const www& other) {
    return me.terms == other.terms;
}

bool operator!=(const www& me, const www& other) {
    return !(me.terms == other.terms);
}

bool operator<(const www& me, const www& other) {
    return me.terms < other.terms; // list and pair compare lexicographically, exactly how we need it!
}

bool operator>(const www& me, const www& other) {
    return other < me;
}

bool operator<=(const www& me, const www& other) {
    return me == other || me < other;
}

bool operator>=(const www& me, const www& other) {
    return me == other || other < me;
}

ostream& operator<<(ostream& os, const www& me) {
    return os << me.to_string(); // pretty printing
}

void www::print() const {
    std::cout << *this << std::endl;
}