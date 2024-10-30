#ifndef FIBONACCI_UTIL_H
#define FIBONACCI_UTIL_H

#include <stdint.h>
#include <stdbool.h>

void write_file(const char *path, const char *string, bool append);

uint64_t parseUINT64(char *str, uint64_t max, uint64_t min);

#endif //FIBONACCI_UTIL_H
