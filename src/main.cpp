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

using std::cout;
using std::endl;
using namespace prime_generator;
using namespace important_funcs;
using namespace www_nim;

//TODO debug
int main() {

    for (int n = 1; n <= 5; n++) {
        cout << alpha(nth_prime(n)) << endl;
    }

    /*
    std::set<int> s1{0,1,2};
    std::set<int> s2{3,4,5};
    s1.insert(s2.begin(), s2.end());
    for (const int i : s1) std::cout << i << " ";
    std::cout << std::endl;
    */

    /*
    auto [begin_it, end_it] = primes(2, 100);
    for (auto it = begin_it; it != end_it; it++) {
        std::cout << std::to_string(*it) << std::endl;
    }

    www x = 0;
    std::cout << x << std::endl;
    */

    /*
    std::cout << "now the big test" << std::endl;

    using boost::multiprecision::cpp_int;
    
    cpp_int big = 1;
    for (int i = 1; i <= 50; ++i) {
        big *= i;
    }
    
    std::cout << "50! = " << big << std::endl;

    std::cout << (2U - 1U) << std::endl;

    std::set<int> set{3, 1, 4, 1, 5, 9, 2, 6, 5};
    for (int x : set) {
        std::cout << x << ' ';
    };
    std::cout << '\n';
    */

    /*
    uint16_t v = 1;
    std::cout << v << std::endl;

    v = 10 % 3U;
    std::cout << v << std::endl;
    */

    /*
    std::cout << sizeof(char) << ", " << sizeof(short) << ", " << sizeof(int)
    << ", " << sizeof(long) << ", " << sizeof(long long) << std::endl;

    std::time_t checkpoint_time = time(nullptr);
    int toy = 0;
    while (time(nullptr) - checkpoint_time < 1) {
        for (int i = 0; i < 100; i++) {
            toy *= 123456;
            toy /= 654321;
            toy += 2021;
            toy *= 6;
        }
    }
    std::cout << "1 second has passed" << std::endl;
    std::cout << (((float)8230) / (12035 + 1)) << std::endl;
    */
    
    return 0;
}