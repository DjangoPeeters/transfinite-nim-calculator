// Mostly written by Claude AI
#ifndef RING_BUFFER_QUEUE_HPP
#define RING_BUFFER_QUEUE_HPP

#include <limits>
#include <cstdint>
#include <atomic>
#include <array>
#include <string>

constexpr size_t DEFAULT_RING_SIZE = 1 << 13;
constexpr unsigned UNSIGNED_MAX = std::numeric_limits<unsigned>::max();

template<typename T, size_t Size>
class ring_buffer_queue {
private:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
    static constexpr size_t mask = Size - 1;
    
    std::array<T, Size> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};

public:
    bool push(const T& item);
    bool push(T&& item);
    bool pop(T& result);
    bool empty() const;
    bool full() const;
    size_t size() const;
};

struct data_quad {
    unsigned index;
    unsigned msbnp1;
    uint32_t cur_size;
    uint32_t res_size;
};

// maybe make a union type to use for logging both excess(p) and q_set(p) calculations
// or maybe make two instantiations, one for excess(p) and another for q_set(p)

// Common instantiation
using ring_buffer_calculation_queue = ring_buffer_queue<data_quad, DEFAULT_RING_SIZE>;

#endif