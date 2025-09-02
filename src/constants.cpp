#include "constants.hpp"

#include <cstdint>
#include <mutex>
#include <string>
#include <fstream>
#include <vector>
#include <map>

using std::vector;
using std::map;

namespace test_values {
    const map<uint16_t, vector<uint16_t>> q_set_cache({{2, {}}});
    const map<uint16_t, uint8_t> excess_cache({{2, 0}});
};

namespace previously_known_values { // known at 1 january 2025
    const map<uint16_t, vector<uint16_t>> q_set_cache(
        {{2,{}}, {3,{2}}, {5,{4}}, {7,{3}}, {11,{5}}, {13,{4, 3}}, {17,{8}}, {19,{9}},
        {23,{11}}, {29,{4, 7}}, {31,{5}}, {37,{4, 9}}, {41,{5}}, {43,{7}}, {47,{23}},
        {53,{13}}, {59,{29}}, {61,{3, 5}}, {67,{3, 11}}, {71,{5, 7}}, {73,{9}},
        {79,{13}}, {83,{41}}, {89,{11}}, {97,{16, 3}}, {101,{25}}, {103,{3, 17}},
        {107,{53}}, {109,{4, 9}}, {113,{4, 7}}, {127,{7}}, {131,{5, 13}}, {137,{17}},
        {139,{3, 23}}, {149,{37}}, {151,{3, 5}}, {157,{13}}, {163,{81}}, {167,{83}},
        {173,{4, 43}}, {179,{89}}, {181,{9, 5}}, {191,{5, 19}}, {193,{32, 3}},
        {197,{4, 49}}, {199,{9, 11}}, {211,{5, 7}}, {223,{37}}, {227,{113}},
        {229,{19}}, {233,{29}}, {239,{7, 17}}, {241,{8, 3}}, {251,{25}}, {257,{16}}}
    );
    const map<uint16_t, uint8_t> excess_cache(
        {{2,0}, {3,0}, {5,0}, {7,1}, {11,1}, {13,0}, {17,0}, {19,4}, {23,1}, {29,0}, {31,1},
        {37,0}, {41,1}, {43,1}, {47,1}, {53,1}, {59,1}, {61,0}, {67,0}, {71,0}, {73,1},
        {79,1}, {83,1}, {89,1}, {97,0}, {101,1}, {103,0}, {107,1}, {109,0}, {113,0},
        {127,1}, {131,0}, {137,1}, {139,0}, {149,1}, {151,0}, {157,1}, {163,4}, {167,1},
        {173,0}, {179,1}, {181,0}, {191,0}, {193,0}, {197,0}, {199,0}, {211,0}, {223,1},
        {227,1}, {229,1}, {233,1}, {239,0}, {241,0}, {251,1}, {257,0}}
    );
};

namespace record_values {
    namespace {
        map<uint16_t, vector<uint16_t>> q_set_records() {
            std::ifstream file;
            file.open("q_set_records.txt");
            map<uint16_t, vector<uint16_t>> result{};

            std::string s, a, b;
            vector<uint16_t> tmp;
            std::size_t i;
            while (file >> s) {
                i = s.find(",");
                a = s.substr(1, i - 1);
                b = s.substr(i+2, s.find("}") - i - 2);
                tmp = {};
                if (b.length() != 0) {
                    i = b.find(",");
                    tmp.push_back((uint16_t)stoi(b.substr(0, i)));
                    while (i < b.length()) {
                        tmp.push_back((uint16_t)stoi(b.substr(i+1, b.find(",", i+1))));
                        i = b.find(",", i+1);
                    }
                }
                result[(uint16_t)stoi(a)] = tmp;
            }

            return result;
        };

        map<uint16_t, uint8_t> excess_records() {
            std::ifstream file;
            file.open("excess_records.txt");
            map<uint16_t, uint8_t> result{};

            std::string s, a, b;
            std::size_t i;
            while (file >> s) {
                i = s.find(",");
                a = s.substr(1, i - 1);
                b = s.substr(i+1, s.find("}") - i - 1);
                result[(uint16_t)stoi(a)] = (uint8_t)stoi(b);
            }

            return result;
        };
    }

    map<uint16_t, vector<uint16_t>> q_set_cache(q_set_records());
    map<uint16_t, uint8_t> excess_cache(excess_records());
    std::mutex q_set_cache_mutex;
    std::mutex excess_cache_mutex;

    void cache_q_set(uint16_t p, vector<uint16_t> q_set_p) {
        std::lock_guard<std::mutex> lock(q_set_cache_mutex);
        if (q_set_cache.find(p) == q_set_cache.end()) {
            // new q_set found!
            std::ofstream file;
            file.open("q_set_records.txt", std::ios::app);
            file << ",\n{" << p << ",{";
            if (!q_set_p.empty()) {
                file << q_set_p[0];
                for (std::size_t i = 1; i < q_set_p.size(); i++) {
                    file << "," << q_set_p[i];
                }
            }
            file << "}}";
            file.close();
        }
    };

    void cache_excess(uint16_t p, uint8_t excess_p) {
        std::lock_guard<std::mutex> lock(excess_cache_mutex);
        if (excess_cache.find(p) == excess_cache.end()) {
            // new excess found!
            std::ofstream file;
            file.open("excess_records.txt", std::ios::app);
            file << ",\n{" << p << "," << (unsigned)excess_p << "}";
            file.close();
        }
    };
};