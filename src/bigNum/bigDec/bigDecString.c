#include "string.h"

#include "bigDecString.h"
#include "bigDec.h"
#include "../constants.h"


char *bigDecToDecString(__attribute__((unused)) bigDec *x) {
    return NULL;
}

char *uint64_t_FractionToDecString(uint64_t fraction) {
    if (fraction == 0) {
        char *res = malloc(2);
        res[0] = '0';
        res[1] = '\0';
        return res;
    }
    size_t wantedDigits = 64;
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

