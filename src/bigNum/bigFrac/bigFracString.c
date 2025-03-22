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

bigFrac *hexStringToBigFrac(const char *decStr) {
    size_t hexStrLength = strlen(decStr);

    if (hexStrLength == 0) {
        fprintf(stderr, "hexStr can not be of length 0\n");
        exit(EXIT_FAILURE);
    }

    char *ptr = strchr(decStr, '.');
    if (ptr == NULL) { // only integer, no fraction part
        return newBigFracFromBigInt(decStringToBigInt(decStr), false);
    }

    size_t pointIndex = ptr - decStr;
    if (pointIndex == 0) {
        fprintf(stderr, "hexStr can not begin with .\n");
        exit(EXIT_FAILURE);
    } else if (ptr[1] == '\0') {
        fprintf(stderr, "hexStr can not end with .\n");
        exit(EXIT_FAILURE);
    }

    bigInt *holePart = hexStringToBigIntLength(decStr, pointIndex, false);

    bigInt *fractionPart = hexStringToBigIntLength(ptr + 1, strlen(ptr + 1), true);
    size_t nibbles = strlen(ptr + 1);
    size_t nibblesToShift = (16 - (nibbles % 16)) % 16;
    bigInt *fractionPartShifted = shiftLeft(fractionPart, nibblesToShift * 4);
    freeBigInt(fractionPart);

    bigInt *resBigInt = newBigIntNotZeroed(getLen(holePart) + getLen(fractionPartShifted));

    memcpy(resBigInt->bigIntArray, fractionPartShifted->bigIntArray + fractionPartShifted->start, getLen(fractionPartShifted) * 8);
    memcpy((resBigInt->bigIntArray + getLen(fractionPartShifted)), holePart->bigIntArray + holePart->start, getLen(holePart) * 8);
    resBigInt->negative = holePart->negative;
    freeBigInt(holePart);

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBlocks = getLen(fractionPartShifted);
    freeBigInt(fractionPartShifted);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}

char *bigFracToHexString(const bigFrac *x) {
    if (isZero(x->bigIntPart)) {
        return getZeroDotZeroString();
    }
    size_t fractionBlocks = x->fractionBlocks;

    x->bigIntPart->start += fractionBlocks;
    char *holePart;
    if (getLen(x->bigIntPart) == 0) {
        if (x->bigIntPart->negative) {
            holePart = malloc(3);
            mallocCheck(holePart);
            holePart[0] = '-';
            holePart[1] = '0';
            holePart[2] = '\0';
        } else {
            holePart = getZeroString();
        }
    } else {
        holePart = bigIntToHexString(x->bigIntPart, false);
    }

    x->bigIntPart->start -= fractionBlocks;
    size_t originalEnd = x->bigIntPart->end;
    x->bigIntPart->end = x->bigIntPart->start + fractionBlocks;
    char *fractionPart;
    if (getLen(x->bigIntPart) == 0) {
        fractionPart = getZeroString();
    } else {
        bool sign = x->bigIntPart->negative;
        x->bigIntPart->negative = false;
        fractionPart = bigIntToHexString(x->bigIntPart, true);
        x->bigIntPart->negative = sign;
    }
    x->bigIntPart->end = originalEnd;

    size_t holePartLength = strlen(holePart);
    size_t fractionPartLength = strlen(fractionPart);
    char *finalString = malloc(holePartLength + fractionPartLength + 2); // for '.' and '\0'

    memcpy(finalString, holePart, holePartLength);
    free(holePart);
    finalString[holePartLength] = '.';
    memcpy(finalString + holePartLength + 1, fractionPart, fractionPartLength);
    free(fractionPart);

    //remove trailing Zeros
    size_t end = holePartLength + fractionPartLength;
    while (finalString[end] == '0') {
        end--;
    }
    finalString[end + 1] = '\0';
    return finalString;
}

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

    size_t wantedBinaryDigits = automaticPrecision ? (size_t) ceil((double) (decStrLength - pointIndex - 1) * log2(10)) : binaryDigits;

    bigInt *holePart = decStringToBigIntLength(decStr, pointIndex, false);

    bigFrac *fractionPart = fractionDecStringToBigFrac(ptr + 1, wantedBinaryDigits);
    bigInt *resBigInt = newBigIntNotZeroed(getLen(holePart) + getLen(fractionPart->bigIntPart));
    memcpy(resBigInt->bigIntArray, fractionPart->bigIntPart->bigIntArray + fractionPart->bigIntPart->start, getLen(fractionPart->bigIntPart) * 8);
    memcpy((resBigInt->bigIntArray + getLen(fractionPart->bigIntPart)), holePart->bigIntArray + holePart->start, getLen(holePart) * 8);
    resBigInt->negative = holePart->negative;
    freeBigInt(holePart);

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBlocks = fractionPart->fractionBlocks;
    freeBigFrac(fractionPart);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}

