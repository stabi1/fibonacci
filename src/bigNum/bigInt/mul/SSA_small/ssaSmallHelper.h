#ifndef FIBONACCI_SSASMALLHELPER_H
#define FIBONACCI_SSASMALLHELPER_H

#include "../../bigInt.h"

bigInt *reduceModNPrime(const bigInt *x, uint64_t nprime);

bigInt *mul2ExpModNPrime(const bigInt *x, uint64_t d, uint64_t nprime);

bigInt *div2ExpModNPrime(const bigInt *x, uint64_t k, uint64_t nprime);

uint64_t **fftInitL(uint64_t k);

bigInt *addModNPrime(const bigInt *a, const bigInt *b, size_t nprime);

bigInt *subModNPrime(const bigInt *a, const bigInt *b, size_t nprime);

bigInt **decompose(const bigInt *x, uint64_t K, uint64_t nprime, uint64_t M, uint64_t Mp);

#endif //FIBONACCI_SSASMALLHELPER_H
