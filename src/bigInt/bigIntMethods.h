#ifndef FIBONACCI_BIGINTMETHODS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bigInt.h"

#define FIBONACCI_BIGINTMETHODS_H

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

void exception();

bigInt *getLowerFrom(bigInt *x, size_t n);

bigInt *getUpperFrom(bigInt *x, size_t n);

void getToomSlice(bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt *erg[]);

bigInt *exactDivideBy3(bigInt *x);

#endif //FIBONACCI_BIGINTMETHODS_H
