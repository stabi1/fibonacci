#include "bigIntUtil.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "bigIntDiv.h"

const char hexLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

const char decLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

const int DEC_STRING_SMALL_FASTER = 100;

struct schoenhageReturn {
    char *res;
    size_t resLen;
};

size_t decCharToValue(char dec);

uint64_t decString20CharsTo_uint64_t(const char *dexStr, size_t strLen);

void inplaceMulAddForConversion(uint64_t *array, size_t arrayLen, uint64_t z, size_t digestsDone);

void
bigIntToDecStringSchoenhageHelper(bigInt *x, size_t digits, char **resString, size_t *resStringCounter, bool beginning);

void *bigIntToDecStringSchoenhageMultithreadHelper(void *input);

struct schoenhageReturn *bigIntToDecStringSchoenhageLenRet(bigInt *x, size_t digits, bool beginning);

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

// -> max length 2000 petabytes
size_t bitLength(const bigInt *x) {
    size_t xLen = x->end - x->start;
    return xLen * 64 - custom_lzcnt(x->bigIntArray[x->end - 1]);
}

void decStringToBigIntHelper(uint64_t *array, const size_t arrayLen, const char *decStr, const size_t decStrLen) {
    size_t digitsPerLong = 19;
    const char *decStringEnd = decStr + decStrLen;
    size_t digestsDone = 0;

    // Process first (potentially short) digit group
    size_t firstGroupLen = decStrLen % digitsPerLong;
    if (firstGroupLen == 0)
        firstGroupLen = digitsPerLong;
    array[0] = decString20CharsTo_uint64_t(decStr, firstGroupLen);
    decStr += firstGroupLen;
    digestsDone += firstGroupLen;

    // Process remaining digit groups
    uint64_t groupVal = 0;
    while (decStr < decStringEnd) {
        groupVal = decString20CharsTo_uint64_t(decStr, digitsPerLong);
        decStr += digitsPerLong;
        digestsDone += digitsPerLong;
        inplaceMulAddForConversion(array, arrayLen, groupVal, digestsDone);
    }
    // calling function still needs to resize!!!
}

void inplaceMulAddForConversion(uint64_t *array, const size_t arrayLen, uint64_t z, size_t digestsDone) {
    // Perform the multiplication word by word
    size_t num_blocks = (long) (((double) digestsDone * 3.32193f) / 64.0f + 1);
    unsigned __int128 yLong = 10000000000000000000ULL;
    unsigned __int128 zLong = z;

    unsigned __int128 product = 0;
    uint64_t carry = 0;
    size_t boundary = num_blocks + 1 > arrayLen ? arrayLen : num_blocks + 1;

    for (size_t i = 0; i < boundary; i++) {
        product = yLong * (array[i]) + carry;
        array[i] = (uint64_t) product;
        carry = product >> 64;
    }

    // Perform the addition
    unsigned __int128 sum = (array[0]) + zLong;
    array[0] = (uint64_t) sum;
    carry = sum >> 64;
    for (size_t i = 1; i < boundary; i++) {
        sum = (array[i]) + carry;
        array[i] = (uint64_t) sum;
        carry = sum >> 64;
        if (carry == 0) break;
    }
}

uint64_t decString20CharsTo_uint64_t(const char *dexStr, size_t strLen) {
    uint64_t res = decCharToValue(dexStr[0]);
    for (size_t i = 1; i < strLen; i++) {
        res = res * 10 + decCharToValue(dexStr[i]);
    }
    return res;
}

size_t decCharToValue(char dec) {
    if (dec < '0' || dec > '9') {
        fprintf(stderr, "DecString Invalid, contains char %c\n", dec);
        exit(EXIT_FAILURE);
    }
    return dec - '0';
}

char *uint64tArrayToHexString(const uint64_t *array, size_t lenInNibbles, size_t start, bool negative) {
    size_t strLength = sizeof(char) * (lenInNibbles + 1);
    if (negative) strLength++;
    char *str = malloc(strLength);
    mallocCheck(str);
    str[strLength - 1] = '\0';
    const uint8_t *restrict buf = (uint8_t *) array;

    long j = (long) lenInNibbles - 1;
    long loopEnd = 1;
    if (negative) {
        j++; // start one later to make space for negative sign
        loopEnd++; // adjust loop end too
    }
    size_t i = start * 8;
    for (; j >= loopEnd; i++, j -= 2) {
        str[j] = hexLookup[buf[i] & 0xF];
        str[j - 1] = hexLookup[buf[i] >> 4];
    }
    if (j == loopEnd - 1) {
        str[j] = hexLookup[buf[i] & 0xF];
    }

    if (negative) str[0] = '-';
    return str;
}

