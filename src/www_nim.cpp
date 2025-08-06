#include "www_nim.hpp"
#include "prime_generator.hpp"
#include "nt_funcs.hpp"
#include "important_funcs.hpp"
#include "fin_nim.hpp"
#include "ww.hpp"
#include "www.hpp"
#include "kappa_component.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

using std::size_t;
using std::vector;
using std::list;
using std::sort;
using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using boost::multiprecision::bit_test;
using namespace prime_generator;

namespace www_nim {
    namespace {
        vector<uint8_t> fin_to_2_pow(uint64_t n) {
            if (n == 0) return {};
            
            vector<uint8_t> result{};
            for (uint8_t i = msb(n); i > 0; i--) {
                if (n & (((uint64_t)1) << i) != 0) {
                    result.push_back(i);
                }
            }
            if (n & 1 != 0) result.push_back(0);
            return result;
        }

        vector<ww> www_to_2_pow(const www& x) {
            vector<ww> result{};
            for (const auto [a, b] : x.get_terms()) {
                ww c = ww() * a;
                for (const uint8_t k : fin_to_2_pow(b)) {
                    result.push_back(c + k);
                }
            }
            return result;
        }

        www www_of_2_pow(const vector<ww>& x) {
            www result(0);
            www temp(1);
            for (const ww xi : x) {
                temp = 1;
                for (const auto [a, b] : xi.get_terms()) {
                    if (a == 0) {
                        temp = temp * (((uint64_t)1) << b);
                    } else {
                        temp = temp * www({{ww({{a - 1, b}}), 1}});
                    }
                }
                result += temp;
            }
            return result;
        }

        vector<ww> xor_list_ww(const vector<ww>& al, const vector<ww>& bl) { // al and bl are sorted from big to small (Cantor-Normal-Form)
            vector<ww> result{};
            size_t ia = 0, ib = 0;
            while (ia < al.size() && ib < bl.size()) {
                if (al[ia] > bl[ib]) {
                    result.push_back(al[ia]);
                    ia++;
                } else if (al[ia] < bl[ib]) {
                    result.push_back(bl[ib]);
                    ib++;
                } else {
                    ia++;
                    ib++;
                }
            }

            while (ia < al.size()) {
                result.push_back(al[ia]);
                ia++;
            }
            while (ib < bl.size()) {
                result.push_back(bl[ib]);
                ib++;
            }
            return result;
        }

        www www_of_kappa_components(const vector<kappa_component>& components, uint64_t coefficient) {
            list<std::pair<uint16_t, uint16_t>> terms{};
            uint16_t temp1, temp2;
            for (const auto component : components) {
                temp1 = nth_prime(component.get_k() + 2);
                temp2 = 1;
                for (uint16_t i = 0; i < component.get_n(); i++) temp2 *= temp1;
                terms.push_back({component.get_k(), temp2 * component.get_exponent()});
            }
            return www({{ww(terms), coefficient}});
        }

        vector<kappa_component> kappa_components_of_term(const std::pair<ww, uint64_t>& a) {
            vector<kappa_component> result{};
            for (const auto [exponent, coefficient] : a.first.get_terms()) {
                uint16_t k = exponent;
                uint16_t p = nth_prime(exponent + 2);
                uint16_t n = 1;
                uint16_t pn = p;
                while (pn <= coefficient) {
                    n++;
                    pn *= p;
                }
                uint16_t temp = 1;
                for (uint16_t i = 0; i < n; i++) {
                    k = (coefficient / temp) % p;
                    if (k != 0) result.push_back(kappa_component(exponent, i, k));
                    temp *= p;
                }
            }
            return result;
        }

