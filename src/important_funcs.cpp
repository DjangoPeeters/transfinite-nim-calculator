#include "important_funcs.hpp"
#include "nt_funcs.hpp"
#include "impartial_term_algebra.hpp"
#include "constants.hpp"

#include <cstdint>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <boost/multiprecision/integer.hpp>

using std::vector;
using std::set;
using std::map;
using std::sort;
using std::unique;
using std::cout;
using std::endl;
using boost::multiprecision::msb;
using namespace nt_funcs;

namespace important_funcs {
    namespace {
        map<uint16_t, vector<uint16_t>> q_set_cache(test_values::q_set_cache);
        map<uint16_t, uint8_t> excess_cache(test_values::excess_cache); // every excess for primes p <= 257 is at most 4

        uint64_t finite_summand(uint16_t p, uint16_t excess) {
            vector<uint16_t> q_set1 = q_set(p);
            uint64_t base_finite_summand = 0;
            if (!q_set1.empty() && (q_set1[0] & 1) == 0) {
                base_finite_summand = ((uint64_t)1) << (q_set1[0] >> 1);
            }
            return base_finite_summand + (uint64_t)excess;
        }

        vector<uint16_t> finite_components(uint16_t p, uint16_t excess) {
            uint64_t finite_summand1 = finite_summand(p, excess);
            if (finite_summand1 <= 1) {
                return {};
            } else {
                vector<uint16_t> result;
                for (uint8_t k = 0; k <= msb(msb(finite_summand1)); k++) result.push_back(((uint16_t)1) << (k + 1)); // k <= 5 because we chose `uint64_t`
                return result;
            }
        }

        set<uint16_t> primitive_components(uint16_t q) {
            uint16_t p = prime_pow(q).first;
            set<uint16_t> components;
            uint16_t pn = 1;
            while (pn < q) {
                pn *= p;
                components.insert(pn);
            }

            for (uint16_t q1 : q_set(p)) components.merge(primitive_components(q1));
            vector<uint16_t> fin_comps = finite_components(p, (uint16_t)excess(p));
            components.insert(fin_comps.begin(), fin_comps.end());
            return components;
        }

        vector<uint16_t> kappa_set(uint16_t h) {
            auto [p, q] = max_pow_min_prime_div(h);
            if (h == q) return {h};

            uint16_t g = h/q;
            vector<uint16_t> kappag_set(kappa_set(g));
            vector<uint16_t> components{};
            for (auto q1 : kappag_set) {
                for (auto n : primitive_components(q1)) {
                    components.push_back(n);
                }
            }
            sort(components.begin(), components.end(), [](uint16_t a, uint16_t b)
                                        {
                                            return prime_pow(a) < prime_pow(b);
                                        });
            components.erase(unique(components.begin(), components.end()), components.end());

            cout << "Computing the degree of kappa(" << g << ") (components = {" << components[0];
            for (size_t i = 1; i < components.size(); i++) {
                cout << ", " << components[i];
            }
            cout << "})." << endl;

            impartial_term_algebra algebra(components);
            cout << "Field has exponent " << algebra.get_term_count() << "." << endl;
            term_array kappag_in_algebra((uint32_t)kappag_set.size());
            uint32_t i = 0;
            for (const auto r : kappag_set) {
                kappag_in_algebra.terms[i] = algebra.get_basis()[find(algebra.get_q_components().begin(),
                    algebra.get_q_components().end(), r) - algebra.get_q_components().begin()];
                i++;
            }

            term_array respow = algebra.square(kappag_in_algebra);
            uint64_t degree = 1; // not sure how much space is adequate for this variable
            while (respow != kappag_in_algebra) {
                respow = algebra.square(respow);
                degree++;
            }
            cout << "Degree is " << degree << "." << endl;

            if (degree % q == 0) {
                return kappag_set;
            } else {
                kappag_set.insert(kappag_set.begin(), q); // prime powers are stored from lowest prime to highest prime
                return kappag_set;
            }
        }
    }

