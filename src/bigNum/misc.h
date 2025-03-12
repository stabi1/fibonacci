#ifndef FIBONACCI_MISC_H
#define FIBONACCI_MISC_H

#include <stdlib.h>
#include <stdint.h>

void mallocCheck(void *p);

char *getCurrentDateTime();

size_t custom_lzcnt(uint64_t n);

size_t custom_tzcnt(uint64_t n);

size_t max(size_t a, size_t b);

size_t min(size_t a, size_t b);

#endif //FIBONACCI_MISC_H
