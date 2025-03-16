#ifndef FIBONACCI_BIGFRAC_H
#define FIBONACCI_BIGFRAC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../bigInt/bigInt.h"

typedef struct BigFracTag { //The datastructure that represents a binary fraction number, the sign of the bigFrac is stored in bigIntPart
    bigInt *bigIntPart;
    size_t fractionBlocks; //how many blocks of bigIntPart belong to the fraction part, means that the . must always at the border between 2 blocks
} bigFrac;


// create/delete
bigFrac *newBigFrac(size_t len);

void freeBigFrac(bigFrac *toDelete);

bigFrac *newBigFracFromBigInt(bigInt *x, bool copy);

bigFrac *getBigFracFromUnsignedInteger(uint64_t integer);

bigFrac *getBigFracFromSignedInteger(int64_t integer);

// compare methods:
int compareBigFrac(const bigFrac *a, const bigFrac *b);

// calc methods
bigFrac *addBigFrac(const bigFrac *x, const bigFrac *y);

bigFrac *subBigFrac(const bigFrac *x, const bigFrac *y);

bigFrac *mulBigFrac(const bigFrac *x, const bigFrac *y);

bigFrac *divideBigFrac(const bigFrac *dividend, const bigFrac *divisor, size_t wantedFractionBlocks);

bigFrac *shiftLeftBigFrac(const bigFrac *x, size_t n);

bigFrac *shiftRightBigFrac(const bigFrac *x, size_t n);

bigFrac* sqrt2(const bigFrac* radicand, size_t wantedFractionBlocks);

// string methods
char *bigFracToHexString(const bigFrac *x);

bigFrac *hexStringToBigFrac(const char *decStr);

char *bigFracToDecString(const bigFrac *x, bool exactPrecision);

bigFrac *decStringToBigFrac(const char *decStr, bool automaticPrecision, size_t binaryDigits);

// IO methods
void printBigFracHex(const bigFrac *x);

void printBigFracDec(const bigFrac *x, bool exactPrecision);

//higherFunctions
bigFrac* goldenRatio(size_t binaryDigits);

bigFrac *pi(size_t binaryDigits);

bigFrac *e(size_t binaryDigits);

#endif
