#include "bigFracRoot.h"

#include "bigFracMethods.h"
#include "../bigInt/bigIntMethods.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

bigFrac *sqrtInitialGuess(const bigFrac *radicand) {
    radicand->bigIntPart->start += radicand->fractionBlocks;
    size_t bitLen = bitLength(radicand->bigIntPart);
    size_t holeLenBlocks = getLen(radicand->bigIntPart);
    radicand->bigIntPart->start -= radicand->fractionBlocks;

    if (holeLenBlocks == 1) {
        double estimateDouble = sqrt(radicand->bigIntPart->bigIntArray[radicand->bigIntPart->end - 1]);
        char estimate[200];
        sprintf(estimate, "%f", estimateDouble);
        bigFrac *refinedEstimate = decStringToBigFrac(estimate, true, 1);
        return refinedEstimate;
    }

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


bool fractionBlocksEqualOld(bigFrac *a, bigFrac *b, size_t wantedFractionBlocks) {
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


bigFrac *sqrt2Old(const bigFrac *radicand, const size_t wantedFractionBlocks) {
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

        // Divide by 2
        bigFrac *xNew = shiftRightBigFrac(sum, 1);
        freeBigFrac(sum);

        // Check convergence
        if (fractionBlocksEqualOld(xNew, x, wantedFractionBlocks)) {
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


bigFrac *sqrt2(const bigFrac *radicand, const size_t wantedFractionBlocks) {
    if (isZero(radicand->bigIntPart)) return getBigFracFromUnsignedInteger(0);
    if (radicand->bigIntPart->negative) {
        fprintf(stderr, "No root of negative numbers allowed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize x with 1 block of correct fraction bits
    bigFrac *x = sqrt2Old(radicand, 1);

    size_t neededIterations = ceil(log2((double) wantedFractionBlocks));

    size_t iteration = 1;
    size_t currentSize = 1;

    while (true) {
        currentSize *= 2;

        // Compute n/x
        bigFrac *quotient;
        if (iteration == neededIterations) {
            quotient = divideBigFrac(radicand, x, wantedFractionBlocks);
        } else {
            quotient = divideBigFrac(radicand, x, currentSize);
        }

        // Compute (x + quotient)
        bigFrac *sum = addBigFrac(x, quotient);
        freeBigFrac(quotient);
        freeBigFrac(x);

        // Divide by 2
        bigFrac *xNew = shiftRightBigFrac(sum, 1);
        freeBigFrac(sum);

        // Convergence reached
        if (iteration == neededIterations) {
            removeLeadingAndTrailingZeroBlocks(xNew);
            return xNew;
        }

        // Update x for next iteration
        x = xNew;
        iteration++;
    }
}
