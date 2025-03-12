#include "bigFracMethods.h"

#include "../bigInt/bigIntMethods.h"
#include "../misc.h"

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
    size_t fractionBlocks = (totalBits - x->fractionBits) / 64;
    size_t trailingZeroBlocks = getTrailingZeroBlocks(x->bigIntPart);
    x->bigIntPart->start += min(trailingZeroBlocks, fractionBlocks);
}
