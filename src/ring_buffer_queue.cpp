// Mostly written by Claude AI
#include "ring_buffer_queue.hpp"

template<typename T, size_t Size>
bool ring_buffer_queue<T, Size>::push(const T& item) {
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) & mask;
    
    if (next_tail == head_.load(std::memory_order_acquire)) {
        return false; // Queue is full
    }
    
    buffer_[current_tail] = item;
    tail_.store(next_tail, std::memory_order_release);
    return true;
}

template<typename T, size_t Size>
bool ring_buffer_queue<T, Size>::push(T&& item) {
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) & mask;
    
    if (next_tail == head_.load(std::memory_order_acquire)) {
        return false; // Queue is full
    }
    
    buffer_[current_tail] = std::move(item);
    tail_.store(next_tail, std::memory_order_release);
    return true;
}

template<typename T, size_t Size>
bool ring_buffer_queue<T, Size>::pop(T& result) {
    size_t current_head = head_.load(std::memory_order_relaxed);
    
    if (current_head == tail_.load(std::memory_order_acquire)) {
        return false; // Queue is empty
    }
    
    result = std::move(buffer_[current_head]);
    head_.store((current_head + 1) & mask, std::memory_order_release);
    return true;
}

template<typename T, size_t Size>
bool ring_buffer_queue<T, Size>::empty() const {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
}

template<typename T, size_t Size>
bool ring_buffer_queue<T, Size>::full() const {
    size_t current_tail = tail_.load(std::memory_order_acquire);
    return ((current_tail + 1) & mask) == head_.load(std::memory_order_acquire);
}

template<typename T, size_t Size>
size_t ring_buffer_queue<T, Size>::size() const {
    size_t current_head = head_.load(std::memory_order_acquire);
    size_t current_tail = tail_.load(std::memory_order_acquire);
    return (current_tail - current_head) & mask;
}

// Explicit instantiation
template class ring_buffer_queue<data_quad, DEFAULT_RING_SIZE>;