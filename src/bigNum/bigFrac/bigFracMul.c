#include "bigFracMul.h"

#include "bigFracMethods.h"

#include <string.h>

bigFrac *mulBigFrac(const bigFrac *x, const bigFrac *y) {
    bigFrac *res = mulBigFracNoResize(x, y);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}

bigFrac *mulBigFracNoResize(const bigFrac *x, const bigFrac *y) {
    bigInt *bigIntRes = mul(x->bigIntPart, y->bigIntPart);
    size_t resFractionBlocks = x->fractionBlocks + y->fractionBlocks;

    // resize if hole part is zero and fraction part has leading zeros -> mul cuts of leading zeros from res
    if (getLen(bigIntRes) < resFractionBlocks) {
        size_t missingLeadingZeroBLocks = resFractionBlocks - getLen(bigIntRes);
        bigInt *tmp = newBigIntNotZeroed(resFractionBlocks);
        memset(tmp->bigIntArray, 0, missingLeadingZeroBLocks*8);
        memcpy(tmp->bigIntArray + missingLeadingZeroBLocks, bigIntRes->bigIntArray, getLen(bigIntRes) * 8);
        freeBigInt(bigIntRes);
        bigIntRes = tmp;
    }

    bigFrac *res = newBigFracFromBigInt(bigIntRes, false);
    res->fractionBlocks = resFractionBlocks;
    return res;
}

