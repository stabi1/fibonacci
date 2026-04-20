#include "bigIntString.h"

#include "bigIntDiv.h"
#include "../constants.h"
#include "../misc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

const int DEC_STRING_SMALL_FASTER = 100;
const int PARALLEL_DEC_STRING_FASTER = 2000;

struct schoenhageReturn {
    char *res;
    size_t resLen;
};

char *uint64tArrayToBinaryString(const uint64_t *array, size_t lenInBit, size_t start, size_t end, bool negative);

size_t decCharToValue(char dec);

uint64_t decString19CharsTo_uint64_t(const char *dexStr, size_t strLen);

void inplaceMulAddForConversion(uint64_t *array, size_t arrayLen, uint64_t z, size_t digetsDone);

void bigIntToDecStringSchoenhageHelper(bigInt *x, size_t digits, char **resString, size_t *resStringCounter, bool beginning);

void *bigIntToDecStringSchoenhageMultithreadHelper(void *input);

struct schoenhageReturn *bigIntToDecStringSchoenhageLenRet(bigInt *x, size_t digits, bool beginning);

//fills the char array with the binary representation of the bigInt
char *bigIntToBinaryString(const bigInt *x, const bool getLeadingZeros) {
    size_t lzcnt = custom_lzcnt(x->bigIntArray[x->end - 1]);
    size_t xLen = x->end - x->start;
    // error handling
    if (lzcnt == 64 && xLen == 1) {
        return getZeroString();
    } else if (!getLeadingZeros && lzcnt == 64 && xLen > 1) {
        fprintf(stderr, "BigInt not printable, has leading zero block\n");
        exit(EXIT_FAILURE);
    }

    size_t lenInBit = getLeadingZeros ? xLen * 64 : bitLength(x);
    char *resStr = uint64tArrayToBinaryString(x->bigIntArray, lenInBit, x->start, x->end, x->negative);
    return resStr;
}

char *uint64tArrayToBinaryString(const uint64_t *array, size_t lenInBit, size_t start, size_t end, bool negative) {
    size_t strLength = sizeof(char) * (lenInBit + 1);
    if (negative) strLength++;
    char *str = malloc(strLength);
    mallocCheck(str);
    str[strLength - 1] = '\0';

    long j = (long) lenInBit - 1; // string counter
    if (negative) {
        j++; // start one later to make space for negative sign
    }
    for (size_t i = start; i < end; i++) {
        for (size_t k = 0; k < 64 && j >= 0; k++) {
            uint64_t mask = 1ULL << k;
            if (array[i] & mask) {
                str[j] = '1';
            } else {
                str[j] = '0';
            }
            j--;
        }
    }
    if (negative) str[0] = '-';
    return str;
}


//fills the char array with the hex representation of the bigInt
char *bigIntToHexString(const bigInt *x, const bool getLeadingZeros) {
    size_t lzcnt = custom_lzcnt(x->bigIntArray[x->end - 1]);
    size_t xLen = x->end - x->start;
    // error handling
    if (lzcnt == 64 && xLen == 1) {
        return getZeroString();
    } else if (!getLeadingZeros && lzcnt == 64 && xLen > 1) {
        fprintf(stderr, "BigInt not printable, has leading zero block\n");
        exit(EXIT_FAILURE);
    }

    size_t lenInNibbles = getLeadingZeros ? xLen * 16 : xLen * 16 - (lzcnt / 4);
    char *resStr = uint64tArrayToHexString(x->bigIntArray, lenInNibbles, x->start, x->negative);
    return resStr;
}

//returns the bigInt of the HexString, hex is being freed
bigInt *hexStringToBigInt(const char *hexStr) {
    return hexStringToBigIntLength(hexStr, strlen(hexStr), false);
}


bigInt *hexStringToBigIntLength(const char *hexStr, const size_t strLen, const bool allowLeadingZeros) {
    size_t hexStrLength = strLen;
    // error handling
    if (hexStrLength == 0) {
        fprintf(stderr, "hexStr can not be of length 0\n");
        exit(EXIT_FAILURE);
    }
    bool negative = false;
    if (hexStr[0] == '-') {
        negative = true;
        hexStrLength--;
        hexStr++;
    } else if (hexStr[0] == '+') {
        hexStrLength--;
        hexStr++;
    }
    if (hexStrLength == 0) {
        fprintf(stderr, "hexStr must contain a number\n");
        exit(EXIT_FAILURE);
    } else if (!allowLeadingZeros && hexStrLength >= 2 && hexStr[0] == '0') {
        fprintf(stderr, "hexStr can not start with 0\n");
        exit(EXIT_FAILURE);
    }

    size_t resLength = hexStrLength / 16;
    if (hexStrLength % 16 != 0) resLength++;
    bigInt *res = newBigIntNotZeroed(resLength);
    res->bigIntArray[res->end - 1] = 0;
    if (negative) res->negative = true;

    size_t j = 0;
    long i = (long) (hexStrLength - 1);
    uint8_t *resByteArray = (uint8_t *) res->bigIntArray;
    for (; i >= 1; i -= 2, j++) {
        uint8_t akt8 = hexToNibble(hexStr[i - 1]);
        akt8 <<= 4;
        akt8 += hexToNibble(hexStr[i]);
        resByteArray[j] = akt8;
    }
    if (i == 0) {
        uint8_t akt8 = hexToNibble(hexStr[i]);
        resByteArray[j] = akt8;
    }
    return res;
}