//Helper for hexStringToBigInt
size_t hexToNibble(char hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    } else {
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

size_t calculateDecStringSpace(const bigInt *x) {
    size_t xLen = getLen(x);
    size_t xMaxBitLen = xLen * 64;
    size_t resMaxLen = (size_t) ((double) xMaxBitLen * 0.30103 + 3);
    return resMaxLen;
}

char *bigIntToDecStringHelper(bigInt *x, bool doFree) {
    bigInt *convertX = doFree ? x : copyBigInt(x);
    if (global_config.parallel)
        return bigIntToDecStringSchoenhageMultithread(convertX);
    else
        return bigIntToDecStringSchoenhage(convertX);
}

char *bigIntToDecStringSchoenhage(bigInt *x) {
    struct schoenhageReturn *ret = bigIntToDecStringSchoenhageLenRet(x, 0, true);
    char *res = ret->res;
    free(ret);
    return res;
}

// FREES the bigInt that is passed!!!
struct schoenhageReturn *bigIntToDecStringSchoenhageLenRet(bigInt *x, size_t digits, bool beginning) {
    size_t resMaxLen = (digits == 0 || beginning) ? calculateDecStringSpace(x) : digits;
    char *res = malloc(resMaxLen);
    mallocCheck(res);
    size_t len = 0;
    bigIntToDecStringSchoenhageHelper(x, digits, &res, &len, beginning);
    res[len] = '\0';
    struct schoenhageReturn *ret = malloc(sizeof(struct schoenhageReturn));
    mallocCheck(ret);
    ret->res = res;
    ret->resLen = len;
    return ret;
}

//shamelessly adapted from Java Jdk8
void bigIntToDecStringSchoenhageHelper(bigInt *x, size_t digits, char **resString, size_t *resStringCounter,
                                       bool beginning) {
    /* If we're smaller than a certain threshold, use the smallToString
       method, padding with leading zeroes when necessary. */
    size_t xLen = getLen(x);
    if (xLen <= DEC_STRING_SMALL_FASTER) {
        char *s = bigIntToDecStringSmall(x);

        // Pad with internal zeros if necessary.
        // Don't pad if we're at the beginning of the string.
        if ((strlen(s) < digits) && !beginning) {
            for (size_t i = strlen(s); i < digits; i++) {
                (*resString)[(*resStringCounter)++] = '0';
            }
        } else if (beginning) { // append negative sign in the front
            if (x->negative) {
                (*resString)[(*resStringCounter)++] = '-';
            }
        }
        freeBigInt(x);
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
        bigInt *vNew = mulSingleThread(v, v);
        freeBigInt(v);
        v = vNew;
    }

    bigInt *r = NULL;
    bigInt *q = divideModSingleThread(x, v, &r, true);
    char *filename_r = storeBigIntInSwap(r);

    size_t expectedDigits = 1 << n;
    // Now recursively build the two halves of each number.
    bigIntToDecStringSchoenhageHelper(q, digits - expectedDigits, resString, resStringCounter,
                                      beginning ? true : false);
    r = loadBigIntFromSwap(r, filename_r);
    bigIntToDecStringSchoenhageHelper(r, expectedDigits, resString, resStringCounter, false);
}

struct schoenhageArgs {
    bigInt *x;
    size_t digits;
    bool beginning;
    size_t depth;
};

char *bigIntToDecStringSchoenhageMultithread(bigInt *x) {
    struct schoenhageArgs *args = malloc(sizeof(struct schoenhageArgs));
    mallocCheck(args);
    args->x = x;
    args->digits = 0;
    args->beginning = true;
    args->depth = global_config.convertDepth;
    struct schoenhageReturn *res = bigIntToDecStringSchoenhageMultithreadHelper(args);
    free(args);
    char *ret = res->res;
    free(res);
    return ret;
}

void *bigIntToDecStringSchoenhageMultithreadHelper(void *input) {
    bigInt *x = ((struct schoenhageArgs *) input)->x;
    size_t digits = ((struct schoenhageArgs *) input)->digits;
    bool beginning = ((struct schoenhageArgs *) input)->beginning;
    size_t depth = ((struct schoenhageArgs *) input)->depth;

    size_t xLen = getLen(x);
    if (global_config.verbose) {
        char *localTime = getCurrentDateTime();
        printf("Convert-Depth: %lu;  %s\n", depth, localTime);
        free(localTime);
    }
    if (depth == 0 || xLen < DEC_STRING_SMALL_FASTER) {
        struct schoenhageReturn *res = bigIntToDecStringSchoenhageLenRet(x, digits, beginning);
        return res;
    }

    size_t b, n;
    b = bitLength(x);
    n = (size_t) llroundl(log((double) b * log(2.0) / log(10.0)) / log(2.0) - 1.0);
    bigInt *v = getBigIntFromUnsignedInteger(10);
    for (size_t i = 0; i < n; i++) {
        bigInt *vNew;
        if (depth == global_config.convertDepth) {
            vNew = mulParallel(v, v, global_config.mulDepth);
        } else {
            size_t parallelMuls = 1 << (global_config.convertDepth - depth);
            size_t mulDepth = calcMulDepthForParallelMuls(parallelMuls);
            vNew = mulParallel(v, v, mulDepth);
        }
        freeBigInt(v);
        v = vNew;
    }
    if (global_config.verbose && depth == global_config.convertDepth) {
        char *localTime = getCurrentDateTime();
        printf("First muls from conversion finished  %s\n", localTime);
        free(localTime);
    }

    bigInt *r = NULL;
    bigInt *q;
    if (depth == global_config.convertDepth) {
        q = divideModMultiThread(x, v, &r, global_config.mulDepth, true);
        if (global_config.verbose) {
            char *localTime = getCurrentDateTime();
            printf("First div from conversion finished;  %s\n", localTime);
            free(localTime);
        }
    } else {
        size_t parallelMuls = 1 << (global_config.convertDepth - depth);
        size_t mulDepth = calcMulDepthForParallelMuls(parallelMuls);
        q = divideModMultiThread(x, v, &r, mulDepth, true);
    }

    size_t expectedDigits = 1 << n;

    // Now recursively build the two halves of each number.
    pthread_t thread_idConvert1;
    pthread_t thread_idConvert2;
    struct schoenhageArgs *args1 = malloc(sizeof(struct schoenhageArgs));
    mallocCheck(args1);
    args1->x = q;
    args1->digits = digits - expectedDigits;
    args1->beginning = beginning ? true : false;
    args1->depth = depth - 1;
    struct schoenhageArgs *args2 = malloc(sizeof(struct schoenhageArgs));
    mallocCheck(args2);
    args2->x = r;
    args2->digits = expectedDigits;
    args2->beginning = false;
    args2->depth = depth - 1;
    if (pthread_create(&thread_idConvert1, NULL, bigIntToDecStringSchoenhageMultithreadHelper, (void *) args1) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread_idConvert2, NULL, bigIntToDecStringSchoenhageMultithreadHelper, (void *) args2) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }
    void *tmp1;
    void *tmp2;
    if (pthread_join(thread_idConvert1, &tmp1) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(thread_idConvert2, &tmp2) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    struct schoenhageReturn *res1 = tmp1;
    struct schoenhageReturn *res2 = tmp2;
    free(args1);
    free(args2);

    char *result = realloc(res1->res, res1->resLen + res2->resLen + 1);
    mallocCheck(result);
    memcpy(result + res1->resLen, res2->res, res2->resLen);
    result[res1->resLen + res2->resLen] = '\0';
    free(res2->res);
    struct schoenhageReturn *ret = malloc(sizeof(struct schoenhageReturn));
    mallocCheck(ret);
    ret->res = result;
    ret->resLen = res1->resLen + res2->resLen;
    free(res1);
    free(res2);

    if (global_config.verbose) {
        char *localTime = getCurrentDateTime();
        printf("Thread with Depth: %lu finished;  %s\n", depth, localTime);
        free(localTime);
    }
    return ret;
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
    d->bigIntArray[0] = 0x8AC7230489E80000; //10^digitsPerUInt64 = 8AC7230489E80000
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

    // Put sign (if any) and first digit group into result buffer
    char *res = malloc(numGroups * digitsPerUInt64 + 2);
    mallocCheck(res);
    size_t resCounter = 0;
    memcpy(res + resCounter, digitGroup[numGroups - 1], strlen(digitGroup[numGroups - 1]));
    resCounter += strlen(digitGroup[numGroups - 1]);
    free(digitGroup[numGroups - 1]);
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
        free(digitGroup[i]);
    }
    res[resCounter] = '\0';
    free(digitGroup);
    return res;
}
