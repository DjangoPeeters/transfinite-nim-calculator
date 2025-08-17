#include "kappa_component.hpp"
#include "prime_generator.hpp"

#include <iostream>

using std::ostream;

kappa_component::kappa_component(uint16_t k, uint16_t n, uint16_t exponent):
k(k), n(n), exponent(exponent), p(prime_generator::nth_prime(k + 2)) {}

uint16_t kappa_component::get_k() const {
    return k;
}

uint16_t kappa_component::get_n() const {
    return n;
}

uint16_t kappa_component::get_exponent() const {
    return exponent;
}

uint16_t kappa_component::get_p() const {
    return p;
}

kappa_component& kappa_component::operator=(const kappa_component& other) {
    k = other.get_k();
    n = other.get_n();
    exponent = other.get_exponent();
    p = other.get_p();
    return *this;
}

bool operator==(const kappa_component& me, const kappa_component& other) {
    return me.k == other.k && me.n == other.n && me.exponent == other.exponent;
}

bool operator<(const kappa_component& me, const kappa_component& other) {
    if (me.k != other.k) {
        return me.k > other.k;
    } else if (me.n != other.n) {
        return me.n > other.n;
    } else {
        return me.exponent < other.exponent;
    }
}

bool operator>(const kappa_component& me, const kappa_component& other) {
    return other < me;
}

bool operator<=(const kappa_component& me, const kappa_component& other) {
    return me == other || me < other;
}

bool operator>=(const kappa_component& me, const kappa_component& other) {
    return me == other || other < me;
}

ostream& operator<<(ostream& os, const kappa_component& me) {
    return os << "KappaComponent(" << me.k << ", " << me.n << ", " << me.exponent << ")";
}