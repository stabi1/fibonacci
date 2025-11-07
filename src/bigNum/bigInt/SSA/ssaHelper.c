#include "ssaHelper.h"

#include "../bigIntAlloc.h"
#include "../bigIntMethods.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//TODO overlap with getBlock

// return a bigInt (NOT array owner) where offset is the offset in x
// and chunkSize is the size of the slice.
// If the slice is out of bounds, return a zero bigInt.
bigInt *sliceBigInt(const bigInt *x, const size_t offset, const size_t chunkSize) {
    const size_t resStart = x->start + offset;
    if (resStart >= x->end) {
        return getZeroBigInt();
    }

    size_t thisSliceSize = chunkSize;
    if (resStart + thisSliceSize >= x->end) {
        thisSliceSize = x->end - resStart;
    }
    bigInt *result = newBigIntStruct(resStart, resStart + thisSliceSize, x->bigIntArray);
    stripLeadingZeros(result);
    return result;
}

// computes the minimal positive remainder of x mod F, where F is the fermatIndex-th fermat number
bigInt *reduceModF(const bigInt *x, const size_t fermatIndex) {
    const size_t totalBits = 1ULL << (fermatIndex + 1);
    const size_t k = totalBits / 2;

    bigInt *v = sliceBigInt(x, k/64, k/64); // high bits
    bigInt *u = sliceBigInt(x, 0, k/64); // low bits
    bigInt *res;

    if (compareBigInt(v, u) == 1) {// v > u -> add 2^{2^fermatIndex} + 1
        bigInt *one = getBigIntFromUnsignedInteger(1);
        bigInt *tmp = add(u, one);

        const size_t nBits = 1ULL << fermatIndex;
        shiftAddSameNumberSafe(tmp, one, nBits / 64); // nBits % 64 is always zero
        freeBigInt(one);

        res = sub(tmp, v);
        freeBigInt(tmp);
    } else {
        res = sub(u, v);
    }
    freeBigInt(v);
    freeBigInt(u);
    return res;
}

// rotates x left by k bits. totalBits is the number of bits of x to where the rotation is applied.
bigInt *rotateLeftModF(const bigInt *x, size_t k, size_t totalBits) {
    if (k == 0) {
        return copyBigInt(x);
    }
    if (totalBits == 0) {
        return getZeroBigInt();
    }

    k = k % totalBits;
    bigInt *res;
    if (k % 64 == 0 && totalBits % 64 == 0) {
        const size_t lowLength = totalBits / 64 - k / 64;
        bigInt *low = sliceBigInt(x, 0, lowLength);
        bigInt *high = sliceBigInt(x, lowLength, getLen(x) - lowLength);
        res = shiftAdd(high, low, k / 64);
        freeBigInt(low);
        freeBigInt(high);
    } else {
        bigInt *high = shiftRight(x, totalBits - k);
        bigInt *low = getFirstNBits(x, totalBits - k);
        bigInt *tmp = shiftLeft(low, k);
        freeBigInt(low);
        res = add(tmp, high);
        freeBigInt(tmp);
        freeBigInt(high);
    }

    return res;
}

// rotates x right by k bits. totalBits is the number of bits of x to where the rotation is applied.
bigInt *rotateRightModF(const bigInt *x, size_t k, const size_t totalBits) {
    if (k == 0) {
        return copyBigInt(x);
    }
    if (totalBits == 0) {
        return getZeroBigInt();
    }

    k = k % totalBits;
    bigInt *res;
    if (k % 64 == 0 && totalBits % 64 == 0) {
        const size_t lowLength = k / 64;
        bigInt *low = sliceBigInt(x, 0, lowLength);
        bigInt *high = sliceBigInt(x, lowLength, getLen(x) - lowLength);
        res = shiftAdd(high, low, totalBits / 64 - k / 64);
        freeBigInt(low);
        freeBigInt(high);
    } else {
        bigInt *high = shiftRight(x, k);
        bigInt *low = getFirstNBits(x, k);
        bigInt *tmp = shiftLeft(low, totalBits - k);
        freeBigInt(low);
        res = add(tmp, high);
        freeBigInt(high);
        freeBigInt(tmp);
    }

    return res;
}

