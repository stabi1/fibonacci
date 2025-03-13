#include "string.h"

#include "bigFracString.h"
#include "bigFrac.h"
#include "bigFracMul.h"
#include "bigFracMethods.h"
#include "../constants.h"
#include "../misc.h"
#include "../bigInt/bigIntString.h"

#include <math.h>
#include <stdio.h>


bigFrac *decStringToBigFrac(const char *decStr, bool automaticPrecision, const size_t binaryDigits) {
    size_t decStrLength = strlen(decStr);

    if (decStrLength == 0) {
        fprintf(stderr, "decStr can not be of length 0\n");
        exit(EXIT_FAILURE);
    }

    char *ptr = strchr(decStr, '.');
    if (ptr == NULL) { // only integer, no fraction part
        return newBigFracFromBigInt(decStringToBigInt(decStr), false);
    }

    size_t pointIndex = ptr - decStr;
    if (pointIndex == 0) {
        fprintf(stderr, "decStr can not begin with .\n");
        exit(EXIT_FAILURE);
    } else if (ptr[1] == '\0') {
        fprintf(stderr, "decStr can not end with .\n");
        exit(EXIT_FAILURE);
    }

    size_t wantedBinaryDigits = automaticPrecision ? (size_t) ceil((double) (decStrLength - pointIndex - 1) * (log(10) / log(2))) : binaryDigits;

    bigInt *holePart = decStringToBigIntLength(decStr, pointIndex);

    bigFrac *fractionPart = fractionDecStringToBigFrac(ptr + 1, wantedBinaryDigits);
    bigInt *resBigInt = newBigIntNotZeroed(getLen(holePart) + getLen(fractionPart->bigIntPart));
    memcpy(resBigInt->bigIntArray, fractionPart->bigIntPart->bigIntArray + fractionPart->bigIntPart->start, getLen(fractionPart->bigIntPart) * 8);
    memcpy((resBigInt->bigIntArray + getLen(fractionPart->bigIntPart)), holePart->bigIntArray + holePart->start, getLen(holePart));
    resBigInt->negative = holePart->negative;

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBits = fractionPart->fractionBits;
    return res;
}

bigFrac *fractionDecStringToBigFrac(const char *decStrFraction, const size_t wantedBinaryDigits) {
    size_t wantedPrecisionInBlocks = wantedBinaryDigits % 64 == 0 ? wantedBinaryDigits / 64 : wantedBinaryDigits / 64 + 1;

    bigInt *resBigInt = newBigIntNotZeroed(wantedPrecisionInBlocks);

    // Build numerator from the decimal string (using your decStringToBigIntHelper)
    bigInt *numerator = decStringToBigInt(decStrFraction);

    // Build denominator = 10^decStrLen as a big integer.
    bigInt *denominator = getBigIntFromUnsignedInteger(1);
    bigInt *ten = getBigIntFromUnsignedInteger(10);
    // Set denominator to 1, then multiply by 10 decStrLen times.
    for (size_t i = 0; i < strlen(decStrFraction); i++) {
        // Multiply denominator by 10
        bigInt *tmp = mul(denominator, ten);
        freeBigInt(denominator);
        denominator = tmp;
    }

    size_t blockDone = 0;
    for (; blockDone < wantedPrecisionInBlocks; blockDone++) {
        // Multiply numerator by 2^(BLOCK_BITS)
        bigInt *tmp = shiftLeft(numerator, 64);
        freeBigInt(numerator);
        numerator = tmp;

        // Divide numerator by denominator: numerator = (quotient, remainder)
        bigInt *reminder;
        bigInt *quotient = divideMod(numerator, denominator, &reminder);

        // Store the 64-bit block of binary digits
        resBigInt->bigIntArray[resBigInt->end - blockDone - 1] = quotient->bigIntArray[0];

        // Update numerator to be the remainder for next iteration.

        freeBigInt(numerator);
        numerator = reminder;
        if (isZero(numerator)) {
            blockDone++;
            break;
        }
    }

    resBigInt->end = blockDone;
    size_t n = wantedBinaryDigits % 64;
    if (n != 0) {
        uint64_t mask = ~((uint64_t) 0xFFFFFFFFFFFFFFFF >> n);
        resBigInt->bigIntArray[resBigInt->end - 1] &= mask;
    }

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBits = blockDone * 64;
    return res;
}


