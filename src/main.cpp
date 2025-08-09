/*
 * This file is part of transfinite-nim-calculator.
 *
 * It is a C++ translation of part of the original program cgsuite,
 * written in Scala by Aaron Siegel, licensed under the GNU GPL v3.0.
 * To be specific, this program is based on "GeneralizedOrdinal.scala",
 * "ImpartialTermAlgebra.scala", "NimFieldConstants.scala" and
 * "NimFieldCalculator.scala".
 *
 * This file is licensed under the GNU General Public License v3.0.
 * See the LICENSE file for more information.
 */

#include "prime_generator.hpp"
#include "nt_funcs.hpp"
#include "important_funcs.hpp"
#include "impartial_term_algebra.hpp"
#include "fin_nim.hpp"
#include "ww.hpp"
#include "www.hpp"
#include "kappa_component.hpp"
#include "www_nim.hpp"

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;
using namespace prime_generator;
using namespace important_funcs;
using namespace www_nim;

void write_alpha(ofstream& file, uint16_t p,
    const vector<uint16_t>& q_set_p, uint8_t excess_p, const www& alpha_p, time_t t) {
    string s_p = to_string(p); while (s_p.length() < 5) s_p = " " + s_p;
    string s_q_set_p = "[";
    if (!q_set_p.empty()) {
        s_q_set_p += to_string(q_set_p[0]);
        for (size_t i = 1; i < q_set_p.size(); i++) {
            s_q_set_p += ", " + to_string(q_set_p[i]);
        }
    }
    s_q_set_p += "]"; while (s_q_set_p.length() < 15) s_q_set_p = " " + s_q_set_p;
    string s_excess_p = to_string(excess_p); while (s_excess_p.length() < 6) s_excess_p = " " + s_excess_p;
    string s_alpha_p = alpha_p.to_string(); while (s_alpha_p.length() < 20) s_alpha_p = " " + s_alpha_p;
    string s_t = to_string(t); while (s_t.length() < 6) s_t = " " + s_t;
    file << s_p << " " << s_q_set_p << " " << s_excess_p << " " << s_alpha_p << " " << s_t << endl;
}

//TODO optimize
//TODO check for memory leakage
int main() {
    
    ofstream file;
    file.open("alpha_log.txt");
    file << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    file << "    p           q_set excess                alpha t(sec)" << endl;
    time_t checkpoint_time;
    uint16_t p;
    time_t t;
    for (int n = 1; n <= 15; n++) {
        p = nth_prime(n);
        checkpoint_time = time(nullptr);
        cout << "===== Calculating alpha(" << p << "). =====" << endl;
        cout << alpha(p) << endl;
        t = time(nullptr) - checkpoint_time;
        cout << "===== Time is " << t << " seconds. =====" << endl;
        cout << endl;
        write_alpha(file, p, q_set(p), excess(p), alpha(p), t);
    }
    file << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl << endl << endl;
    file.close();
    
    // test 1: 4282 seconds for alpha(47) :(
    // test 2: 3400 seconds for alpha(47) :/       (use more pointers)
    // test 3: 3350 seconds for alpha(47) :/       (use uint64_t* instead of vector<bool>)
    // test 4: 3300 seconds for alpha(47) :/       (use index instead of moving i and store msb(n))
    // test 5: 3185 seconds for alpha(47) :/       (use more pointers)
    // test 6:  601 seconds for alpha(47) :|       (use more pointers and nullptr instead of initial {0, 0} in table)
    // test 7:  493 seconds for alpha(47) :|       (use custom struct instead of std::vector so there's less overhead)
    
    return 0;
}