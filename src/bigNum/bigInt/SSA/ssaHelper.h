#ifndef FIBONACCI_SSAHELPER_H
#define FIBONACCI_SSAHELPER_H

#include "../bigInt.h"

bigInt *sliceBigInt(const bigInt *x, size_t offset, size_t chunkSize);

bigInt *reduceModF(const bigInt *x, size_t fermatIndex);

bigInt *rotateLeftModF(const bigInt *x, size_t k, size_t totalBits);

bigInt *rotateRightModF(const bigInt *x, size_t k, size_t totalBits);

bigInt *addModF(const bigInt *a, const bigInt *b, size_t fermatIndex);

bigInt *subModF(const bigInt *a, const bigInt *b, size_t fermatIndex);

bigInt *getFirstNBits(const bigInt *x, size_t n);

void reduceToFirstNBits(bigInt *x, size_t n);

#endif //FIBONACCI_SSAHELPER_H
