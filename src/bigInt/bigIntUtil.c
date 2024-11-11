#include "bigIntUtil.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

const char hexLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

const char decLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

const int DECSTRINGSMALLFASTER = 10;

void mallocCheck(void *p) {
    if (p == NULL) {
        fprintf(stderr, "An error occurred: Malloc returned null. Program terminated\n");
        exit(EXIT_FAILURE);
    }
}

size_t custom_lzcnt(uint64_t n) {
    if (n == 0) {
        return 64;
    }
    return __builtin_clzll(n);
}

uint64_t bitLength(bigInt *x) {
    size_t xLen = x->end - x->start;
    return xLen * 64 - custom_lzcnt(x->bigIntArray[x->end - 1]);
}

char *uint64tToHexString(uint64_t *array, size_t lenInNibbles, size_t start, bool negative) {
    size_t strLength = sizeof(char) * (lenInNibbles + 1);
    if (negative) strLength++;
    char *str = malloc(strLength);
    mallocCheck(str);
    str[strLength - 1] = '\0';
    uint8_t *buf = (uint8_t *) array;

    long j = (long) lenInNibbles - 1;
    if (negative) j++;
    size_t i = start * 8;
    for (; j >= 1; i++, j -= 2) {
        str[j] = hexLookup[buf[i] & 0xF];
        str[j - 1] = hexLookup[buf[i] >> 4];
    }
    if (j == 0) {
        str[j] = hexLookup[buf[i] & 0xF];
    }

    if (negative) str[0] = '-';
    return str;
}

//Helper for hexStringToBigInt
size_t hexToNibble(char hex) {
    switch (hex) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'A':
            return 10;
        case 'B':
            return 11;
        case 'C':
            return 12;
        case 'D':
            return 13;
        case 'E':
            return 14;
        case 'F':
            return 15;
        default:
            fprintf(stderr, "HexString Invalid, contains char %c\n", hex);
            exit(EXIT_FAILURE);
    }
}

//returns the decimal representation of the uint64_t
char *uint64_t_toDecString(uint64_t x) {
    size_t bufLen = 20;
    char buf[bufLen];
    size_t charPos = bufLen - 1;
    while (x >= 10) {
        buf[charPos--] = decLookup[x % 10];
        x = x / 10;
    }
    buf[charPos] = decLookup[x];
    char *res = malloc(bufLen - charPos + 1);
    mallocCheck(res);
    memcpy(res, buf + charPos, bufLen - charPos);
    res[bufLen - charPos] = '\0';
    return res;
}

char *bigIntToDecStringHelper(bigInt *x) {
    size_t xLen = x->end - x->start;
    size_t resMaxLen = (size_t) ((double) xLen * 8 * 2 * 1.3);
    char *res = malloc(resMaxLen);
    mallocCheck(res);
    size_t len = 0;
    bigIntToDecStringSchoenhage(x, 0, &res, &len, &resMaxLen);
    res[len] = '\0';
    return res;
}

