#include "bigFracRoot.h"

#include "bigFracMethods.h"

#include <stdio.h>

bigFrac *sqrtInitialGuess(const bigFrac *radicand) {
    radicand->bigIntPart->start += radicand->fractionBlocks;
    size_t bitLen = bitLength(radicand->bigIntPart);
    radicand->bigIntPart->start -= radicand->fractionBlocks;

    size_t approxExponent = bitLen / 2;

    // Compute initial estimate
    bigFrac *one = getBigFracFromUnsignedInteger(1);
    bigFrac *estimate = shiftLeftBigFrac(one, approxExponent);

    // Improve by adding a small fraction based on the highest bits of the radicand
    bigFrac *topBits = getBigFracFromUnsignedInteger(radicand->bigIntPart->bigIntArray[radicand->bigIntPart->end - 1]); // Extract top 10 bits
    bigFrac *tmp = shiftLeftBigFrac(one, bitLen);
    freeBigFrac(one);
    bigFrac *fractionPart = divideBigFrac(topBits, tmp, getLen(tmp->bigIntPart));
    freeBigFrac(topBits);
    freeBigFrac(tmp);

    bigFrac *refinedEstimate = addBigFrac(estimate, fractionPart);
    freeBigFrac(estimate);
    freeBigFrac(fractionPart);

    return refinedEstimate;
}


bool fractionBlocksEqual(bigFrac *a, bigFrac *b, size_t wantedFractionBlocks) {
    if (a->fractionBlocks < wantedFractionBlocks || b->fractionBlocks < wantedFractionBlocks) {
        return false;
    }

    for (size_t i = 0; i < wantedFractionBlocks; i++) {
        uint64_t aVal = a->bigIntPart->bigIntArray[a->bigIntPart->start + i];
        uint64_t bVal = b->bigIntPart->bigIntArray[b->bigIntPart->start + i];
        if (aVal != bVal) {
            return false;
        }
    }
    return true;
}


bigFrac *sqrt2(const bigFrac *radicand, const size_t wantedFractionBlocks) {
    if (isZero(radicand->bigIntPart)) return getBigFracFromUnsignedInteger(0);
    if (radicand->bigIntPart->negative) {
        fprintf(stderr, "No root of negative numbers allowed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize x
    bigFrac *x = sqrtInitialGuess(radicand);

    size_t bufferBlocks = 5;

    while (true) {
        // Compute n/x
        bigFrac *quotient = divideBigFrac(radicand, x, wantedFractionBlocks + bufferBlocks);

        // Compute (x + quotient)
        bigFrac *sum = addBigFrac(x, quotient);
        freeBigFrac(quotient);

        // Divide by 2: could use right-shift if your representation allows
        bigFrac *xNew = shiftRightBigFrac(sum, 1);
        freeBigFrac(sum);

        // Check convergence
        if (fractionBlocksEqual(xNew, x, wantedFractionBlocks)) {
            freeBigFrac(x);
            xNew->bigIntPart->start += bufferBlocks;
            xNew->fractionBlocks -= bufferBlocks;
            removeLeadingAndTrailingZeroBlocks(xNew);
            return xNew;
        }

        freeBigFrac(x);
        // Update x for next iteration
        x = xNew;
    }
}