#ifndef FIBONACCI_BIGFRAC_H
#define FIBONACCI_BIGFRAC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bigInt/bigInt.h"

typedef struct BigFracTag { //The datastructure that represents a binary fraction number, the sign of the bigFrac is stored in bigIntPart
    bigInt *bigIntPart;
    size_t fractionBits; //how many bits of bigIntPart belong to the fraction part
} bigFrac;


// create/delete
bigFrac *newBigFrac(size_t len);

void freeBigFrac(bigFrac *toDelete);

bigFrac *newBigFracFromBigInt(bigInt *x, bool copy);

bigFrac* getBigFracFromUnsignedInteger(uint64_t integer);

bigFrac* getBigFracFromSignedInteger(int64_t integer);

// calc methods
bigFrac *mulBigFrac(const bigFrac *x, const bigFrac *y);


#endif
