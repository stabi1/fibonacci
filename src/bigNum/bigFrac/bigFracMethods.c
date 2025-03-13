#include "bigFracMethods.h"

#include "../bigInt/bigIntMethods.h"
#include "../misc.h"

#include <stdio.h>

bigFrac *getBigFracFromUnsignedInteger(uint64_t integer) {
    bigInt *bigIntPart = getBigIntFromUnsignedInteger(integer);
    return newBigFracFromBigInt(bigIntPart, false);
}

bigFrac *getBigFracFromSignedInteger(int64_t integer) {
    bigInt *bigIntPart = getBigIntFromSignedInteger(integer);
    return newBigFracFromBigInt(bigIntPart, false);
}

void removeLeadingAndTrailingZeroBlocks(bigFrac *x) {
    size_t xLen = getLen(x->bigIntPart);

    //Leading Zeros
    size_t totalBits = xLen * 64;
    size_t holeBlocks = (totalBits - x->fractionBits) / 64;
    size_t leadingZeroBlocks = xLen - getOccupiedBlocks(x->bigIntPart);
    x->bigIntPart->end -= min(leadingZeroBlocks, holeBlocks);

    //Trailing
    size_t fractionBlocks = x->fractionBits / 64;
    size_t trailingZeroBlocks = getTrailingZeroBlocks(x->bigIntPart);
    x->bigIntPart->start += min(trailingZeroBlocks, fractionBlocks);
    x->fractionBits -= min(trailingZeroBlocks, fractionBlocks) * 64;
}

bigFrac *alignToBlock(const bigFrac *x) {
    size_t toShift = (64 - (x->fractionBits % 64)) % 64;
    bigFrac *res = shiftLeftBigFrac(x, toShift);
    res->fractionBits += toShift;
    return res;
}

bigFrac *shiftLeftBigFrac(const bigFrac *x, const size_t n) {
    bigInt *shiftedBigInt = shiftLeft(x->bigIntPart, n);
    bigFrac *res = newBigFracFromBigInt(shiftedBigInt, false);
    res->fractionBits = x->fractionBits;
    return res;
}

bigFrac *shiftRightBigFrac(const bigFrac *x, const size_t n) {
    bigInt *shiftedBigInt = shiftRight(x->bigIntPart, n);
    bigFrac *res = newBigFracFromBigInt(shiftedBigInt, false);
    res->fractionBits = x->fractionBits;
    return res;
}
