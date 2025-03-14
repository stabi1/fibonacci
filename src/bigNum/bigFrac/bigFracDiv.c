#include "bigFracDiv.h"

#include "bigFracMethods.h"

#include <stdio.h>

bigFrac *divideBigFrac(const bigFrac *dividend, const bigFrac *divisor, const size_t wantedFractionBlocks) {
    if (isZero(divisor->bigIntPart)) {
        fprintf(stderr, "Division by zero!\n");
        exit(EXIT_FAILURE);
    }

    // Compute the required bit-shift:
    int64_t shiftBits = 64 * ((int64_t) wantedFractionBlocks - ((int64_t) dividend->fractionBlocks - (int64_t) divisor->fractionBlocks));

    // Shift the dividend by the computed shift amount.
    bigInt *shiftedDividend = dividend->bigIntPart;
    if (shiftBits > 0) {
        shiftedDividend = shiftLeft(dividend->bigIntPart, (size_t) shiftBits);
    } else if (shiftBits < 0) {
        // If negative, shift right. Assume you have a function bigIntShiftRight.
        shiftedDividend = shiftRight(dividend->bigIntPart, (size_t) (-shiftBits));
    }

    // Now perform the division: shiftedDividend / B->bigIntPart.
    bigInt *quotient = divide(shiftedDividend, divisor->bigIntPart);

    if(shiftBits != 0) {
        freeBigInt(shiftedDividend);
    }

    quotient->negative = dividend->bigIntPart->negative ^ divisor->bigIntPart->negative;
    bigFrac *res = newBigFracFromBigInt(quotient, false);
    res->fractionBlocks = wantedFractionBlocks;
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}
