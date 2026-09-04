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
#include "www_nim_calc/expr_parser.hpp"
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

using std::cout;
using std::ios;
using std::ofstream;
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

void compute_and_report_alpha(uint16_t p) {
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

//TODO optimize
//TODO split calculating into more threads
// p_min lets a run be resumed/chunked (e.g. across separate jobs on a server with a wall-time cap)
void alphas_upto(uint16_t p_max, uint16_t p_min = 3) {
    alpha(2); // `alpha(nth_prime(1))` (a.k.a. `alpha(2)`) is a dummy value
    unsigned n = 2;
    uint16_t p = nth_prime(n);
    while (p < p_min) {
        n++;
        p = nth_prime(n);
    }
    while (p <= p_max) {
        compute_and_report_alpha(p);
        n++;
        p = nth_prime(n);
    }
}

void alphas(uint16_t p_min = 3) {
    alphas_upto(UINT16_MAX, p_min);
}

void excess_to_afile() {
    size_t n = 2;
    uint16_t p = 0;
    size_t found = 0;
    const auto cache = get_excess_cache();
    const size_t total = cache.size() - 1; // ignore excess(2)
    ofstream file;
    file.open(logs_dir + "/excess_afile.txt", ios::ate);
    while (found < total) {
        p = nth_prime(n);
        if (cache.find(p) != cache.end()) {
            file << (n-1) << ' ' << (unsigned)cache.at(p) << '\n';
            found++;
        } else {
            file << (n-1) << " ?\n";
        }
        n++;
    }
    file.close();
}

void excess_to_bfile() {
    size_t n = 2;
    uint16_t p = 3;
    const auto cache = get_excess_cache();
    ofstream file;
    file.open(logs_dir + "/excess_bfile.txt", ios::ate);
    while (cache.find(p) != cache.end()) {
        file << (n-1) << ' ' << (unsigned)cache.at(p) << '\n';
        n++;
        p = nth_prime(n);
    }
    file.close();
}

namespace {

void set_logs_dir_if_given(int argc, char* argv[], int index) {
    if (index < argc) {
        logs_dir = argv[index];
        cout << "logs will be kept in directory " << logs_dir << " (relative path)\n";
    }
}

// a prime given either directly (e.g. "127") or as "nth_prime N"
uint16_t parse_prime_arg(int argc, char* argv[], int index) {
    if (argv[index] == string("nth_prime") && index + 1 < argc) {
        return nth_prime(strtosize(argv[index + 1]));
    }
    return strtou16(argv[index]);
}

// the smallest prime for which either excess or q_set hasn't been computed yet
uint16_t next_unknown_prime() {
    unsigned n = 2;
    uint16_t p = 3;
    const auto excess_cache = get_excess_cache();
    const auto q_set_cache = get_q_set_cache();
    while (excess_cache.find(p) != excess_cache.end()
        && q_set_cache.find(p) != q_set_cache.end()) {
        n++;
        p = nth_prime(n);
    }
    return p;
}

void cmd_alphas(int argc, char* argv[]) {
    set_logs_dir_if_given(argc, argv, 2);
    init();
    if (2 < argc) {
        if (3 < argc) MAX_TERM_COUNT = strtou32(argv[3]);
        cout << "setting MAX_TERM_COUNT to " << MAX_TERM_COUNT << "\n";
    }
    if (4 < argc) {
        alphas(parse_prime_arg(argc, argv, 4));
    } else {
        alphas();
    }
}

void cmd_alpha(int argc, char* argv[]) {
    set_logs_dir_if_given(argc, argv, 2);
    init();
    if (3 < argc) {
        compute_and_report_alpha(parse_prime_arg(argc, argv, 3));
    } else {
        compute_and_report_alpha(next_unknown_prime());
    }
}

void cmd_afile(int argc, char* argv[]) {
    set_logs_dir_if_given(argc, argv, 2);
    init();
    excess_to_afile();
}

void cmd_bfile(int argc, char* argv[]) {
    set_logs_dir_if_given(argc, argv, 2);
    init();
    excess_to_bfile();
}

void cmd_calc(int argc, char* argv[]) {
    set_logs_dir_if_given(argc, argv, 2);
    init();
    if (argc <= 3) {
        cout << "usage: calc [logs_dir] EXPRESSION\n";
        cout << "  ordinal arithmetic: + * and w^E (Cantor normal form), e.g. \"w^3 + w*2 + 1\"\n";
        cout << "  nim (field) arithmetic: +. *. ^. , e.g. \"w +. w\", \"w *. w\", \"w ^. 5\"\n";
        return;
    }
    try {
        www result = expr_parser::parse_and_evaluate(argv[3]);
        cout << result.to_string() << '\n';
    } catch (const expr_parser::parse_error& e) {
        cout << "parse error: " << e.what() << '\n';
    }
}

} // namespace

int main(int argc, char* argv[]) {
    cout << "argc == " << argc << '\n';
    for (int ndx{}; ndx != argc; ++ndx) {
        cout << "argv[" << ndx << "] == " << argv[ndx] << '\n';
    }
    cout << "argv[" << argc << "] == " << static_cast<void*>(argv[argc]) << "\n\n";

    if (argc <= 1) {
        init();
        alphas_upto(150);
        return 0;
    }

    const string command = argv[1];
    if (command == "alphas") {
        cmd_alphas(argc, argv);
    } else if (command == "alpha") {
        cmd_alpha(argc, argv);
    } else if (command == "afile") {
        cmd_afile(argc, argv);
    } else if (command == "bfile") {
        cmd_bfile(argc, argv);
    } else if (command == "calc") {
        cmd_calc(argc, argv);
    }

    return 0;
}
