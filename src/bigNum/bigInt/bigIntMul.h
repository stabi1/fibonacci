#ifndef IMPLEMENTIERUNG_KARATSUBA_H
#define IMPLEMENTIERUNG_KARATSUBA_H

#include <stddef.h>
#include "bigIntMethods.h"

bigInt *karatsuba(const bigInt *x, const bigInt *y);

bigInt *multiplyToomCook3(const bigInt *a, const bigInt *b);

bigInt *multiplyToomCook3MultiThread(const bigInt *a, const bigInt *b, size_t depth);

#endif //IMPLEMENTIERUNG_KARATSUBA_H
