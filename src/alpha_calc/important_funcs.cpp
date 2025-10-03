#include "important_funcs.hpp"
#include "impartial_term_algebra.hpp"
#include "constants.hpp"
#include "ring_buffer_queue.hpp"
#include "calculation_logger.hpp"
#include "../number_theory/nt_funcs.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <set>
#include <map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

using std::vector;
using std::set;
using std::map;
using std::sort;
using std::unique;
using std::cout;
using uint256_t = boost::multiprecision::uint256_t;
using boost::multiprecision::msb;
using namespace nt_funcs;

constexpr bool TEST_MODE = true;
uint32_t MAX_TERM_COUNT = 10000000;

//TODO check when calculations failed and report so appropriately

namespace important_funcs {
    namespace {
        map<uint16_t, vector<uint16_t>> q_set_cache(TEST_MODE ? test_values::q_set_cache : record_values::q_set_cache);
        // every excess for primes p <= 257 is at most 4
        map<uint16_t, uint8_t> excess_cache(TEST_MODE ? test_values::excess_cache : record_values::excess_cache);
        std::mutex q_set_cache_mutex;
        std::mutex excess_cache_mutex;

        void cache_q_set(uint16_t p, vector<uint16_t> q_set_p) {
            std::lock_guard<std::mutex> lock(q_set_cache_mutex);
            if (q_set_cache.find(p) == q_set_cache.end()) {
                q_set_cache[p] = q_set_p;
                record_values::cache_q_set(p, q_set_p);
            }
        };

        void cache_excess(uint16_t p, uint8_t excess_p) {
            std::lock_guard<std::mutex> lock(excess_cache_mutex);
            if (excess_cache.find(p) == excess_cache.end()) {
                // new excess found!
                excess_cache[p] = excess_p;
                record_values::cache_excess(p, excess_p);
            }
        };

        uint256_t finite_summand(uint16_t p, uint16_t excess) {
            const vector<uint16_t> q_set1 = q_set(p);
            uint256_t base_finite_summand = 0;
            if (!q_set1.empty() && (q_set1[0] & 1) == 0) {
                base_finite_summand = ((uint256_t)1) << (q_set1[0] >> 1);
            }
            return base_finite_summand + (uint256_t)excess;
        }

        vector<uint16_t> finite_components(uint16_t p, uint16_t excess) {
            uint256_t finite_summand1 = finite_summand(p, excess);
            if (finite_summand1 <= 1) {
                return {};
            } else {
                vector<uint16_t> result;
                for (uint8_t k = 0; k <= msb(msb(finite_summand1)); k++) result.push_back(((uint16_t)1) << (k + 1)); // k <= 7 because we chose `uint256_t`
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

            for (const uint16_t q1 : q_set(p)) {
                for (const uint16_t q2 : primitive_components(q1)) {
                    components.insert(q2);
                }
            }
            excess_return ex = excess(p);
            if (ex.failed) {
                cout << "primitive_components failed\n";
                return {};
            }
            uint16_t exr = ex.result;
            const vector<uint16_t> fin_comps = finite_components(p, exr);
            components.insert(fin_comps.begin(), fin_comps.end());
            return components;
        }

        vector<uint16_t> kappa_set(uint16_t h) {
            uint16_t q = max_pow_min_prime_div(h).second;
            if (h == q) return {h};

            uint16_t g = h/q;
            vector<uint16_t> kappag_set(kappa_set(g));
            if (kappag_set.empty()) return {};
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

            cout << "Computing the degree of kappa(" << g << ").\n";

            // Shared resources
            ring_buffer_calculation_queue log_queue;
            std::atomic<bool> calculation_done{false};

            cout << "Constructing algebra (components = {" << components[0];
            for (size_t i = 1; i < components.size(); i++) {
                cout << ", " << components[i];
            }
            cout << "}).\n";

            // Check if term_count won't be too big
            uint32_t term_count_check = term_count_calc(components);
            if (term_count_check > MAX_TERM_COUNT) { // roughly more than 100 days of computing time needed at the time of writing
                cout << "constructing algebra failed due to imposed size limit\n";
                cout << "term_count would have been " << term_count_check << "\n";
                
                return {};
            }

            // Create objects
            impartial_term_algebra algebra(log_queue, calculation_done, components);
            calculation_logger logger(log_queue, calculation_done, logs_dir + "/calculation.log");

            cout << "Field has exponent " << algebra.get_term_count() << "." << '\n';
            term_array kappag_in_algebra((uint32_t)kappag_set.size());
            uint32_t i = 0;
            for (const auto r : kappag_set) {
                kappag_in_algebra.terms[i] = algebra.get_basis()[find(algebra.get_q_components().begin(),
                    algebra.get_q_components().end(), r) - algebra.get_q_components().begin()];
                i++;
            }

            uint32_t degree = 0;
            if (algebra.get_term_count() < ((uint32_t)1 << 14)) {
                degree = algebra.degree(kappag_in_algebra);
            } else {
                cout << "Starting multithreaded calculation..." << '\n';

                // Start threads
                std::thread calc_thread(&impartial_term_algebra::q_set_degree, &algebra, kappag_in_algebra, std::ref(degree));
                std::thread log_thread(&calculation_logger::progress_calculation_logger, &logger);

                // Wait for completion
                calc_thread.join();
                log_thread.join();

                cout << "All threads completed. Check calculation.log for full log." << '\n';
            }
            cout << "Degree is " << degree << "." << '\n';

            if (degree % q == 0) {
                return kappag_set;
            } else {
                kappag_set.insert(kappag_set.begin(), q); // prime powers are stored from lowest prime to highest prime
                return kappag_set;
            }
        }
    }

