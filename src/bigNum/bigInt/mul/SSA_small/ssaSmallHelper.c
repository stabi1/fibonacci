#include "ssaSmallHelper.h"

#include "../SSA/ssaHelper.h"
#include "../../bigIntMethods.h"
#include "../../../misc.h"

#include <stdio.h>

bigInt *reduceModNPrime(const bigInt *x, const uint64_t nprime) {
    const size_t lowLength = nprime / 64;
    bigInt *low = sliceBigInt(x, 0, lowLength);
    bigInt *high = sliceBigInt(x, lowLength, getLen(x) - lowLength);

    bigInt *res;
    if (compareBigInt(high, low) == 1) {
        // high > low -> add 2^{nprime} + 1
        bigInt *one = getBigIntFromUnsignedInteger(1);
        bigInt *tmp = add(low, one);

        shiftAddSameNumberSafe(tmp, one, nprime / 64); // nprime % 64 is always zero
        freeBigInt(one);

        res = sub(tmp, high);
        freeBigInt(tmp);
    } else {
        res = sub(low, high);
    }
    freeBigInt(low);
    freeBigInt(high);

    return res;
}

// computes x*2^d mod 2^nprime + 1
bigInt *mul2ExpModNPrime(const bigInt *x, const uint64_t d, const uint64_t nprime) {
    if (isZero(x)) {
        return getZeroBigInt();
    }
    if (d == 0) {
        return copyBigInt(x);
    }
    uint64_t dNormalized = d % (2 * nprime);

    bool negate = false;
    if (dNormalized >= nprime) {
        dNormalized -= nprime;
        negate = true;
    }
    uint64_t cut_bit = nprime - dNormalized;

    // part that wraps)
    bigInt *H = shiftRight(x, cut_bit);

    // part that stays)
    bigInt *L = getFirstNBits(x, cut_bit);

    // Result = (L << d) - H
    bigInt *L_shifted = shiftLeft(L, dNormalized);

    bigInt *res;
    if (compareBigInt(L_shifted, H) >= 0) {
        res = sub(L_shifted, H);
    } else {
        // Wrap around: (L << d) - H + (2^nprime + 1)
        bigInt *one = getBigIntFromUnsignedInteger(1);
        shiftAddSameNumberSafe(L_shifted, one, 0);
        shiftAddSameNumberSafe(L_shifted, one, nprime / 64); // nprime % 64 is always zero
        freeBigInt(one);

        res = sub(L_shifted, H);
    }

    // Handle Negation (if d was originally >= nprime)
    if (negate) {
        // TODO Optimize
        bigInt *one = getBigIntFromUnsignedInteger(1);
        bigInt *modBase = shiftLeft(one, nprime);
        bigInt *modulus = add(modBase, one);

        bigInt *finalRes = sub(modulus, res);

        freeBigInt(res);
        res = finalRes;
        freeBigInt(one); freeBigInt(modBase); freeBigInt(modulus);
    }

    freeBigInt(H);
    freeBigInt(L);
    freeBigInt(L_shifted);

    return res;
}

// computes x/2^k mod 2^nprime + 1
bigInt *div2ExpModNPrime(const bigInt *x, const uint64_t k, const uint64_t nprime) {
    const uint64_t inv_shift = 2 * nprime - k;
    bigInt *res = mul2ExpModNPrime(x, inv_shift, nprime);
    return res;
}

// Builds bit-reversal array
uint64_t **fftInitL(const uint64_t k) {
    uint64_t **l = malloc(sizeof(uint64_t *) * (k + 1));
    mallocCheck(l);
    l[0] = malloc(sizeof(uint64_t) * 1);
    mallocCheck(l[0]);
    l[0][0] = 0;
    for (size_t i = 1; i < k + 1; i++) {
        const size_t g = 1ULL << (i - 1);
        l[i] = malloc(sizeof(uint64_t) * (1ULL << i));
        mallocCheck(l[i]);
        const uint64_t *prev = l[i - 1];
        uint64_t *current = l[i];
        for (size_t j = 0; j < g; j++) {
            current[j] = 2 * prev[j];
            current[g + j] = 1 + current[j];
        }
    }
    return l;
}

// (a + b) mod 2^nprime + 1
bigInt *addModNPrime(const bigInt *a, const bigInt *b, const size_t nprime) {
    bigInt *tmp = add(a, b);
    bigInt *res = reduceModNPrime(tmp, nprime);
    freeBigInt(tmp);
    return res;
}

// (a - b) mod 2^nprime + 1
bigInt *subModNPrime(const bigInt *a, const bigInt *b, const size_t nprime) {
    bigInt *res;
    if (compareBigInt(a, b) >= 0) {
        // a >= b, normal subtraction
        bigInt *tmp = sub(a, b);
        res = reduceModNPrime(tmp, nprime);
        freeBigInt(tmp);
    } else {
        // a < b, compute (2^nprime + 1) - (b - a)
        bigInt *diff = sub(b, a);
        // TODO Optimize
        bigInt *one = getBigIntFromUnsignedInteger(1);
        bigInt *tmpFn = shiftLeft(one, nprime);
        bigInt *Fn = add(tmpFn, one);
        freeBigInt(tmpFn);
        freeBigInt(one);

        res = sub(Fn, diff); // Guaranteed strictly positive
        freeBigInt(diff);
        freeBigInt(Fn);
    }

    return res;
}

bigInt **decompose(const bigInt *x, const uint64_t K, const uint64_t nprime, const uint64_t M, const uint64_t Mp) {
    bigInt **res = malloc(sizeof(bigInt *) * K);
    mallocCheck(res);
    for (size_t i = 0; i < K; i++) {
        bigInt *chunk = sliceBigInt(x, i * (M / 64), M / 64);
        bigInt *val = mul2ExpModNPrime(chunk, i * Mp, nprime);
        freeBigInt(chunk);
        res[i] = val;
    }
    return res;
}