bigInt *decStringToBigInt(const char *decStr) {
    return decStringToBigIntLength(decStr, strlen(decStr), false);
}

bigInt *decStringToBigIntLength(const char *decStr, const size_t strLen, const bool allowLeadingZeros) {
    size_t decStrLength = strLen;
    if (decStrLength == 0) {
        fprintf(stderr, "decStr can not be of length 0\n");
        exit(EXIT_FAILURE);
    }
    bool negative = false;
    if (decStr[0] == '-') {
        negative = true;
        decStrLength--;
        decStr++;
    } else if (decStr[0] == '+') {
        decStrLength--;
        decStr++;
    }
    if (decStrLength == 0) {
        fprintf(stderr, "decStr must contain a number\n");
        exit(EXIT_FAILURE);
    } else if (!allowLeadingZeros && decStrLength >= 2 && decStr[0] == '0') {
        fprintf(stderr, "decStr can not start with 0\n");
        exit(EXIT_FAILURE);
    }

    double num_blocks = ((double) decStrLength * 3.32193f) / 64.0f + 1;
    size_t resLength = (size_t) num_blocks;
    bigInt *res = newBigInt(resLength);
    if (negative) res->negative = true;

    decStringToBigIntHelper(res->bigIntArray, resLength, decStr, decStrLength);
    //resize if necessary
    size_t newLen = getOccupiedBlocks(res);
    if (newLen != res->end - res->start) {
        res->end = res->start + newLen;
        uint64_t *tmp = realloc(res->bigIntArray, newLen * sizeof(uint64_t));
        mallocCheck(tmp);
        res->bigIntArray = tmp;
        res->completeLength = res->end - res->start;
    }
    return res;
}


// calling function still needs to resize
void decStringToBigIntHelper(uint64_t *array, const size_t arrayLen, const char *decStr, const size_t decStrLen) {
    const char *decStringEnd = decStr + decStrLen;
    size_t digestsDone = 0;

    // Process first (potentially short) digit group
    size_t firstGroupLen = decStrLen % DEC_DIGITS_PER_UINT64;
    if (firstGroupLen == 0)
        firstGroupLen = DEC_DIGITS_PER_UINT64;
    array[0] = decString19CharsTo_uint64_t(decStr, firstGroupLen);
    decStr += firstGroupLen;
    digestsDone += firstGroupLen;

    // Process remaining digit groups
    uint64_t groupVal = 0;
    while (decStr < decStringEnd) {
        groupVal = decString19CharsTo_uint64_t(decStr, DEC_DIGITS_PER_UINT64);
        decStr += DEC_DIGITS_PER_UINT64;
        digestsDone += DEC_DIGITS_PER_UINT64;
        inplaceMulAddForConversion(array, arrayLen, groupVal, digestsDone);
    }
}

void inplaceMulAddForConversion(uint64_t *array, const size_t arrayLen, uint64_t z, size_t digetsDone) {
    // Perform the multiplication word by word
    size_t num_blocks = (long) (((double) digetsDone * 3.32193f) / 64.0f + 1);
    uint128_t yLong = 10000000000000000000ULL;
    uint128_t zLong = z;

    uint128_t product = 0;
    uint64_t carry = 0;
    size_t boundary = num_blocks + 1 > arrayLen ? arrayLen : num_blocks + 1;

    for (size_t i = 0; i < boundary; i++) {
        product = yLong * (array[i]) + carry;
        array[i] = (uint64_t) product;
        carry = product >> 64;
    }

    // Perform the addition
    uint128_t sum = (array[0]) + zLong;
    array[0] = (uint64_t) sum;
    carry = sum >> 64;
    for (size_t i = 1; i < boundary; i++) {
        sum = (array[i]) + carry;
        array[i] = (uint64_t) sum;
        carry = sum >> 64;
        if (carry == 0) break;
    }
}