// (a + b) mod F, where F is the fermatIndex-th fermat number
bigInt *addModF(const bigInt *a, const bigInt *b, const size_t fermatIndex) {
    bigInt *s = add(a, b);

    const size_t maxBitlength = 1ULL << (fermatIndex + 1);
    while (bitLength(s) > maxBitlength) {
        bigInt *low;
        bigInt *high;
        if (maxBitlength >= 64) {
            const size_t maxlength = maxBitlength / 64;
            low = sliceBigInt(s, 0, maxlength);
            high = sliceBigInt(s, maxlength, getLen(s) - maxlength);
        } else {
            low = getFirstNBits(s, maxBitlength);
            high = shiftRight(s, maxBitlength);
        }

        bigInt *tmp = add(low, high);
        freeBigInt(s);
        freeBigInt(low);
        freeBigInt(high);
        s = tmp;
    }
    return s;
}

// (a - b) mod F, where F is the fermatIndex-th fermat number
bigInt *subModF(const bigInt *a, const bigInt *b, const size_t fermatIndex) {
    bigInt *rotated = rotateLeftModF(b, 1ULL << fermatIndex, 1ULL << (fermatIndex + 1));
    bigInt *res = addModF(a, rotated, fermatIndex);
    freeBigInt(rotated);
    return res;
}

// return a bigInt with the first n bits of x
bigInt *getFirstNBits(const bigInt *x, size_t n) {
    if (n == 0 || isZero(x)) {
        return getZeroBigInt();
    }

    // Calculate how many complete 64-bit words we need
    const size_t completeWords = n / 64;
    const size_t remainingBits = n % 64;

    // Calculate how many words we actually have available
    const size_t availableWords = getLen(x);

    // Determine how many words we'll actually use
    const size_t wordsNeeded = completeWords + (remainingBits > 0 ? 1 : 0);
    const size_t actualWords = completeWords < availableWords ? wordsNeeded : availableWords;

    // Create new bigInt with the required size
    bigInt *result = newBigInt(actualWords);

    // Copy the complete words
    memcpy(result->bigIntArray, x->bigIntArray + x->start, actualWords * 8);

    const size_t i = actualWords - 1;
    // Handle the last word
    if (completeWords < availableWords && remainingBits > 0) {
        // Create mask for remaining bits: (1 << remainingBits) - 1
        const uint64_t mask = (1ULL << remainingBits) - 1;
        result->bigIntArray[i] = x->bigIntArray[x->start + i] & mask;
    }

    // Strip any leading zeros that might have been created by masking
    stripLeadingZeros(result);
    return result;
}

void reduceToFirstNBits(bigInt *x, const size_t n) {
    if (!x->arrayOwner) {
        fprintf(stderr, "reduceToFirstNBits: x is not array owner\n");
        exit(EXIT_FAILURE);
    }

    // Calculate how many complete 64-bit words we need
    const size_t completeWords = n / 64;
    const size_t remainingBits = n % 64;

    // Calculate how many words we actually have available
    const size_t availableWords = getLen(x);

    // Determine how many words we'll actually use
    const size_t wordsNeeded = completeWords + (remainingBits > 0 ? 1 : 0);
    const size_t actualWords = completeWords < availableWords ? wordsNeeded : availableWords;

    // Copy the complete words
    const size_t i = actualWords - 1;

    // Handle the last word
    if (completeWords < availableWords && remainingBits > 0) {
        // Create mask for remaining bits: (1 << remainingBits) - 1
        const uint64_t mask = (1ULL << remainingBits) - 1;
        x->bigIntArray[x->start + i] = x->bigIntArray[x->start + i] & mask;
    }
    x->end = x->start + actualWords;

    // Strip any leading zeros that might have been created by masking
    stripLeadingZeros(x);
}
