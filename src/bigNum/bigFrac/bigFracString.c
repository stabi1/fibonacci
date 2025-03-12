#include "string.h"

#include "bigFracString.h"
#include "bigFrac.h"
#include "bigFracMul.h"
#include "../constants.h"
#include "../misc.h"
#include "../bigInt/bigIntUtil.h"

#include <math.h>
#include <stdio.h>


char *bigDecToDecString(__attribute__((unused)) bigFrac *x) {
    return NULL;
}

char *bigDecToDecStringSmall(bigFrac *x, bool exactPrecision) {
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

    size_t realFractionBits = xLen * 64 - custom_tzcnt(bigIntPart->bigIntArray[bigIntPart->end - 1]);
    double log10_2 = log(2) / log(10);
    size_t decimalDigits = exactPrecision ? realFractionBits : (size_t) ((double) realFractionBits * log10_2);

    char **digitGroup = malloc(sizeof(char **) * ((decimalDigits / DEC_DIGITS_PER_UINT64) + 3));
    mallocCheck(digitGroup);

    // Translate number to string, a digit group at a time
    size_t numGroups = 0;
    bigFrac *tmp = newBigFracFromBigInt(bigIntPart, true);
    tmp->fractionBits = getLen(tmp->bigIntPart) * 64;
    bigFrac *d = getBigFracFromUnsignedInteger(0x8AC7230489E80000); // 10^DEC_DIGITS_PER_UINT64 = 8AC7230489E80000
    for (size_t i = 0; i < (decimalDigits / DEC_DIGITS_PER_UINT64); i++) {
        if(isZero(tmp->bigIntPart)) {break;}
        bigFrac *mulRes = mulBigFracNoResize(tmp, d);
        if (getLen(tmp->bigIntPart) == getLen(mulRes->bigIntPart)) {
            char *zero = malloc(2);
            mallocCheck(zero);
            zero[0] = '0';
            zero[1] = '\0';
            digitGroup[numGroups++] = zero;
        } else {
            digitGroup[numGroups++] = uint64_t_toDecString(mulRes->bigIntPart->bigIntArray[mulRes->bigIntPart->end - 1]);
            mulRes->bigIntPart->end -= 1;
        }
        if (getLen(mulRes->bigIntPart) > 1 && mulRes->bigIntPart->bigIntArray[mulRes->bigIntPart->start] == 0) {
            mulRes->bigIntPart->start++;
            mulRes->fractionBits -= 64;
        }
        freeBigFrac(tmp);
        tmp = mulRes;
    }
    if (decimalDigits % DEC_DIGITS_PER_UINT64 != 0 || !isZero(tmp->bigIntPart)) {
        freeBigFrac(d);
        d = getBigFracFromUnsignedInteger(10);
        char *buf = malloc(DEC_DIGITS_PER_UINT64 + 1);
        mallocCheck(buf);
        size_t j = 0;
        for (; j < (decimalDigits % DEC_DIGITS_PER_UINT64); j++) {
            if(isZero(tmp->bigIntPart)) {break;}
            bigFrac *mulRes = mulBigFracNoResize(tmp, d);

            // Extract the integer part from the upper 64 bits.
            int digit = 0; // not if -> was already resized by mul because digit is zero
            if (getLen(tmp->bigIntPart) != getLen(mulRes->bigIntPart)) {
                digit = (int) (mulRes->bigIntPart->bigIntArray[mulRes->bigIntPart->end - 1]);
                // Remove the integer part leaving the fractional remainder.
                mulRes->bigIntPart->end -= 1;
            }
            buf[j] = decLookup[digit];
            if (getLen(mulRes->bigIntPart) > 1 && mulRes->bigIntPart->bigIntArray[mulRes->bigIntPart->start] == 0) {
                mulRes->bigIntPart->start++;
                mulRes->fractionBits -= 64;
            }
            tmp = mulRes;
        }
        buf[j] = '\0';
        digitGroup[numGroups++] = buf;
    }
    freeBigFrac(tmp);
    freeBigFrac(d);

    char *res = malloc(numGroups * DEC_DIGITS_PER_UINT64 + 2);
    mallocCheck(res);
    size_t resCounter = 0;
    // Append remaining digit groups padded with leading zeros
    for (size_t i = 0; i < numGroups - 1; i++) {
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
    memcpy(res + resCounter, digitGroup[numGroups - 1], strlen(digitGroup[numGroups - 1]));
    resCounter += strlen(digitGroup[numGroups - 1]);
    free(digitGroup[numGroups - 1]);

    res[resCounter] = '\0';
    free(digitGroup);
    return res;
}
