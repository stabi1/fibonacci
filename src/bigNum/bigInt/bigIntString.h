#ifndef FIBONACCI_BIGINTSTRING_H
#define FIBONACCI_BIGINTSTRING_H

#include "bigInt.h"
#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

char *uint64tArrayToHexString(const uint64_t *array, size_t lenInNibbles, size_t start, bool negative);

void decStringToBigIntHelper(uint64_t *array, size_t arrayLen, const char *decStr, size_t decStrLen);

char *uint64_t_toDecString(uint64_t x);

char *bigIntToDecStringHelper(bigInt *x, bool doFree);

char* bigIntToDecStringSchoenhage(bigInt *x);

char* bigIntToDecStringSchoenhageMultithread(bigInt *x);

char *bigIntToDecStringSmall(bigInt *x);

size_t hexToNibble(char hex);

bigInt *decStringToBigIntLength(const char *decStr, size_t strLen);

#endif //FIBONACCI_BIGINTSTRING_H
