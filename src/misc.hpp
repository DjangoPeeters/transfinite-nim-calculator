#ifndef MISC_HPP
#define MISC_HPP

#include <cstdlib>
#include <cstdint>
#include <string>

uint16_t strtou16(const char* str);
uint32_t strtou32(const char* str);
size_t strtosize(const char* str);

// Creates `dir` (and any missing parent directories, like `mkdir -p`) if it doesn't already
// exist. Failures are silently ignored — if `dir` genuinely can't be created (e.g. a
// permissions issue), the file opens that follow will fail with a clear error of their own.
void ensure_dir_exists(const std::string& dir);

#endif