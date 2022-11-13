#ifndef FIBONACCI_BIGINTUTIL_H
#define FIBONACCI_BIGINTUTIL_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

char *extendHexString(char *hex);

char *trimZeroes(char *str);

char *uint64tToHexString(uint64_t *array, size_t lenInBytes, size_t start);

char *uint64tToDecString(uint64_t *array, size_t lenInBytes);

size_t hexToNibble(char hex);

#endif //FIBONACCI_BIGINTUTIL_H
