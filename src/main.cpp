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
#include "misc.hpp"
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
test 12:   3 seconds for alpha(47) :)))     (only square result and keep multiplier small)
*/

// most important file for the calculation: important_funcs.cpp (`TEST_MODE = true` initializes the caches empty except for `p = 2`)

/*
To calculate alpha(719), the next unknown alpha at the time of writing this, we'd need at least about 200 GB in memory.
I don't know how long this will take, but my best guess says it'll take at least 3 years and 3 months...
unless we find a way to go about doing this calculation in a smarter way.
*/

//TODO optimize
//TODO split calculating into more threads
void alphas() {
    time_t checkpoint_time;
    uint16_t p;
    time_t t;
    alpha_return ar = alpha(2);
    unsigned n = 2; // `alpha(nth_prime(1))` (a.k.a. `alpha(2)`) is a dummy value
    while (1) {
        p = nth_prime(n);
        checkpoint_time = time(nullptr);
        cout << "===== Calculating alpha(" << p << "). =====\n";
        ar = alpha(p);
        t = time(nullptr) - checkpoint_time;
        if (ar.failed) {
            cout << "calculating alpha(" << p << ") failed\n\n";
        } else {
            cout << ar.result << '\n';
            cout << "===== Time is " << t << " seconds. =====\n\n";
        }
        n++;
    }
}

int main(int argc, char* argv[]) {
    cout << "argc == " << argc << '\n';
    for (int ndx{}; ndx != argc; ++ndx) {
        cout << "argv[" << ndx << "] == " << argv[ndx] << '\n';
    }
    cout << "argv[" << argc << "] == " << static_cast<void*>(argv[argc]) << '\n';
    if (1 < argc) {
        if (argv[1] == string("alphas")) {
            if (2 < argc) {
                logs_dir = argv[2];
                cout << "logs will be kept in directory " << logs_dir << " (relative path)\n";
            }
            init();
            if (2 < argc) {
                if (3 < argc) MAX_TERM_COUNT = strtou32(argv[3]);
                cout << "setting MAX_TERM_COUNT to " << MAX_TERM_COUNT << "\n";
            }
            alphas();
        } else if(argv[1] == string("alpha")) {
            if (2 < argc) {
                logs_dir = argv[2];
                cout << "logs will be kept in directory " << logs_dir << " (relative path)\n";
            }
            init();
            if (2 < argc) {
                if (3 < argc) {
                    uint16_t p;
                    if (argv[3] == string("nth_prime") && 4 < argc) {
                        p = nth_prime(strtosize(argv[4]));
                    } else {
                        p = strtou16(argv[3]);
                    }
                    time_t checkpoint_time = time(nullptr);
                    cout << "===== Calculating alpha(" << p << "). =====\n";
                    alpha_return ar = alpha(p);
                    time_t t = time(nullptr) - checkpoint_time;
                    if (ar.failed) {
                        cout << "calculating alpha(" << p << ") failed\n\n";
                    } else {
                        cout << ar.result << '\n';
                        cout << "===== Time is " << t << " seconds. =====\n\n";
                    }
                }
            }
            if (argc <= 3) {
                unsigned n = 2;
                uint16_t p = nth_prime(n);
                auto excess_cache = get_excess_cache();
                auto q_set_cache = get_q_set_cache();
                while (excess_cache.find(p) != excess_cache.end()
                    && q_set_cache.find(p) != q_set_cache.end()) {
                    n++;
                    p = nth_prime(n);
                }
                time_t checkpoint_time = time(nullptr);
                cout << "===== Calculating alpha(" << p << "). =====\n";
                alpha_return ar = alpha(p);
                time_t t = time(nullptr) - checkpoint_time;
                if (ar.failed) {
                    cout << "calculating alpha(" << p << ") failed\n\n";
                } else {
                    cout << ar.result << '\n';
                    cout << "===== Time is " << t << " seconds. =====\n\n";
                }
            }
        }
    }

    return 0;
}
