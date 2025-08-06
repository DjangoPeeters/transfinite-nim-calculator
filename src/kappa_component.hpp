#ifndef KAPPA_COMPONENT_HPP
#define KAPPA_COMPONENT_HPP

#include <cstdint>
#include <ostream>

using std::ostream;

class kappa_component { // maybe nest this class inside `www_nim` because it'll only be used there?
    private:
        uint16_t k;
        uint16_t n;
        uint16_t exponent;
        uint16_t p;
    public:
        kappa_component(uint16_t k, uint16_t n, uint16_t exponent);
        kappa_component(const kappa_component& other) = default;
        ~kappa_component() = default;

        const uint16_t get_k() const;
        const uint16_t get_n() const;
        const uint16_t get_exponent() const;
        const uint16_t get_p() const;

        kappa_component& operator=(const kappa_component& other);
        friend bool operator==(const kappa_component& me, const kappa_component& other);
        friend bool operator<(const kappa_component& me, const kappa_component& other);
        friend bool operator>(const kappa_component& me, const kappa_component& other);
        friend bool operator<=(const kappa_component& me, const kappa_component& other);
        friend bool operator>=(const kappa_component& me, const kappa_component& other);
        friend ostream& operator<<(ostream& os, const kappa_component& me);
};

#endif