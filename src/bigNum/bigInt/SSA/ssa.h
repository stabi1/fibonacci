#ifndef FIBONACCI_SSA_H
#define FIBONACCI_SSA_H

#include "../bigInt.h"

bigInt *SSA_modular(const bigInt *A, const bigInt *B);

bigInt** FFT_modF(bigInt** a, size_t N, const size_t fermatIndex);

#endif //FIBONACCI_SSA_H
