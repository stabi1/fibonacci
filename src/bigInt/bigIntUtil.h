#ifndef FIBONACCI_BIGINTUTIL_H
#define FIBONACCI_BIGINTUTIL_H

#include "bigInt.h"
#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

char *uint64tArrayToHexString(const uint64_t *array, size_t lenInNibbles, size_t start, bool negative);

void decStringToBigIntHelper(uint64_t *array, size_t arrayLen, const char *decStr, size_t decStrLen);

char *uint64_t_toDecString(uint64_t x);

char *bigIntToDecStringHelper(bigInt *x, bool doFree);

char* bigIntToDecStringSchoenhage(bigInt *x);

char* bigIntToDecStringSchoenhageMultithread(bigInt *x);

char *bigIntToDecStringSmall(bigInt *x);

size_t hexToNibble(char hex);

#endif //FIBONACCI_BIGINTUTIL_H
