#ifndef IMPLEMENTIERUNG_KARATSUBA_H
#define IMPLEMENTIERUNG_KARATSUBA_H

#include <stddef.h>
#include "bigIntMethods.h"

bigInt *mulSingleThread(bigInt *x, bigInt *y);

bigInt *mulParallel(bigInt *x, bigInt *y, size_t depth);

bigInt *karatsuba(bigInt *x, bigInt *y);

bigInt *multiplyToomCook3(bigInt *a, bigInt *b);

bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth);

#endif //IMPLEMENTIERUNG_KARATSUBA_H