bigFrac *fractionDecStringToBigFrac(const char *decStrFraction, const size_t wantedBinaryDigits) {
    size_t wantedPrecisionInBlocks = wantedBinaryDigits % 64 == 0 ? wantedBinaryDigits / 64 : wantedBinaryDigits / 64 + 1;

    bigInt *resBigInt = newBigIntNotZeroed(wantedPrecisionInBlocks);

    // Build numerator from the decimal string (using your decStringToBigIntHelper)
    bigInt *numerator = decStringToBigIntLength(decStrFraction, strlen(decStrFraction), true);

    // Build denominator = 10^decStrLen as a big integer.
    bigInt *ten = getBigIntFromUnsignedInteger(10);
    bigInt *denominator = powBigInt(ten, strlen(decStrFraction));
    freeBigInt(ten);

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
        freeBigInt(quotient);

        // Update numerator to be the remainder for next iteration.
        freeBigInt(numerator);
        numerator = reminder;
        if (isZero(numerator)) {
            blockDone++;
            break;
        }
    }
    freeBigInt(numerator);
    freeBigInt(denominator);

    resBigInt->end = blockDone;
    size_t n = wantedBinaryDigits % 64;
    if (n != 0) {
        uint64_t mask = ~((uint64_t) 0xFFFFFFFFFFFFFFFF >> n);
        resBigInt->bigIntArray[resBigInt->start] &= mask;
    }

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBlocks = blockDone;
    return res;
}

char *bigFracToDecString(const bigFrac *x, bool exactPrecision) {
    if (isZero(x->bigIntPart)) {
        return getZeroDotZeroString();
    }
    size_t fractionBlocks = x->fractionBlocks;

    x->bigIntPart->start += fractionBlocks;
    char *holePart;
    if (getLen(x->bigIntPart) == 0) {
        if (x->bigIntPart->negative) {
            holePart = malloc(3);
            mallocCheck(holePart);
            holePart[0] = '-';
            holePart[1] = '0';
            holePart[2] = '\0';
        } else {
            holePart = getZeroString();
        }
    } else {
        holePart = bigIntToDecString(x->bigIntPart, false);
    }

    x->bigIntPart->start -= fractionBlocks;
    size_t originalEnd = x->bigIntPart->end;
    x->bigIntPart->end = x->bigIntPart->start + fractionBlocks;
    char *fractionPart;
    if (getLen(x->bigIntPart) == 0) {
        fractionPart = getZeroString();
    } else {
        fractionPart = bigFracToDecStringFractionPart(x, exactPrecision);
    }
    x->bigIntPart->end = originalEnd;

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


char *bigFracToDecStringFractionPart(const bigFrac *x, bool exactPrecision) {
    bigInt *bigIntPart = x->bigIntPart;

    size_t xLen = getLen(bigIntPart);
    if (isZero(bigIntPart)) {
        return getZeroString();
    }

    size_t realFractionBits = xLen * 64 - custom_tzcnt(bigIntPart->bigIntArray[bigIntPart->end - 1]);
    size_t decimalDigits = exactPrecision ? realFractionBits : (size_t) ((double) realFractionBits * log10(2));
    decimalDigits = decimalDigits == 0 ? 1 : decimalDigits;

    bigFrac *tmp = newBigFracFromBigInt(bigIntPart, true);
    tmp->bigIntPart->negative = false;
    tmp->fractionBlocks = getLen(tmp->bigIntPart);
    bigInt *ten = getBigIntFromUnsignedInteger(10);
    bigInt *dInt = powBigInt(ten, decimalDigits); // 10^decimalDigits
    freeBigInt(ten);
    bigFrac *d = newBigFracFromBigInt(dInt, false);
    bigFrac *mulRes = mulBigFracNoResize(tmp, d);
    freeBigFrac(tmp);
    freeBigFrac(d);

    mulRes->bigIntPart->start += mulRes->fractionBlocks;
    char *res = bigIntToDecString(mulRes->bigIntPart, true);

    // bigIntToDecString does not do leading zeros, add in front if missing
    size_t resLen = strlen(res);
    if (resLen < decimalDigits) {
        size_t missingDigits = decimalDigits - resLen;
        char *tmpRes = malloc(decimalDigits + 1);
        mallocCheck(tmpRes);
        memset(tmpRes, '0', missingDigits);
        memcpy(tmpRes + missingDigits, res, resLen);
        tmpRes[decimalDigits] = '\0';
        free(res);
        res = tmpRes;
    }

    free(mulRes);
    return res;
}
