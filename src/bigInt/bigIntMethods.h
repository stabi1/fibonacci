#ifndef FIBONACCI_BIGINTMETHODS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bigInt.h"

#define FIBONACCI_BIGINTMETHODS_H

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

bigInt *getLowerFrom(bigInt *x, size_t n);

bigInt *getUpperFrom(bigInt *x, size_t n);

bigInt *shiftAdd(bigInt *x, bigInt *toShift, size_t n);

void getToomSlice(bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt *erg[]);

bigInt *getBlock(bigInt *x, size_t index, size_t numBlocks, size_t blockLength);

int compareShiftedBigInt(bigInt *a, bigInt *b, size_t n);

bigInt *getZeroBigInt();

bool isZero(bigInt *x);

size_t getOccupiedBlocks(bigInt *x);

int compareBigIntArrays(bigInt *a, bigInt *b);

bool isValidBigInt(bigInt *x);

bigInt *stripLeadingZeros(bigInt *x);

#endif //FIBONACCI_BIGINTMETHODS_H