    map<uint16_t, vector<uint16_t>> get_q_set_cache() {
        return q_set_cache;
    }

    vector<uint16_t> q_set(uint16_t p) {
        if (p == 2) {
            return {};
        }
        if (q_set_cache.find(p) != q_set_cache.end()) {
            return q_set_cache[p];
        }

        vector<uint16_t> result = kappa_set(f(p));
        q_set_cache[p] = result;
        return result;
    }

    map<uint16_t, uint8_t> get_excess_cache() {
        return excess_cache;
    }
    
    uint8_t excess(uint16_t p) {
        if (p == 2) {
            return 0U;
        }
        if (excess_cache.find(p) != excess_cache.end()) {
            return excess_cache[p];
        }

        vector<uint16_t> q_set1 = q_set(p); // can't be empty because now p != 2
        cout << "[p = " << p << "] Computing excess (Q-Set = {" << q_set1[0];
        for (size_t i = 1; i < q_set1.size(); i++) {
            cout << ", " << q_set1[i];
        }
        cout << "})." << endl;

        vector<uint16_t> components = {};
        for (auto q : q_set1) {
            for (auto n : primitive_components(q)) {
                components.push_back(n);
            }
        }
        sort(components.begin(), components.end(), [](uint16_t a, uint16_t b)
                                    {
                                        return prime_pow(a) < prime_pow(b);
                                    });
        components.erase(unique(components.begin(), components.end()), components.end());

        uint8_t excess1 = 0;
        uint16_t fp = f(p);
        if (fp != 2 && (fp & 1) == 0 && is_3_pow(fp >> 1)) {
            excess1 = 4;
        } else if (q_set1.size() == 1 && (q_set1[0] & 1) != 0) {
            excess1 = 1;
        }

        bool done = false;
        term_array one(1);
        one.terms[0] = 0;
        while (!done) {
            uint64_t finite_summand1 = finite_summand(p, excess1);
            vector<uint16_t> finite_components1(finite_components(p, excess1));
            components.insert(components.end(), finite_components1.begin(), finite_components1.end());

            auto algebra = impartial_term_algebra(components);
            vector<uint32_t> alpha1{}; // actually alpha_finite_terms, but reused later for saving space
            if (finite_summand1 != 0) {
                for (uint8_t i = 0; i <= msb(finite_summand1); i++) {
                    if ((finite_summand1 & (((uint64_t)1) << i)) != 0) {
                        alpha1.push_back(i);
                    }
                }
            }
            vector<uint32_t> alpha_large_terms{};
            for (uint16_t q : q_set1) { // already sorted
                if ((q & 1) != 0) {
                    alpha_large_terms.push_back(algebra.get_basis()[find(algebra.get_q_components().begin(),
                        algebra.get_q_components().end(), q) - algebra.get_q_components().begin()]);
                }
            }
            alpha1.insert(alpha1.end(), alpha_large_terms.begin(), alpha_large_terms.end());
            sort(alpha1.begin(), alpha1.end()); // make this sorting more efficient

            cout << "[p = " << p << "] Trying excess = " << (uint16_t)excess1 << ", alpha = [";
            if (!alpha1.empty()) {
                cout << alpha1[0];
                for (size_t i = 1; i < alpha1.size(); i++) {
                    cout << ", " << alpha1[i];
                }
            }
            cout << "] (" << algebra.get_term_count() << " terms)" << endl;

            if (div_2_pow_min_1(p, algebra.get_term_count())) {
                cpp_int testpow(((cpp_int(1) << algebra.get_term_count()) - 1) / p);

                term_array alpha_terms((uint32_t)alpha1.size());
                for (uint32_t i = 0; i < alpha1.size(); i++) {
                    alpha_terms.terms[i] = alpha1[i];
                }
                term_array respow = algebra.power(alpha_terms, testpow);
                if (respow != one) done = true;
            } else {
                cout << "[p = " << p << "] div_2_pow_min_1 failed." << endl;
                done = true;
            }

            if (!done) excess1++;
        }

        excess_cache[p] = excess1;
        return excess1;
    }
};