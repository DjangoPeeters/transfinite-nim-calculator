#include "periodic_bools.hpp"

#include <vector>

using std::vector;
using std::size_t;

//TODO optimize
namespace periodic_bools { // entries beyond the end of a vector are treated as `false`
    namespace {
        std::size_t vb_div_prefix(const vector<bool>& bools, size_t num_end, size_t den_end) {
            size_t q = 0, i = 0;
            bool done = (den_end > num_end);
            while (!done) {
                i += den_end;
                q++;
                if (i + den_end > num_end) {
                    done = true;
                } else {
                    for (size_t j = 0; j < den_end; j++) {
                        if (bools.size() <= i + j) {
                            if (j < bools.size() && bools[j]) {
                                done = true;
                                break;
                            } 
                        } else if (bools[j] != bools[i + j]) {
                            done = true;
                            break;
                        }
                    }
                }
            }
            return q;
        }

        // `end` is the end of the part that we're checking
        bool vb_is_multiple(const vector<bool>& bools, size_t end) {
            size_t q = 0;
            for (size_t i = 1; i < end; i++) {
                if (end % i != 0) continue;
                q = vb_div_prefix(bools, end, i);
                if (q > 1) return true;
            }
            return false;
        }

        size_t pad_right(const vector<bool>& bools) {
            size_t falses = 0, tmp = 0, true_stop = 0;
            for (size_t i = 0; i < bools.size(); i++) {
                if (bools[i]) {
                    tmp = 0;
                    true_stop = i+1;
                } else {
                    falses++;
                    if (tmp > falses) falses = tmp;
                }
            }

            return true_stop + falses;
        }

        size_t strip_right(const vector<bool>& bools) {
            size_t s = bools.size();
            while (s > 0 && !bools[s-1]) s--;
            return s;
        }
    };

    std::pair<std::size_t, std::size_t> find_rep(const vector<bool>& bools) {
        if (bools.empty()) return {0, 0};
        
        size_t end = pad_right(bools), i = 1, q;
        auto resq = vb_div_prefix(bools, end, 1);

        for (size_t k = 1; k < end; k++) {
            if (!vb_is_multiple(bools, k)) {
                q = vb_div_prefix(bools, end, k);
                if (q > resq || (q > 1 && resq*i < q*k)) {
                    i = k;
                    resq = q;
                }
            }
        }

        return {resq, i};
    }
}