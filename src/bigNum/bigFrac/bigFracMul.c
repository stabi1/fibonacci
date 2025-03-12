#include "bigFracMul.h"

#include "bigFracMethods.h"

bigFrac *mulBigFrac(const bigFrac *x, const bigFrac *y) {
    bigInt* bigIntRes = mul(x->bigIntPart, y->bigIntPart);
    bigFrac * res = newBigFracFromBigInt(bigIntRes, false);
    res->fractionBits = x->fractionBits + y->fractionBits;
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}


