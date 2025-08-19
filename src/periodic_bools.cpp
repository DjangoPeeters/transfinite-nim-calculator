#include "periodic_bools.hpp"

#include <vector>

using std::vector;
using std::size_t;

namespace periodic_bools {
    namespace {
        // `end` is the end of the part we're dividing by
        std::pair<std::size_t, vector<bool>> vb_div_part(const vector<bool>& num, size_t end) {
            size_t q = 0, i = 0;

            bool done = end > num.size();
            while (!done) {
                i += end;
                q++;
                if (i + end > num.size()) {
                    done = true;
                } else {
                    for (size_t j = 0; j < end; j++) {
                        if (num[j] != num[i + j]) {
                            done = true;
                            break;
                        }
                    }
                }
            }

            vector<bool> r = vector<bool>(num.size() - i);
            for (size_t j = 0; j < r.size(); j++) {
                r[j] = num[i + j];
            }

            return {q, r};
        }

        bool vb_is_multiple(const vector<bool>& bools) {
            std::pair<size_t, vector<bool>> qr = {0, {}};
            for (size_t i = 1; i < bools.size(); i++) {
                qr = vb_div_part(bools, i);
                if (qr.second.empty() && qr.first > 1) return true;
            }
            
            return false;
        }
    };

    vector<bool> pad_right(const vector<bool>& bools) {
        size_t falses = 0, tmp = 0;
        for (size_t i = 0; i < bools.size(); i++) {
            if (bools[i]) {
                falses = 0;
            } else {
                falses++;
                if (falses > tmp) tmp = falses;
            }
        }

        vector<bool> result(bools);
        for (size_t i = 0; i < falses; i++) {
            result.push_back(false);
        }

        return result;
    }

    vector<bool> strip_right(const vector<bool>& bools) {
        size_t s = bools.size();

        while (s > 0 && !bools[s-1]) s--;

        vector<bool> result = vector<bool>(s);
        result.assign(bools.begin(), bools.begin() + s);
        return result;
    }

    std::pair<std::pair<size_t, vector<bool>>, vector<bool>> vb_euclid(const vector<bool>& bools) {
        if (bools.empty()) return {{0, {}}, {}};
        const auto j = bools.size();
        auto i = 1;

        auto resqr = vb_div_part(bools, 1);
        size_t q = 0;
        vector<bool> r{}, ss{};
        std::pair<size_t, vector<bool>> qr;
        
        for (size_t k = 1; k < j; k++) {
            ss.push_back(bools[k-1]);
            if (!vb_is_multiple(ss)) {
                qr = vb_div_part(bools, k);
                if (qr.first > resqr.first || (qr.first > 1 && qr.second.size() < resqr.second.size())) {
                    i = k;
                    resqr = qr;
                }
            }
        }
        vector<bool> ress = vector<bool>(i);
        for (size_t k = 0; k < i; k++) ress[k] = bools[k];

        return {{resqr.first, ress}, resqr.second};
    }
}