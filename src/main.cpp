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
#include <set>
#include <ctime>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>

using std::cout;
using std::endl;
using boost::multiprecision::msb;
using namespace prime_generator;
using namespace important_funcs;
using namespace www_nim;

//TODO optimize
int main() {
    
    std::time_t checkpoint_time;
    for (int n = 1; n <= 15; n++) {
        checkpoint_time = time(nullptr);
        cout << "===== Calculating alpha(" << nth_prime(n) << "). =====" << endl;
        cout << alpha(nth_prime(n)) << endl;
        cout << "===== Time is " << (time(nullptr) - checkpoint_time) << " seconds. =====" << endl;
        cout << endl;
    }
    
    // test 1: 4282 seconds for alpha(47) :(
    // test 2: 3400 seconds for alpha(47) :/       (use more pointers)
    // test 3: 3350 seconds for alpha(47) :/       (use uint64_t* instead of vector<bool>)
    // test 4: 3300 seconds for alpha(47) :/       (use index instead of moving i and store msb(n))
    // test 5: 3185 seconds for alpha(47) :/       (use more pointers)
    // test 6:  601 seconds for alpha(47) :|       (use more pointers and nullptr instead of initial {0, 0} in table)
    // passed Aaron Siegel's java program with 415 seconds (see http://www.neverendingbooks.org/aaron-siegel-on-transfinite-number-hacking/)
    // test 7:  215 seconds for alpha(47) :)       (use custom struct instead of std::vector so there's less overhead)
    
    return 0;
}