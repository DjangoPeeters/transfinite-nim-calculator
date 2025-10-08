#include "misc.hpp"

#include <cstdlib>
#include <cstdint>

uint16_t strtou16(const char* str) {
    size_t i = 0;
    uint16_t r = 0;
    while (str[i] != (char)0) {
        r = 10*r + (str[i] - '0');
        i++;
    }
    return r;
}

uint32_t strtou32(const char* str) {
    size_t i = 0;
    uint32_t r = 0;
    while (str[i] != (char)0) {
        r = 10*r + (str[i] - '0');
        i++;
    }
    return r;
}

size_t strtosize(const char* str) {
    size_t i = 0, r = 0;
    while (str[i] != (char)0) {
        r = 10*r + (str[i] - '0');
        i++;
    }
    return r;
}
