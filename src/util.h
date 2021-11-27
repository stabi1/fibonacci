#ifndef FIBONACCI_UTIL_H
#define FIBONACCI_UTIL_H

#include <stdint-gcc.h>
#include <stdbool.h>
#include <stddef.h>

char *extendHexString(char *hex);

char *trimZeroes(char *str);

bool checkIsNumber(char* value);

size_t custom_lzcnt(uint64_t);

char* uint64tToHexString(uint64_t* array, size_t lenInBytes, size_t start);

char* uint64tToDecString(uint64_t* array, size_t lenInBytes);

size_t hexToNibble(char hex);

#endif //FIBONACCI_UTIL_H
