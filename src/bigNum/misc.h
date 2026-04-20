#ifndef FIBONACCI_MISC_H
#define FIBONACCI_MISC_H

#include <stdlib.h>
#include <stdint.h>

void mallocCheck(const void *p);

char *getZeroString();

char *getZeroDotZeroString();

char *getCurrentDateTime();

size_t custom_lzcnt(uint64_t n);

size_t custom_tzcnt(uint64_t n);

size_t max(size_t a, size_t b);

size_t min(size_t a, size_t b);

uint64_t bit_reverse(uint64_t x, uint64_t bits);

uint64_t gcd(uint64_t a, uint64_t b);

uint64_t lcm(uint64_t a, uint64_t b);

#endif //FIBONACCI_MISC_H
