#ifndef IMPLEMENTIERUNG_KARATSUBA_H

#include <stddef.h>
#include "bigIntMethods.h"

#define IMPLEMENTIERUNG_KARATSUBA_H

// size_t naiveMulFaster;
// size_t karatsubaFaster;

bigInt *karatsuba(bigInt *x, bigInt *y);

bigInt *multiplyToomCook3(bigInt *a, bigInt *b);

bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth);

#endif //IMPLEMENTIERUNG_KARATSUBA_H
