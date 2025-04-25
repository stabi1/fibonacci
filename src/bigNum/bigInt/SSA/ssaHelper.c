#include "ssaHelper.h"

#include "../bigIntAlloc.h"
#include "../bigIntMethods.h"

#include <stdio.h>

//TODO overlap with getBlock

// return a bigInt (NOT array owner) where offset is the offset of (offset * chunkSize) in x
// and chunkSize is the size of the slice.
// If the slice is out of bounds, return a zero bigInt.
bigInt *sliceBigInt(const bigInt *x, size_t offset, size_t chunkSize) {
    size_t resStart = x->start + offset * chunkSize;
    if (resStart >= x->end) {
        return getZeroBigInt();
    }

    size_t thisSliceSize = chunkSize;
    if (resStart + thisSliceSize >= x->end) {
        thisSliceSize = x->end - resStart;
    }
    bigInt *result = stripLeadingZeros(newBigIntStruct(resStart, resStart + thisSliceSize, x->bigIntArray));
    return result;
}

// computes the minimal positive remainder of x mod F, where F is the fermatIndex-th fermat number
bigInt *reduceModF(const bigInt *x, const bigInt *fermatNumber, size_t fermatIndex) {
    size_t totalBits = 1ULL << (fermatIndex + 1);
    size_t k = totalBits/2;

    bigInt *v = shiftRight(x, totalBits - k); // high bits
    bigInt *u = getFirstNBits(x, totalBits - k); // low bits
    bigInt *res;

    if(compareBigInt(v, u) == 1) { // v > u
        bigInt *tmp = sub(u, v);
        res = add(tmp, fermatNumber);
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
    if(k == 0) {
        return copyBigInt(x);
    }
    k = k % totalBits;

    bigInt *high = shiftRight(x, totalBits - k);
    bigInt *low = getFirstNBits(x, totalBits - k);
    bigInt *tmp = shiftLeft(low, k);
    freeBigInt(low);
    bigInt *res = add(tmp, high);
    freeBigInt(tmp);
    freeBigInt(high);
    return res;
}

// rotates x right by k bits. totalBits is the number of bits of x to where the rotation is applied.
bigInt *rotateRightModF(const bigInt *x, size_t k, size_t totalBits) {
    if(k == 0) {
        return copyBigInt(x);
    }
    k = k % totalBits;

    bigInt *high = shiftRight(x,  k);
    bigInt *low = getFirstNBits(x, k);
    bigInt *tmp = shiftLeft(low, totalBits - k);
    freeBigInt(low);
    bigInt *res = add(tmp, high);
    freeBigInt(high);
    freeBigInt(tmp);
    return res;
}

// (a + b) mod F, where F is the fermatIndex-th fermat number
bigInt *addModF(const bigInt *a, const bigInt *b, size_t fermatIndex) {
    bigInt *s = add(a, b);

    size_t maxBitlength = 1ULL << (fermatIndex + 1);
    while (bitLength(s) > maxBitlength) {
        bigInt *low = getFirstNBits(s, maxBitlength);
        bigInt *high = shiftRight(s, maxBitlength);
        freeBigInt(s);
        s = add(low, high);
        freeBigInt(low);
        freeBigInt(high);
    }
    return s;
}

// (a - b) mod F, where F is the fermatIndex-th fermat number
bigInt *subModF(const bigInt *a, const bigInt *b, size_t fermatIndex) {
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
    size_t completeWords = n / 64;
    size_t remainingBits = n % 64;

    // Calculate how many words we actually have available
    size_t availableWords = x->end - x->start;

    // Determine how many words we'll actually use
    size_t wordsNeeded = completeWords + (remainingBits > 0 ? 1 : 0);
    size_t actualWords = wordsNeeded < availableWords ? wordsNeeded : availableWords;

    // Create new bigInt with the required size
    bigInt *result = newBigInt(actualWords);

    // Copy the complete words
    size_t i;
    for (i = 0; i < actualWords - 1; i++) {
        result->bigIntArray[i] = x->bigIntArray[x->start + i];
    }

    // Handle the last word
    if (remainingBits > 0 && i < availableWords) {
        // Create mask for remaining bits: (1 << remainingBits) - 1
        uint64_t mask = (1ULL << remainingBits) - 1;
        result->bigIntArray[i] = x->bigIntArray[x->start + i] & mask;
    } else if (i < availableWords) {
        // Copy the last complete word
        result->bigIntArray[i] = x->bigIntArray[x->start + i];
    }

    // Strip any leading zeros that might have been created by masking
    result = stripLeadingZeros(result);

    return result;
}
