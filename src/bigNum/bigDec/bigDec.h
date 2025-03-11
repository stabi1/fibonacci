#ifndef FIBONACCI_BIGDEC_H
#define FIBONACCI_BIGDEC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bigInt/bigInt.h"

typedef struct BigDecTag { //The datastructure that represents a binary fraction number
    bigInt *bigIntAbs;
    size_t fractionBits;
    bool negative;
} bigDec;

#endif
