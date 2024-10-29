#ifndef FIBONACCI_BIGINTUTIL_H
#define FIBONACCI_BIGINTUTIL_H

#include "bigInt.h"
#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void mallocCheck(void *p);

size_t custom_lzcnt(uint64_t n);

uint64_t bitLength(bigInt *x);

char *extendHexString(char *hex);

char *trimZeroes(char *str);

char *uint64tToHexString(uint64_t *array, size_t lenInBytes, size_t start);

char *uint64_t_toDecString(uint64_t x);

char *bigIntToDecStringHelper(bigInt *x);

void bigIntToDecStringSchoenhage(bigInt *x, size_t digits, char **resString, size_t *resStringCounter, size_t *resMaxLen);

char *bigIntToDecStringSmall(bigInt *x);

size_t hexToNibble(char hex);

#endif //FIBONACCI_BIGINTUTIL_H
