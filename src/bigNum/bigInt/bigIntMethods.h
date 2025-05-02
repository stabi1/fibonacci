#ifndef FIBONACCI_BIGINTMETHODS_H
#define FIBONACCI_BIGINTMETHODS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bigInt.h"

bigInt *getLowerFrom(const bigInt *x, size_t n);

bigInt *getUpperFrom(const bigInt *x, size_t n);

void shiftAddSameNumber(const bigInt *x, const bigInt *toShift, const size_t n);

bigInt *shiftAdd(const bigInt *x, const bigInt *toShift, size_t n);

void getToomSlice(const bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt *erg[]);

bigInt *getBlock(const bigInt *x, size_t index, size_t numBlocks, size_t blockLength);

int compareShiftedBigInt(const bigInt *a, const bigInt *b, size_t n);

size_t getOccupiedBlocks(const bigInt *x);

size_t getTrailingZeroBlocks(const bigInt *x);

int compareBigIntArrays(const bigInt *a, const bigInt *b);

bool isValidBigInt(const bigInt *x);

void stripLeadingZeros(bigInt *x);

#endif //FIBONACCI_BIGINTMETHODS_H
