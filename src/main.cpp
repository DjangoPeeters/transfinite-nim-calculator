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

#include "alpha_calc/calculation_logger.hpp"
#include "alpha_calc/important_funcs.hpp"
#include "number_theory/prime_generator.hpp"
#include "www_nim_calc/ww.hpp"
#include "www_nim_calc/www.hpp"
#include "www_nim_calc/www_nim.hpp"

#include <cstdlib>
#include <cstdint>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>

using namespace std;
using namespace prime_generator;
using namespace important_funcs;
using namespace www_nim;

/*
record times:
test 1: 4282 seconds for alpha(47) :(
test 2: 3400 seconds for alpha(47) :/       (use more pointers)
test 3: 3350 seconds for alpha(47) :/       (use uint64_t* instead of vector<bool>)
test 4: 3300 seconds for alpha(47) :/       (use index instead of moving i and store msb(n))
test 5: 3185 seconds for alpha(47) :/       (use more pointers)
test 6:  601 seconds for alpha(47) :|       (use more pointers and nullptr instead of initial {0, 0} in table)
test 7:  493 seconds for alpha(47) :|       (use custom struct instead of std::vector so there's less overhead)
passed Aaron Siegel's java program with 415 seconds
test 8:  195 seconds for alpha(47) :)       (use custom struct more)
test 9:  110 seconds for alpha(47) :)       (use another custom struct only for transferring instead of creation)
test 10:  85 seconds for alpha(47) :))      (use separate threads for calculating and logging)
test 11:  35 seconds for alpha(47) :))      (tweak push interval from calculating to logging)
test 12:   3 seconds for alpha(47) :)))     (use sliding-window method where we only multiply a large term_array with a small constant term_array)
*/

// most important file for the calculation: important_funcs.cpp (`TEST_MODE = true` initializes the cache empty except for `p = 2`)

void prep_alpha(fstream& file) {
    file.open(logs_dir + "/alpha_log.txt", ios::app);
    file << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
    file << "    p           q_set excess                alpha t(sec)\n";
    file << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n";
    file.close();
}

void write_alpha(fstream& file, uint16_t p,
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
    file.open(logs_dir + "/alpha_log.txt", std::ios::in | std::ios::out | std::ios::ate);
    file.seekp(-59, std::ios::end);
    file << s_p << " " << s_q_set_p << " " << s_excess_p << " " << s_alpha_p << " " << s_t << '\n';
    file << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n";
    file.close();
}

//TODO optimize
//TODO split calculating into more threads
int main(int argc, char *argv[])
{
    cout << "argc == " << argc << '\n';
    for (int ndx{}; ndx != argc; ++ndx) {
        cout << "argv[" << ndx << "] == " << argv[ndx] << '\n';
    }
    cout << "argv[" << argc << "] == " << static_cast<void*>(argv[argc]) << '\n';
    if (1 < argc) logs_dir = argv[1];
 
    fstream file;
    prep_alpha(file);
    time_t checkpoint_time;
    uint16_t p;
    time_t t;
    unsigned n = 2; // `alpha(nth_prime(1))` (a.k.a. `alpha(2)`) is a dummy value
    while (1) {
        p = nth_prime(n);
        checkpoint_time = time(nullptr);
        cout << "===== Calculating alpha(" << p << "). =====\n";
        cout << alpha(p) << '\n';
        t = time(nullptr) - checkpoint_time;
        cout << "===== Time is " << t << " seconds. =====\n\n";
        write_alpha(file, p, q_set(p), excess(p), alpha(p), t);
        n++;
    }

    return 0;
}