    const std::map<uint16_t, std::vector<uint16_t>>& get_q_set_cache() {
        std::lock_guard<std::mutex> lock(q_set_cache_mutex);
        return q_set_cache;
    }

    vector<uint16_t> q_set(uint16_t p) {
        if (p == 2) {
            return {};
        }
        std::lock_guard<std::mutex> lock(q_set_cache_mutex);
        if (q_set_cache.find(p) != q_set_cache.end()) {
            return q_set_cache[p];
        }
        lock.~lock_guard();

        const vector<uint16_t> result = kappa_set(f(p));
        if (!result.empty()) cache_q_set(p, result);
        return result;
    }

    const std::map<uint16_t, uint8_t>& get_excess_cache() {
        std::lock_guard<std::mutex> lock(excess_cache_mutex);
        return excess_cache;
    }
    
    excess_return excess(uint16_t p) {
        if (p == 2) {
            excess_return r;
            r.failed = false;
            r.result = 0U;
            r.used_cache = false;
            return r;
        }
        std::lock_guard<std::mutex> lock(excess_cache_mutex);
        if (excess_cache.find(p) != excess_cache.end()) {
            excess_return r;
            r.failed = false;
            r.result = excess_cache[p];
            r.used_cache = true;
            return r;
        }
        lock.~lock_guard();

        const vector<uint16_t> q_set1 = q_set(p);
        if (q_set1.empty()) {
            excess_return r;
            r.failed = true;
            r.term_count = 0;
            r.used_cache = false;
            return r;
        }
        cout << "[p = " << p << "] Computing excess (Q-Set = {" << q_set1[0];
        for (size_t i = 1; i < q_set1.size(); i++) {
            cout << ", " << q_set1[i];
        }
        cout << "})." << '\n';

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
        vector<uint16_t> q_components{};
        while (!done) {
            uint256_t finite_summand1 = finite_summand(p, excess1);
            const vector<uint16_t> finite_components1(finite_components(p, excess1));
            q_components = components;
            q_components.insert(q_components.end(), finite_components1.begin(), finite_components1.end());

            // Shared resources
            ring_buffer_calculation_queue log_queue;
            std::atomic<bool> calculation_done{false};

            cout << "[p = " << p << "] Constructing algebra (components = {" << q_components[0];
            for (size_t i = 1; i < q_components.size(); i++) {
                cout << ", " << q_components[i];
            }
            cout << "})." << '\n';

            // Check if term_count won't be too big
            uint32_t term_count_check = term_count_calc(q_components);
            if (term_count_check > MAX_TERM_COUNT) { // roughly more than 100 days of computing time needed at the time of writing
                cout << "constructing algebra failed due to imposed size limit\n";
                cout << "term_count would have been " << term_count_check << "\n";
                
                excess_return r;
                r.failed = true;
                r.term_count = term_count_check;
                r.used_cache = false;
                return r;
            }

            // Create objects
            impartial_term_algebra algebra(log_queue, calculation_done, q_components); //TODO make constructor faster?
            calculation_logger logger(log_queue, calculation_done, logs_dir + "/calculation.log");
            
            vector<uint32_t> alpha1{}; // actually alpha_finite_terms, but reused later for saving space
            if (finite_summand1 != 0) {
                for (uint32_t i = 0; i <= msb(finite_summand1); i++) {
                    if ((finite_summand1 & (((uint256_t)1) << i)) != 0) {
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
            cout << "] (" << algebra.get_term_count() << " terms)" << '\n';

            if (div_2_pow_min_1(p, algebra.get_term_count())) {
                const cpp_int testpow(((cpp_int(1) << algebra.get_term_count()) - 1) / p);
                // we need to exploit more properties of `testpow`

                term_array alpha_terms((uint32_t)alpha1.size());
                for (uint32_t i = 0; i < alpha1.size(); i++) {
                    alpha_terms.terms[i] = alpha1[i];
                }

                term_array respow = term_array();
                if (testpow < 1 << 7) {
                    respow = algebra.power(alpha_terms, testpow);
                } else {
                    cout << "Starting multithreaded calculation..." << '\n';

                    // Start threads
                    std::thread calc_thread(&impartial_term_algebra::excess_power, &algebra, alpha_terms, testpow, std::ref(respow));
                    std::thread log_thread(&calculation_logger::progress_calculation_logger, &logger);

                    // Wait for completion
                    calc_thread.join();
                    log_thread.join();

                    cout << "All threads completed. Check calculation.log for full log." << '\n';
                }

                if (respow != one) done = true;
            } else {
                cout << "[p = " << p << "] div_2_pow_min_1 failed." << '\n';
                done = true;
            }

            if (!done) excess1++;
        }

        cache_excess(p, excess1);
        excess_return r;
        r.failed = false;
        r.result = excess1;
        r.used_cache = false;
        return r;
    }
};