char *bigDecToDecString(const bigFrac *x, bool exactPrecision) {
    bigFrac *aligned = alignToBlock(x);
    if (isZero(aligned->bigIntPart)) {
        char *zero = malloc(4);
        mallocCheck(zero);
        zero[0] = '0';
        zero[1] = '.';
        zero[2] = '0';
        zero[3] = '\0';
        return zero;
    }
    size_t fractionBlocks = aligned->fractionBits / 64;
    size_t originalStart = aligned->bigIntPart->start;

    aligned->bigIntPart->start += fractionBlocks;
    char *holePart;
    if (getLen(aligned->bigIntPart) == 0) {
        if (aligned->bigIntPart->negative) {
            holePart = malloc(3);
            mallocCheck(holePart);
            holePart[0] = '-';
            holePart[1] = '0';
            holePart[2] = '\0';
        } else {
            holePart = getZeroString();
        }
    } else {
        holePart = bigIntToDecString(aligned->bigIntPart, false);
    }

    aligned->bigIntPart->start = originalStart;
    aligned->bigIntPart->end = aligned->bigIntPart->start + fractionBlocks;
    char *fractionPart;
    if (getLen(aligned->bigIntPart) == 0) {
        fractionPart = getZeroString();
    } else {
        fractionPart = bigDecToDecStringFractionPart(aligned, exactPrecision);
    }

    size_t holePartLength = strlen(holePart);
    size_t fractionPartLength = strlen(fractionPart);
    char *finalString = malloc(holePartLength + fractionPartLength + 2); // for '.' and '\0'

    memcpy(finalString, holePart, holePartLength);
    free(holePart);
    finalString[holePartLength] = '.';
    memcpy(finalString + holePartLength + 1, fractionPart, fractionPartLength);
    free(fractionPart);
    finalString[holePartLength + fractionPartLength + 1] = '\0';
    return finalString;
}

char *bigDecToDecStringFractionPart(bigFrac *x, bool exactPrecision) {
    const char *zeros = "00000000000000000000";

    bigInt *bigIntPart = x->bigIntPart;

    size_t xLen = getLen(bigIntPart);
    if (isZero(bigIntPart)) {
        return getZeroString();
    }

    size_t realFractionBits = xLen * 64 - custom_tzcnt(bigIntPart->bigIntArray[bigIntPart->end - 1]);
    double log10_2 = log(2) / log(10);
    size_t decimalDigits = exactPrecision ? realFractionBits : (size_t) ((double) realFractionBits * log10_2);
    decimalDigits = decimalDigits == 0 ? 1 : decimalDigits;

    char **digitGroup = malloc(sizeof(char **) * ((decimalDigits / DEC_DIGITS_PER_UINT64) + 3));
    mallocCheck(digitGroup);

    // Translate number to string, a digit group at a time
    size_t numGroups = 0;
    bigFrac *tmp = newBigFracFromBigInt(bigIntPart, true);
    tmp->fractionBits = getLen(tmp->bigIntPart) * 64;
    bigFrac *d = getBigFracFromUnsignedInteger(0x8AC7230489E80000); // 10^DEC_DIGITS_PER_UINT64 = 8AC7230489E80000
    for (size_t i = 0; i < (decimalDigits / DEC_DIGITS_PER_UINT64); i++) {
        if (isZero(tmp->bigIntPart)) { break; }
        bigFrac *mulRes = mulBigFracNoResize(tmp, d);
        if (getLen(tmp->bigIntPart) == getLen(mulRes->bigIntPart)) {
            digitGroup[numGroups++] = getZeroString();
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
            if (isZero(tmp->bigIntPart)) { break; }
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
