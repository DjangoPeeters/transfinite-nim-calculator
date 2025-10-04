#include "www_nim.hpp"
#include "fin_nim.hpp"
#include "ww.hpp"
#include "www.hpp"
#include "kappa_component.hpp"
#include "../number_theory/prime_generator.hpp"
#include "../number_theory/nt_funcs.hpp"
#include "../alpha_calc/important_funcs.hpp"
#include "../alpha_calc/calculation_logger.hpp"

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <ostream>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <mutex>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

using std::cout;
using std::size_t;
using std::vector;
using std::list;
using std::sort;
using std::to_string;
using boost::multiprecision::cpp_int;
using boost::multiprecision::msb;
using boost::multiprecision::bit_test;
using namespace prime_generator;
using namespace fin_nim;

namespace www_nim {
    namespace {
        std::mutex alpha_cache_mutex;

        void cache_alpha(uint16_t p, alpha_return alpha_p, time_t t) {
            size_t index = prime_pi(p)-2; // 0-based indexing starting from prime 3
            string s_p = to_string(p);
            string line = "";
            string s_alpha_p = "";
            string s_t = "";
            size_t line_length = 76;
            // construct line we want to print
            if (alpha_p.failed) {
                line = "Skipping " + s_p + " (exponent " + to_string(alpha_p.term_count) + ")";
            } else {
                while (s_p.length() < 5) s_p = " " + s_p;
                const vector<uint16_t>& q_set_p = important_funcs::get_q_set_cache().at(p);
                string s_q_set_p = "[";
                if (!q_set_p.empty()) {
                    s_q_set_p += to_string(q_set_p[0]);
                    for (size_t i = 1; i < q_set_p.size(); i++) {
                        s_q_set_p += ", " + to_string(q_set_p[i]);
                    }
                }
                s_q_set_p += "]"; while (s_q_set_p.length() < 15) s_q_set_p = " " + s_q_set_p;
                uint8_t excess_p = important_funcs::get_excess_cache().at(p);
                string s_excess_p = to_string(excess_p); while (s_excess_p.length() < 6) s_excess_p = " " + s_excess_p;
                s_alpha_p = alpha_p.result.to_string(); while (s_alpha_p.length() < 40) s_alpha_p = " " + s_alpha_p;
                if (s_alpha_p.length() > 40) s_alpha_p = s_alpha_p.substr(0, 40);
                s_t = to_string(t); while (s_t.length() < 6) s_t = " " + s_t;
                line = s_p + " " + s_q_set_p + " " + s_excess_p + " " + s_alpha_p + " " + s_t;
            }
            if (line.length() > line_length) {
                line = line.substr(0, line_length);
            } else {
                line.append(line_length - line.length(), ' ');
            }
            line += '\n';

            // editing the file
            std::lock_guard<std::mutex> lock(alpha_cache_mutex);
            std::fstream file;
            file.open(logs_dir + "/alpha_records.txt", std::ios::in | std::ios::out);
            if (!file.is_open()) {
                cout << "failed to open alpha_records.txt\n";
                exit(1);
            }

            // Determine current number of lines
            file.seekg(0, std::ios::end);
            std::streampos fileSize = file.tellg();
            size_t currentLineIndex = fileSize / (line_length + 1);

            // Fill gaps if needed
            if (currentLineIndex < index) {
                cout << "filling gaps...\n";
                file.seekp(currentLineIndex * (line_length + 1));
                for (size_t i = currentLineIndex; i < index; i++) {
                    uint16_t temp_p = nth_prime(i + 2); // corresponding odd prime with 0-based index i
                    string temp_line = "Skipping " + to_string(temp_p);
                    if (temp_line.length() > line_length) {
                        temp_line = temp_line.substr(0, line_length);
                    } else {
                        temp_line.append(line_length - temp_line.length(), ' ');
                    }
                    file << temp_line << '\n';
                }
                file.flush();
            }

            // Now read/write at the target index
            std::streampos targetPos = index * (line_length + 1);
            file.seekg(targetPos);

            string currentLine;
            bool lineExists = static_cast<bool>(std::getline(file, currentLine));
            file.clear(); // stupid error flags not saying anything

            if (!lineExists || currentLine[0] == 'S') {
                // Line doesn't exist or was skipped - write new data
                file.seekp(targetPos);
                file << line;
                file.flush();
            } else {
                if (!alpha_p.failed) { // If the calculation failed and we already have a record for when it didn't fail, we shouldn't alter this line
                    // Compare with existing data
                    string s_old_alpha_p = currentLine.substr(29, 40);
                    if (s_alpha_p != s_old_alpha_p) {
                        cout << "calculated alphas were inconsistent\n";
                        exit(1);
                    }
                
                    // Check if new calculation was faster
                    string s_old_t = currentLine.substr(line_length - 6, 6);
                    uint32_t old_t = 0;
                    for (int i = 0; i < 6; i++) {
                        old_t = 10*old_t;
                        if ('0' <= s_old_t[i] && s_old_t[i] <= '9') old_t += (s_old_t[i] - '0');
                    }
                    uint32_t new_t = (uint32_t)t;

                    if (new_t < old_t) {
                        file.seekp(targetPos);
                        file << line;
                        file.flush();
                    }
                }
            }

            file.close();
        }