uint64_t decString19CharsTo_uint64_t(const char *dexStr, size_t strLen) {
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
        j++; // start one later to make space for the negative sign
        loopEnd++; // adjust the loop end too
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

size_t calculateDecStringSpace(const bigInt *x) {
    size_t xLen = getLen(x);
    size_t xMaxBitLen = xLen * 64;
    size_t resMaxLen = (size_t) ((double) xMaxBitLen * 0.30103 + 3);
    return resMaxLen;
}

//fills the char array with the dec presentation of the bigInt
char *bigIntToDecString(bigInt *x, bool doFree) {
    if (isZero(x)) {
        if (doFree) {
            freeBigInt(x);
        }
        return getZeroString();
    }

    bigInt *convertX = doFree ? x : copyBigInt(x);
    if (global_config.parallel) {
        return bigIntToDecStringSchoenhageMultithread(convertX);
    } else {
        return bigIntToDecStringSchoenhage(convertX);
    }
}

//Frees x
char *bigIntToDecStringSchoenhage(bigInt *x) {
    struct schoenhageReturn *ret = bigIntToDecStringSchoenhageLenRet(x, 0, true);
    char *res = ret->res;
    free(ret);
    return res;
}

// FREES the bigInt that is passed
struct schoenhageReturn *bigIntToDecStringSchoenhageLenRet(bigInt *x, size_t digits, bool beginning) {
    size_t resMaxLen = (digits == 0 || beginning) ? calculateDecStringSpace(x) : digits + 1;
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

void bigIntToDecStringSchoenhageHelper(bigInt *x, size_t digits, char **resString, size_t *resStringCounter,
                                       bool beginning) {
    /* If we're smaller than a certain threshold, use the smallToString
       method, padding with leading zeroes when necessary. */
    size_t xLen = getLen(x);
    if (xLen <= DEC_STRING_SMALL_FASTER) {
        if (beginning) { // append a negative sign in the front if negative and in the beginning
            if (x->negative) {
                (*resString)[(*resStringCounter)++] = '-';
            }

            if (isZero(x)) { // should never happen
                freeBigInt(x);
                fprintf(stderr, "Error in dec string conversion: leading bigInt part is zero\n");
                return;
            }

            size_t resTmpLen = calculateDecStringSpace(x);
            char *resTmp = malloc(resTmpLen);
            mallocCheck(resTmp);

            bigInt *tmp = x;
            size_t resTmpPos = resTmpLen - 1;
            uint64_t reminder = 0;
            while (true) {
                bigInt *q = divideBy10Raised19(tmp, &reminder);
                freeBigInt(tmp);
                if (isZero(q)) {
                    freeBigInt(q);
                    break;
                }

                for(size_t i = 0; i < DEC_DIGITS_PER_UINT64 - 1; i++) {
                    resTmp[resTmpPos--] = decLookup[reminder % 10];
                    reminder = reminder / 10;
                }
                resTmp[resTmpPos--] = decLookup[reminder];

                tmp = q;
            }
            while(reminder != 0) {
                resTmp[resTmpPos--] = decLookup[reminder % 10];
                reminder = reminder / 10;
            }

            size_t resLen = resTmpLen - 1 - resTmpPos;
            memcpy(*resString + *resStringCounter, resTmp + resTmpPos + 1, resLen);
            *resStringCounter += resLen;
            free(resTmp);
            return;
        }

        if (isZero(x)) {
            freeBigInt(x);
            memset(*resString + *resStringCounter, '0', digits);
            *resStringCounter += digits;
            return;
        }
        // Translate number to string, DEC_DIGITS_PER_UINT64 digits a time
        size_t localResPos = *resStringCounter + digits - 1;
        bigInt *tmp = x;
        uint64_t reminder = 0;
        while (true) {
            bigInt *q = divideBy10Raised19(tmp, &reminder);
            freeBigInt(tmp);
            if (isZero(q)) {
                freeBigInt(q);
                break;
            }
            // conversion of 64bit integer to decimal string (always leading zeros to pad to DEC_DIGITS_PER_UINT64 digits)
            for(size_t i = 0; i < DEC_DIGITS_PER_UINT64 - 1; i++) {
                (*resString)[localResPos--] = decLookup[reminder % 10];
                reminder = reminder / 10;
            }
            (*resString)[localResPos--] = decLookup[reminder];

            tmp = q;
        }

        for(size_t i = 0; i < DEC_DIGITS_PER_UINT64 - 1; i++) {
            (*resString)[localResPos--] = decLookup[reminder % 10];
            reminder = reminder / 10;
            if (localResPos == *resStringCounter) {
                break;
            }
        }
        (*resString)[localResPos] = decLookup[reminder];

        *resStringCounter += digits;
        return;
    }

    size_t b, n;
    b = bitLength(x);
    n = (size_t) llroundl(log((double) b * log(2.0) / log(10.0)) / log(2.0) - 1.0);
    bigInt *v = getBigIntFromUnsignedInteger(10);
    for (size_t i = 0; i < n; i++) {
        bigInt *vNew = mulSingleThread(v, v);
        freeBigInt(v);
        v = vNew;
    }

    bigInt *r = nullptr;
    bigInt *q = divideModSingleThread(x, v, &r, true);
    char *filename_r = storeBigIntInSwap(r);

    size_t expectedDigits = 1 << n;
    // Now recursively build the two halves of each number.
    bigIntToDecStringSchoenhageHelper(q, digits - expectedDigits, resString, resStringCounter,
                                      beginning);
    r = loadBigIntFromSwap(r, filename_r);
    bigIntToDecStringSchoenhageHelper(r, expectedDigits, resString, resStringCounter, false);
}

struct schoenhageArgs {
    bigInt *x;
    size_t digits;
    bool beginning;
    size_t depth;
};

// Frees x
char *bigIntToDecStringSchoenhageMultithread(bigInt *x) {
    if (global_config.convertDepth == 0) {
        fprintf(stderr, "Warning: string multithread conversion method was called but convertDepth=0, using single thread version\n");
    }
    if (PARALLEL_DEC_STRING_FASTER > getLen(x) || global_config.convertDepth == 0) {
        return bigIntToDecStringSchoenhage(x);
    }

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
    if (global_config.superVerbose) {
        char *localTime = getCurrentDateTime();
        printf("Convert-Depth: %lu;  %s\n", depth, localTime);
        free(localTime);
    }
    if (depth == 0 || xLen < DEC_STRING_SMALL_FASTER) {
        struct schoenhageReturn *res = bigIntToDecStringSchoenhageLenRet(x, digits, beginning);
        return res;
    }

    size_t xBitLength = bitLength(x);
    size_t n = (size_t) llroundl(log((double) xBitLength * log(2.0) / log(10.0)) / log(2.0) - 1.0);
    bigInt *v = getBigIntFromUnsignedInteger(10);
    for (size_t i = 0; i < n; i++) {
        bigInt *vNew;
        if (depth == global_config.convertDepth) {
            vNew = mulParallel(v, v, global_config.mulDepth);
        } else {
            size_t parallelMuls = 1ULL << (global_config.convertDepth - depth);
            size_t mulDepth = calcMulDepthForParallelMuls(parallelMuls);
            vNew = mulParallel(v, v, mulDepth);
        }
        freeBigInt(v);
        v = vNew;
    }
    if (global_config.superVerbose && depth == global_config.convertDepth) {
        char *localTime = getCurrentDateTime();
        printf("First muls from conversion finished  %s\n", localTime);
        free(localTime);
    }

    bigInt *r = nullptr;
    bigInt *q;
    if (depth == global_config.convertDepth) {
        q = divideModMultiThread(x, v, &r, global_config.mulDepth, true);
        if (global_config.superVerbose) {
            char *localTime = getCurrentDateTime();
            printf("First div from conversion finished;  %s\n", localTime);
            free(localTime);
        }
    } else {
        size_t parallelMuls = 1 << (global_config.convertDepth - depth);
        size_t mulDepth = calcMulDepthForParallelMuls(parallelMuls);
        q = divideModMultiThread(x, v, &r, mulDepth, true);
    }

    size_t expectedDigits = 1ULL << n;

    // Now recursively build the two halves of each number.
    pthread_t thread_idConvert1;
    pthread_t thread_idConvert2;
    struct schoenhageArgs *args1 = malloc(sizeof(struct schoenhageArgs));
    mallocCheck(args1);
    args1->x = q;
    args1->digits = digits - expectedDigits;
    args1->beginning = beginning;
    args1->depth = depth - 1;
    struct schoenhageArgs *args2 = malloc(sizeof(struct schoenhageArgs));
    mallocCheck(args2);
    args2->x = r;
    args2->digits = expectedDigits;
    args2->beginning = false;
    args2->depth = depth - 1;
    if (pthread_create(&thread_idConvert1, nullptr, bigIntToDecStringSchoenhageMultithreadHelper, (void *) args1) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread_idConvert2, nullptr, bigIntToDecStringSchoenhageMultithreadHelper, (void *) args2) != 0) {
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

    if (global_config.superVerbose) {
        char *localTime = getCurrentDateTime();
        printf("Thread with Depth: %lu finished;  %s\n", depth, localTime);
        free(localTime);
    }
    return ret;
}
