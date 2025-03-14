#include "bigFracHigherFunctions.h"

#include <math.h>

double goldenRatioDouble = 1.618033988749;

bigFrac* goldenRatio(size_t binaryDigits) {
    size_t nFib = (binaryDigits + log2(sqrt(5)))/log2(goldenRatioDouble); // accurate Estimate
    nFib += 10; // to be safe

    bigInt* fibN = fibonacci(nFib);
    bigInt* fibNMinus1 = fibonacci(nFib - 1);

    bigFrac* fibNFrac = newBigFracFromBigInt(fibN, false);
    bigFrac *fibNMinus1Frac = newBigFracFromBigInt(fibNMinus1, false);

    bigFrac *res = divideBigFrac(fibNFrac, fibNMinus1Frac, getLen(fibNMinus1));

    freeBigFrac(fibNFrac);
    freeBigFrac(fibNMinus1Frac);
    return res;
}