        vector<ww> www_to_2_pow(const www& x) {
            vector<ww> result{};
            for (const auto& ab : x.get_terms()) {
                ww c = ww() * ab.first;
                for (const uint8_t k : fin_to_2_pow(ab.second)) {
                    result.push_back(c + k);
                }
            }
            return result;
        }

        www www_of_2_pow(const vector<ww>& x) {
            www result(0);
            www temp(1);
            for (const ww& xi : x) {
                temp = 1;
                for (const auto& ab : xi.get_terms()) {
                    if (ab.first == 0) {
                        temp = temp * (((uint256_t)1) << ab.second);
                    } else {
                        temp = temp * www({{ww({{ab.first - 1, ab.second}}), 1}});
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

        www www_of_kappa_components(const vector<kappa_component>& components, uint256_t coefficient) {
            list<std::pair<uint16_t, uint16_t>> terms{};
            uint16_t temp1, temp2;
            for (const auto& component : components) {
                temp1 = nth_prime(component.get_k() + 2);
                temp2 = 1;
                for (uint16_t i = 0; i < component.get_n(); i++) temp2 *= temp1;
                terms.push_back({component.get_k(), temp2 * component.get_exponent()});
            }
            return www({{ww(terms), coefficient}});
        }

        vector<kappa_component> kappa_components_of_term(const std::pair<ww, uint256_t>& a) {
            vector<kappa_component> result{};
            for (const auto& expcoeff : a.first.get_terms()) {
                uint16_t k = expcoeff.first;
                uint16_t p = nth_prime(expcoeff.first + 2);
                uint16_t n = 1;
                uint16_t pn = p;
                while (pn <= expcoeff.second) {
                    n++;
                    pn *= p;
                }
                uint16_t temp = 1;
                for (uint16_t i = 0; i < n; i++) {
                    k = (expcoeff.second / temp) % p;
                    if (k != 0) result.push_back(kappa_component(expcoeff.first, i, k));
                    temp *= p;
                }
            }
            return result;
        }

        www reduce_components(const vector<kappa_component>& components,
            const vector<kappa_component>& processed_components, uint256_t coefficient) {
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
                        alpha_return ar = alpha(nth_prime(a.get_k() + 2));
                        if (ar.failed) {
                            cout << "reduce_components failed because alpha failed\n";
                            return 0;
                        }
                        www alpha1 = ar.result;

                        www result(0);
                        for (const auto& expcoef : alpha1.get_terms()) {
                            auto exp = expcoef.first;
                            auto coef = expcoef.second;
                            if (exp == 0) {
                                auto it = components.begin();
                                it++; it++;
                                new_components.insert(new_components.end(), it, components.end());

                                result = www_nim_add(result, reduce_components(new_components, processed_components,
                                    fin_nim_mul(coefficient, coef)));
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

        www www_nim_mul_term(const std::pair<ww, uint256_t>& a, const std::pair<ww, uint256_t>& b) {
            vector<kappa_component> components = kappa_components_of_term(a), other = kappa_components_of_term(b);
            components.insert(components.end(), other.begin(), other.end());
            sort(components.begin(), components.end());
            return reduce_components(components, {}, fin_nim_mul(a.second, b.second));
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
        for (const auto& ap : a.get_terms()) {
            for (const auto& bp : b.get_terms()) {
                result = www_nim_add(result, www_nim_mul_term(ap, bp));
            }
        }
        return result;
    }

    www www_nim_square(const www& a) {
        www result(0);
        for (const auto& ap : a.get_terms()) {
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
        auto pn = nt_funcs::prime_pow(q);
        uint16_t p = pn.first;
        uint16_t n = pn.second;
        if (p == 2) {
            // Fermat 2-power
            return www(((uint256_t)1)<<(((uint16_t)1)<<(n-1)));
        }
        uint16_t k = (uint16_t)prime_pi(p) - 1; // 0-indexed for historical reasons
        uint16_t pn1 = 1;
        for (uint16_t i = 1; i < n; i++) pn1 *= p;
        return www({{ww({{k-1, pn1}}), 1}});
    }

    alpha_return alpha(uint16_t p) {
        const time_t start = time(nullptr);
        const excess_return exr = important_funcs::excess(p);
        if (exr.failed) {
            cout << "alpha failed\n";
            alpha_return ar(true, 0, exr.term_count);
            if (p != 2) cache_alpha(p, ar, 0);
            return ar;
        }
        www result(exr.result);
        const auto q_set_r = important_funcs::q_set(p);
        if (q_set_r.first != 0) {
            cout << "alpha failed\n";
            alpha_return ar(true, 0, q_set_r.first);
            if (p != 2) cache_alpha(p, ar, 0);
            return ar;
        }
        for (const uint16_t q : q_set_r.second) {
            result = kappa(q) + result;
        }
        const time_t t = time(nullptr) - start;
        const alpha_return ar(false, result, 0);
        if (!exr.used_cache && p != 2) cache_alpha(p, ar, t);
        return ar;
    }
};