#include "string.h"

#include "bigFracString.h"
#include "bigFrac.h"
#include "../constants.h"


char *bigDecToDecString(__attribute__((unused)) bigFrac *x) {
    return NULL;
}

char *uint64_t_FractionToDecString(uint64_t fraction) {
    if (fraction == 0) {
        char *res = malloc(2);
        res[0] = '0';
        res[1] = '\0';
        return res;
    }
    size_t wantedDigits = 20;
    size_t bufLen = wantedDigits + 1;
    char buf[bufLen];

    uint128_t value = fraction;
    // 2^64 as a mask for the fractional part
    uint128_t mask = (((uint128_t)1 << 64) - 1);

    // max of 19 chars out of a 64bit number
    for (size_t i = 0; i < wantedDigits; i++) {
        value *= 10;
        // Extract the integer part from the upper 64 bits.
        int digit = (int)(value >> 64);
        // Remove the integer part leaving the fractional remainder.
        value &= mask;
        buf[i] = decLookup[digit];
    }

    // Trim trailing zeros.
    size_t end = wantedDigits - 1;
    while (end > 0 && buf[end] == '0') {
        end--;
    }
    buf[end + 1] = '\0';

    size_t strLen = end + 1;
    char *res = malloc(strLen + 1);
    mallocCheck(res);
    memcpy(res, buf, strLen);
    res[strLen] = '\0';
    return res;
}

/*char *bigDecToDecStringSmall(bigFrac *x) {
    const char *zeros = "00000000000000000000";

    bigInt *bigIntPart = x->bigIntPart;

    size_t xLen = getLen(bigIntPart);
    if (isZero(bigIntPart)) {
        char *zero = malloc(2);
        mallocCheck(zero);
        zero[0] = '0';
        zero[1] = '\0';
        return zero;
    }

    // Compute upper bound on number of digit groups and allocate space
    // size_t maxNumDigitGroups = (4 * xLen + 6) / 7;
    size_t maxNumDigitGroups = (6 * xLen + 8) / 5;
    char **digitGroup = malloc(sizeof(char **) * maxNumDigitGroups);
    mallocCheck(digitGroup);

    // Translate number to string, a digit group at a time
    int numGroups = 0;
    bigInt *tmp = copyBigInt(bigIntPart);
    bigInt *d = newBigInt(1);
    d->bigIntArray[0] = 0x8AC7230489E80000; //10^DEC_DIGITS_PER_UINT64 = 8AC7230489E80000
    while (!(tmp->end - tmp->start == 1 && tmp->bigIntArray[0] == 0)) {
        bigInt *r = NULL;
        bigInt *q = divideModSingleThread(tmp, d, &r, false);
        freeBigInt(tmp);
        digitGroup[numGroups++] = uint64_t_toDecString(r->bigIntArray[0]);
        freeBigInt(r);
        tmp = q;
    }
    freeBigInt(tmp);
    freeBigInt(d);

    char *res = malloc(numGroups * DEC_DIGITS_PER_UINT64 + 2);
    mallocCheck(res);
    size_t resCounter = 0;
    memcpy(res + resCounter, digitGroup[numGroups - 1], strlen(digitGroup[numGroups - 1]));
    resCounter += strlen(digitGroup[numGroups - 1]);
    free(digitGroup[numGroups - 1]);
    // Append remaining digit groups padded with leading zeros
    for (int i = numGroups - 2; i >= 0; i--) {
        // Prepend (any) leading zeros for this digit group
        size_t numLeadingZeros = DEC_DIGITS_PER_UINT64 - strlen(digitGroup[i]);
        if (numLeadingZeros != 0) {
            memcpy(res + resCounter, zeros, numLeadingZeros);
            resCounter += numLeadingZeros;
        }
        memcpy(res + resCounter, digitGroup[i], strlen(digitGroup[i]));
        resCounter += strlen(digitGroup[i]);
        free(digitGroup[i]);
    }
    res[resCounter] = '\0';
    free(digitGroup);
    return res;
}*/

