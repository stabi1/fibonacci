#ifndef FIBONACCI_BIGINTUTIL_H
#define FIBONACCI_BIGINTUTIL_H

#include "bigInt.h"
#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void mallocCheck(void *p);

char *uint64tArrayToHexString(uint64_t *array, size_t lenInNibbles, size_t start, bool negative);

void decStringToBigIntHelper(uint64_t *array, size_t arrayLen, const char *decStr, size_t decStrLen);

char *uint64_t_toDecString(uint64_t x);

char *bigIntToDecStringHelper(bigInt *x);

void bigIntToDecStringSchoenhage(bigInt *x, size_t digits, char **resString, size_t *resStringCounter, size_t *resMaxLen);

char *bigIntToDecStringSmall(bigInt *x);

size_t hexToNibble(char hex);

#endif //FIBONACCI_BIGINTUTIL_H
