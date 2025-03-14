#include "bigFracMul.h"

#include "bigFracMethods.h"

#include <stdio.h>

bigFrac *mulBigFrac(const bigFrac *x, const bigFrac *y) {
    bigFrac *res = mulBigFracNoResize(x, y);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}

bigFrac *mulBigFracNoResize(const bigFrac *x, const bigFrac *y) {
    bigInt *bigIntRes = mul(x->bigIntPart, y->bigIntPart);
    bigFrac *res = newBigFracFromBigInt(bigIntRes, false);
    res->fractionBlocks = x->fractionBlocks + y->fractionBlocks;
    return res;
}



