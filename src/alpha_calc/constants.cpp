#include "constants.hpp"
#include "calculation_logger.hpp"
#include "../misc.hpp"

#include <cstdint>
#include <mutex>
#include <string>
#include <fstream>
#include <vector>
#include <map>

using std::vector;
using std::map;

namespace record_values {
    std::mutex q_set_cache_mutex;
    std::mutex excess_cache_mutex;
    std::mutex degree_kappa_cache_mutex;

    namespace {
        map<uint16_t, vector<uint16_t>> q_set_records() {
            std::lock_guard<std::mutex> lock(q_set_cache_mutex);
            std::ifstream file;
            file.open(logs_dir + "/q_set_records.txt");
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
            std::lock_guard<std::mutex> lock(excess_cache_mutex);
            std::ifstream file;
            file.open(logs_dir + "/excess_records.txt");
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

        map<uint16_t, uint32_t> degree_kappa_records() {
            std::lock_guard<std::mutex> lock(degree_kappa_cache_mutex);
            std::ifstream file;
            file.open(logs_dir + "/degree_kappa_records.txt");
            map<uint16_t, uint32_t> result{};

            std::string s, a, b;
            std::size_t i;
            while (file >> s) {
                i = s.find(",");
                a = s.substr(1, i - 1);
                b = s.substr(i+1, s.find("}") - i - 1);
                result[strtou16(a.c_str())] = strtou32(b.c_str());
            }

            return result;
        };
    }

    map<uint16_t, vector<uint16_t>> q_set_cache{};
    map<uint16_t, uint8_t> excess_cache{};
    map<uint16_t, uint32_t> degree_kappa_cache{};

    void init() {
        q_set_cache = q_set_records();
        excess_cache = excess_records();
        degree_kappa_cache = degree_kappa_records();
    }

    void cache_q_set(uint16_t p, vector<uint16_t> q_set_p) {
        std::lock_guard<std::mutex> lock(q_set_cache_mutex);
        if (q_set_cache.find(p) == q_set_cache.end()) {
            // new q_set found!
            std::ofstream file;
            file.open(logs_dir + "/q_set_records.txt", std::ios::app);
            file << (file.tellp() == std::streampos(0) ? "{" : ",\n{") << p << ",{";
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
            file.open(logs_dir + "/excess_records.txt", std::ios::app);
            file << (file.tellp() == std::streampos(0) ? "{" : ",\n{") << p << "," << (unsigned)excess_p << "}";
            file.close();
        }
    };

    void cache_degree_kappa(uint16_t h, uint32_t degree_kappa_h) {
        std::lock_guard<std::mutex> lock(degree_kappa_cache_mutex);
        if (degree_kappa_cache.find(h) == degree_kappa_cache.end()) {
            // new degree found!
            degree_kappa_cache[h] = degree_kappa_h;
            std::ofstream file;
            file.open(logs_dir + "/degree_kappa_records.txt", std::ios::app);
            file << (file.tellp() == std::streampos(0) ? "{" : ",\n{") << h << "," << degree_kappa_h << "}";
            file.close();
        }
    };
};