//shamelessly adapted from Java Jdk8
void
bigIntToDecStringSchoenhage(bigInt *x, size_t digits, char **resString, size_t *resStringCounter, size_t *resMaxLen) {
    /* If we're smaller than a certain threshold, use the smallToString
       method, padding with leading zeroes when necessary. */
    size_t xLen = x->end - x->start;
    if (xLen <= DECSTRINGSMALLFASTER) {
        char *s = bigIntToDecStringSmall(x);

        // Pad with internal zeros if necessary.
        // Don't pad if we're at the beginning of the string.
        if ((strlen(s) < digits) && (*resStringCounter > 0)) {
            for (size_t i = strlen(s); i < digits; i++) {
                if (*resStringCounter >= *resMaxLen) {
                    *resString = realloc(*resString, (size_t) ((double) *resMaxLen * 1.5));
                    *resMaxLen = (size_t) ((double) *resMaxLen * 1.5);
                    mallocCheck(*resString);
                }
                (*resString)[(*resStringCounter)++] = '0';
            }
        }
        if (*resStringCounter + strlen(s) >= *resMaxLen) {
            *resString = realloc(*resString, (size_t) ((double) *resMaxLen * 1.5));
            *resMaxLen = (size_t) ((double) *resMaxLen * 1.5);
            mallocCheck(*resString);
        }
        memcpy(*resString + *resStringCounter, s, strlen(s));
        *resStringCounter += strlen(s);
        free(s);
        return;
    }

    size_t b, n;
    b = bitLength(x);
    n = (size_t) llroundl(log((double) b * log(2.0) / log(10.0)) / log(2.0) - 1.0);
    bigInt *v = newBigInt(1);
    v->bigIntArray[v->start] = 10;
    for (size_t i = 0; i < n; i++) {
        bigInt *vNew = multiplyToomCook3(v, v);
        freeBigInt(v);
        v = vNew;
    }

    bigInt *r = NULL;
    bigInt *q = divideMod(x, v, &r);

    int expectedDigits = 1 << n;

    // Now recursively build the two halves of each number.
    bigIntToDecStringSchoenhage(q, digits - expectedDigits, resString, resStringCounter, resMaxLen);
    bigIntToDecStringSchoenhage(r, expectedDigits, resString, resStringCounter, resMaxLen);
    freeBigInt(r);
    freeBigInt(q);
    freeBigInt(v);
}

//shamelessly adapted from Java Jdk8
char *bigIntToDecStringSmall(bigInt *x) {
    const size_t digitsPerUInt64 = 19;
    const char *zeros = "00000000000000000000";

    size_t xLen = x->end - x->start;
    if (xLen == 1 && x->bigIntArray[0] == 0) {
        char *zero = malloc(2);
        mallocCheck(zero);
        zero[0] = '0';
        zero[1] = '\0';
        return zero;
    }

    // Compute upper bound on number of digit groups and allocate space
    //size_t maxNumDigitGroups = (4 * xLen + 6) / 7;
    size_t maxNumDigitGroups = (6 * xLen + 8) / 5;
    char **digitGroup = malloc(sizeof(char **) * maxNumDigitGroups);
    mallocCheck(digitGroup);

    // Translate number to string, a digit group at a time
    int numGroups = 0;
    bigInt *tmp = copyBigInt(x);
    bigInt *d = newBigInt(1);
    d->bigIntArray[0] = 0x8AC7230489E80000; //10^digitsPerUInt64= 8AC7230489E80000
    while (!(tmp->end - tmp->start == 1 && tmp->bigIntArray[0] == 0)) {
        bigInt *r = NULL;
        bigInt *q = divideMod(tmp, d, &r);
        digitGroup[numGroups++] = uint64_t_toDecString(r->bigIntArray[0]);
        freeBigInt(tmp);
        freeBigInt(r);
        tmp = q;
    }
    freeBigInt(tmp);
    freeBigInt(d);

    // Put sign (if any) and first digit group into result buffer
    char *res = malloc(numGroups * digitsPerUInt64 + 2);
    size_t resCounter = 0;
    if (x->negative) {
        res[resCounter++] = '-';
    }
    memcpy(res + resCounter, digitGroup[numGroups - 1], strlen(digitGroup[numGroups - 1]));
    resCounter += strlen(digitGroup[numGroups - 1]);
    // Append remaining digit groups padded with leading zeros
    for (int i = numGroups - 2; i >= 0; i--) {
        // Prepend (any) leading zeros for this digit group
        size_t numLeadingZeros = digitsPerUInt64 - strlen(digitGroup[i]);
        if (numLeadingZeros != 0) {
            memcpy(res + resCounter, zeros, numLeadingZeros);
            resCounter += numLeadingZeros;
        }
        memcpy(res + resCounter, digitGroup[i], strlen(digitGroup[i]));
        resCounter += strlen(digitGroup[i]);
    }
    res[resCounter] = '\0';
    for (int i = 0; i < numGroups; i++) {
        free(digitGroup[i]);
    }
    free(digitGroup);
    return res;
}
