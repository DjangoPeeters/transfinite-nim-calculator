#include "misc.hpp"

#include <cstdlib>
#include <cstdint>
#include <string>
#include <sys/stat.h>

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

void ensure_dir_exists(const std::string& dir) {
    if (dir.empty()) return;
    size_t pos = 0;
    while (pos != std::string::npos) {
        pos = dir.find('/', pos + 1);
        mkdir(dir.substr(0, pos).c_str(), 0755); // ignore result: EEXIST is fine, other errors surface at file-open time
    }
}