        www reduce_components(const vector<kappa_component>& components,
            const vector<kappa_component>& processed_components, uint64_t coefficient) {
            if (components.empty()) return www_of_kappa_components(processed_components, coefficient);
            if (components.size() > 1 && components[0].get_k() == components[1].get_k() && components[0].get_n() == components[1].get_n()) {
                auto a = components[0], b = components[1];
                if (a.get_exponent() + b.get_exponent() < a.get_p()) {
                    vector<kappa_component> new_components{kappa_component(a.get_k(), a.get_n(), a.get_exponent() + b.get_exponent())};
                    auto it = components.begin();
                    it++; it++;
                    new_components.insert(new_components.end(), it, components.end());
                    return reduce_components(new_components, processed_components, coefficient);
                } else {
                    vector<kappa_component> new_components{};
                    if (a.get_exponent() + b.get_exponent() != a.get_p()) {
                        new_components.push_back(kappa_component(a.get_k(), a.get_n(), a.get_exponent() + b.get_exponent() - a.get_p()));
                    }

                    if (a.get_n() > 0) {
                        new_components.push_back(kappa_component(a.get_k(), a.get_n() - 1, 1));
                        auto it = components.begin();
                        it++; it++;
                        new_components.insert(new_components.end(), it, components.end());
                        sort(new_components.begin(), new_components.end());
                        return reduce_components(new_components, processed_components, coefficient);
                    } else {
                        www alpha1 = alpha(nth_prime(a.get_k() + 2));

                        www result(0);
                        for (const auto [exp, coef] : alpha1.get_terms()) {
                            if (exp == 0) {
                                auto it = components.begin();
                                it++; it++;
                                new_components.insert(new_components.end(), it, components.end());

                                result = www_nim_add(result, reduce_components(new_components, processed_components,
                                    fin_nim::fin_nim_mul(coefficient, coef)));
                            } else {
                                auto kappa_comps = kappa_components_of_term({exp, coef});
                                new_components.insert(new_components.end(), kappa_comps.begin(), kappa_comps.end());
                                auto it = components.begin();
                                it++; it++;
                                new_components.insert(new_components.end(), it, components.end());
                                sort(new_components.begin(), new_components.end());

                                result = www_nim_add(result,
                                    reduce_components(new_components, processed_components, coefficient));
                            }
                        }
                        return result;
                    }
                }
            }

            // now we have components.size() >= 1
            vector<kappa_component> new_components{};
            auto it = components.begin();
            it++;
            new_components.insert(new_components.end(), it, components.end());
            vector<kappa_component> new_processed_components{components[0]};
            new_processed_components.insert(new_processed_components.end(),
                processed_components.begin(), processed_components.end());
            return reduce_components(new_components, new_processed_components, coefficient);
        }

        www www_nim_mul_term(const std::pair<ww, uint64_t>& a, const std::pair<ww, uint64_t>& b) {
            vector<kappa_component> components = kappa_components_of_term(a), other = kappa_components_of_term(b);
            components.insert(components.end(), other.begin(), other.end());
            sort(components.begin(), components.end());
            return reduce_components(components, {}, fin_nim::fin_nim_mul(a.second, b.second));
        }
    }

    inline www www_nim_add(const www& a, const www& b) {
        return www_of_2_pow(xor_list_ww(www_to_2_pow(a), www_to_2_pow(b)));
    }

    www www_nim_mul(const www& a, const www& b) {
        if (a == 0 || b == 0) return 0;
        if (a == 1) return b;
        if (b == 1) return a;

        www result(0);
        for (const auto ap : a.get_terms()) {
            for (const auto bp : b.get_terms()) {
                result = www_nim_add(result, www_nim_mul_term(ap, bp));
            }
        }
        return result;
    }

    www www_nim_square(const www& a) {
        www result(0);
        for (const auto ap : a.get_terms()) {
            result = www_nim_add(result, www_nim_mul_term(ap, ap));
        }
        return result;
    }

    www www_nim_pow(const www& a, const cpp_int& n) {
        cpp_int N(n);
        www curpow(a), respow(1);

        while (N > 0) {
            if (bit_test(N, 0)) {
                respow = www_nim_mul(respow, curpow);
            }
            curpow = www_nim_square(curpow);
            N >>= 1;
        }
        return respow;
    }

    www kappa(uint16_t q) { // q should be a non-trivial prime power
        auto [p, n] = nt_funcs::prime_pow(q);
        if (p == 2) {
            // Fermat 2-power
            return www(((uint64_t)1)<<(((uint16_t)1)<<(n-1)));
        }
        uint16_t k = (uint16_t)prime_pi(p) - 1; // 0-indexed for historical reasons
        uint16_t pn1 = 1;
        for (uint16_t i = 1; i < n; i++) pn1 *= p;
        return www({{ww({{k-1, pn1}}), 1}});
    }

    www alpha(uint16_t p) {
        www result(important_funcs::excess(p));
        for (auto q : important_funcs::q_set(p)) {
            result = kappa(q) + result;
        }
        return result;
    }
};