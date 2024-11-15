#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bigIntMethods.h"
#include "bigIntUtil.h"
#include "bigIntAsm.h"

//allocates memory for a new bigInt of the given size
bigInt *newBigInt(size_t len) {
    if (len == 0) {
        fprintf(stderr, "newBigInt: len can not be zero!\n");
        exit(4);
    }
    bigInt *res = malloc(sizeof(bigInt));
    mallocCheck(res);

    res->bigIntArray = calloc(len, sizeof(uint64_t));
    mallocCheck(res->bigIntArray);
    res->start = 0;
    res->end = len;
    res->arrayOwner = true;
    res->negative = false;
    return res;
}

//creates a new bigInt Struct with the array of another bigInt->not the owner of the array
bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray) {
    if (start - end == 0) {
        return getZeroBigInt();
    }
    bigInt *res = malloc(sizeof(bigInt));
    mallocCheck(res);
    res->bigIntArray = bigIntArray;
    res->start = start;
    res->end = end;
    res->arrayOwner = false;
    res->negative = false;
    return res;
}

//Frees the memory of the BigInteger
void freeBigInt(bigInt *toDelete) {
    if (toDelete->arrayOwner) { //if the bigInt is the owner free the array
        free(toDelete->bigIntArray);
    }
    free(toDelete);
}

//deep copies BigInt
bigInt *copyBigInt(bigInt *x) {
    size_t xLen = x->end - x->start;
    bigInt *res = newBigInt(xLen);
    memcpy(res->bigIntArray, x->bigIntArray + x->start, xLen * 8);
    res->negative = x->negative;
    return res;
}

bool isValidBigInt(bigInt *x) {
    if (isZero(x)) return true;
    if (x->start >= x->end) return false;
    if (x->bigIntArray[x->end - 1] == 0) return false;
    return true;
}

bigInt *stripLeadingZeros(bigInt *x) {
    if (getLen(x) == 0) {
        freeBigInt(x);
        return getZeroBigInt();
    }
    size_t blocks = getOccupiedBlocks(x);
    blocks = blocks == 0 ? 1 : blocks;
    x->end = x->start + blocks;
    return x;
}

//Returns the bigInt with value 0
bigInt *getZeroBigInt() {
    return newBigInt(1);
}

bool isZero(bigInt *x) {
    if (x->end - x->start == 1 && x->bigIntArray[x->start] == 0) {
        return true;
    }
    return false;
}

size_t getLen(bigInt *x) {
    return (x->end) - (x->start);
}

size_t getOccupiedBlocks(bigInt *x) {
    long i = (long) x->end;
    size_t n = 0;
    while (i > (long) x->start + 1) {
        if (x->bigIntArray[i - 1] != 0) break;
        i--;
        n++;
    }
    return x->end - n - x->start;
}

// Returns -1, 0 or 1 as a is numerically less than, equal to, or greater than b
int compareBigInt(bigInt *a, bigInt *b) {
    if (isZero(a) && isZero(b))
        return 0;

    if (a->negative == b->negative) {
        if (a->negative)
            return compareBigIntArrays(b, a);
        else
            return compareBigIntArrays(a, b);
    }
    return a->negative ? 1 : -1;
}

int compareBigIntArrays(bigInt *a, bigInt *b) {
    if (isZero(a) && isZero(b)) {
        return 0;
    }

    size_t aLen = getLen(a);
    size_t bLen = getLen(b);
    if (aLen < bLen) {
        return -1;
    } else if (aLen > bLen) {
        return 1;
    }
    for (long i = (long) a->end - 1, j = (long) b->end - 1; i >= (long) a->start; i--, j--) {
        uint64_t aVal = a->bigIntArray[i];
        uint64_t bVal = b->bigIntArray[j];
        if (aVal < bVal)
            return -1;
        if (aVal > bVal)
            return 1;
    }
    return 0;
}

//compares like compareBigInt, but b is shifted n blocks (n*64 bit) to the left
int compareShiftedBigInt(bigInt *a, bigInt *b, size_t n) {
    long aLen = (long) (a->end - a->start) - (long) n;
    long bLen = (long) (b->end - b->start);
    if (aLen < bLen) {
        return -1;
    } else if (aLen > bLen) {
        return 1;
    }
    for (size_t i = a->end - 1, j = b->end - 1; i >= a->start; i--, j--) {
        uint64_t aVal = a->bigIntArray[i];
        uint64_t bVal = b->bigIntArray[j];
        if (aVal < bVal)
            return -1;
        if (aVal > bVal)
            return 1;
    }
    return 0;
}

//shifts bigInt to the left
bigInt *shiftLeft(bigInt *x, size_t n) {
    if (isZero(x)) {
        return getZeroBigInt();
    }
    if (n == 0) {
        return copyBigInt(x);
    }
    size_t toShift64 = n / 64;
    if (toShift64 == 0) {
        return shiftLeft_Asm(x, n);
    }
    size_t xLen = x->end - x->start;
    bigInt *resTmp = newBigInt(xLen + toShift64);
    memcpy(resTmp->bigIntArray + toShift64, x->bigIntArray, xLen * 8);
    if (n % 64 == 0) {
        return resTmp;
    }
    bigInt *res = shiftLeft_Asm(resTmp, n % 64);
    freeBigInt(resTmp);
    return res;
}

//shifts bigInt to the right
bigInt *shiftRight(bigInt *x, size_t n) {
    if (n == 0) {
        return copyBigInt(x);
    }
    size_t toShift64 = n / 64;
    if (toShift64 == 0) {
        return shiftRight_Asm(x, n);
    }
    size_t xLen = x->end - x->start;
    if (xLen <= toShift64) {
        return getZeroBigInt();
    }
    bigInt *resTmp = newBigInt(xLen - toShift64);
    memcpy(resTmp->bigIntArray, x->bigIntArray + toShift64, (xLen - toShift64) * 8);
    if (n % 64 == 0) {
        return resTmp;
    }
    bigInt *res = shiftRight_Asm(resTmp, n % 64);
    freeBigInt(resTmp);
    return res;
}

bigInt *shiftAdd(bigInt *x, bigInt *toShift, size_t n) {
    if (isZero(toShift)) { // toShift == 0
        return copyBigInt(x);
    }
    return shiftAdd_Asm(x, toShift, n);
}

//fills the char array with the hex presentation of the bigInt
char *bigIntToHexString(bigInt *x) {
    size_t lzcnt = custom_lzcnt(x->bigIntArray[x->end - 1]);
    size_t xLen = x->end - x->start;
    // error handling
    if (lzcnt == 64 && xLen == 1) {
        size_t resStrLen = x->negative ? 2 : 1;
        char *resStr = malloc(resStrLen + 1);
        mallocCheck(resStr);
        resStr[resStrLen] = '\0';
        if (x->negative) {
            resStr[0] = '-';
            resStr[1] = '0';
        } else {
            resStr[0] = '0';
        }
        return resStr;
    } else if (lzcnt == 64 && xLen > 1) {
        fprintf(stderr, "BigInt not printable, has leading zero block");
        exit(EXIT_FAILURE);
    }

    size_t lenInNibbles = xLen * 16 - (lzcnt / 4);
    char *resStr = uint64tArrayToHexString(x->bigIntArray, lenInNibbles, x->start, x->negative);
    return resStr;
}

//fills the char array with the dec presentation of the bigInt
char *bigIntToDecString(bigInt *x) {
    return bigIntToDecStringHelper(x);
}

//returns the bigInt of the HexString, hex is being freed
bigInt *hexStringToBigInt(const char *hexStr) {
    size_t hexStrLength = strlen(hexStr);
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
    } else if (hexStrLength >= 2 && hexStr[0] == '0') {
        fprintf(stderr, "number can not start with 0\n");
        exit(EXIT_FAILURE);
    }

    size_t resLength = hexStrLength / 16;
    if (hexStrLength % 16 != 0) resLength++;
    bigInt *res = newBigInt(resLength);
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
    size_t decStrLength = strlen(decStr);
    if (decStrLength == 0) {
        fprintf(stderr, "hexStr can not be of length 0");
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
        fprintf(stderr, "hexStr must contain a number");
        exit(EXIT_FAILURE);
    } else if (decStrLength >= 2 && decStr[0] == '0') {
        fprintf(stderr, "number can not start with 0");
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
    }
    return res;
}

//get the lower half of the bigInt (same array, new Struct with different pointers)
bigInt *getLowerFrom(bigInt *x, size_t n) {
    if (x->end < x->start + n) {
        n = x->end;
    }
    bigInt *res = stripLeadingZeros(newBigIntStruct(x->start, x->start + n, x->bigIntArray));
    return res;
}

//get the upper half of the bigInt (same array, new Struct with different pointers)
bigInt *getUpperFrom(bigInt *x, size_t n) {
    if (x->start + n >= x->end) {
        return getZeroBigInt();
    }
    bigInt *res = stripLeadingZeros(newBigIntStruct(x->start + n, x->end, x->bigIntArray));
    return res;
}

//Returns a slice of a bigInt for Toom-Cook
void getToomSlice(bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt *erg[]) {
    size_t len = x->end - x->start;
    size_t offset = fullSize - len;
    long long start0, end0, start1, end1, start2, end2;

    //slice 0
    start0 = (long long) x->start - (long long) (offset);
    end0 = (long long) ((long long) x->start + upperSize - 1 - offset);

    start1 = (long long) x->start + (long long) (upperSize + (1 - 1) * lowerSize - offset);
    end1 = start1 + (long long) lowerSize - 1;

    start2 = (long long) x->start + (long long) (upperSize + (2 - 1) * lowerSize - offset);
    end2 = start2 + (long long) lowerSize - 1;

    size_t size0, size1, size2;
    if (start0 < (long long) x->start) {
        start0 = (long long) x->start;
    }
    if (end0 < (long long) x->start) {
        size0 = 0;
    } else {
        size0 = ((end0 - start0) + 1);
    }
    if (start1 < (long long) x->start) {
        start1 = (long long) x->start;
    }
    if (end1 < (long long) x->start) {
        size1 = 0;
    } else {
        size1 = (end1 - start1) + 1;
    }
    if (start2 < (long long) x->start) {
        start2 = (long long) x->start;
    }
    if (end2 < (long long) x->start) {
        size2 = 0;
    } else {
        size2 = (end2 - start2) + 1;
    }

    bigInt *s0 = stripLeadingZeros(newBigIntStruct(x->start, x->start + size2, x->bigIntArray));
    erg[0] = s0;
    bigInt *s1 = stripLeadingZeros(newBigIntStruct(x->start + size2, x->start + size2 + size1, x->bigIntArray));
    erg[1] = s1;
    bigInt *s2 = stripLeadingZeros(newBigIntStruct(x->start + size2 + size1, x->start + size2 + size1 + size0, x->bigIntArray));
    erg[2] = s2;
}

bigInt *getBlock(bigInt *x, size_t index, size_t numBlocks, size_t blockLength) {
    size_t blockStart = index * blockLength;
    size_t xLen = x->end - x->start;
    if (blockStart >= xLen) {
        return getZeroBigInt();
    }

    size_t blockEnd;
    if (index == numBlocks - 1) {
        blockEnd = xLen;
    } else {
        blockEnd = (index + 1) * blockLength;
    }
    if (blockEnd > xLen) {
        return getZeroBigInt();
    }
    return stripLeadingZeros(newBigIntStruct(x->start + blockStart, x->start + blockEnd, x->bigIntArray));
}

bigInt *add(bigInt *x, bigInt *y) {
    if (!isValidBigInt(y) || !isValidBigInt(y)) {
        fprintf(stderr, "add: invalid bigInts supplied\n");
        exit(4);
    }

    bigInt *addBigger = x;
    bigInt *addSmaller = y;
    if (compareBigIntArrays(x, y) == -1) {
        bigInt *tmp = addBigger;
        addBigger = addSmaller;
        addSmaller = tmp;
    }

    if (x->negative == y->negative) {
        return add_Asm(addBigger, addSmaller, addBigger->negative);
    } else {
        return sub_Asm(addBigger, addSmaller, addBigger->negative);
    }
}

bigInt *sub(bigInt *x, bigInt *y) {
    if (!isValidBigInt(x) || !isValidBigInt(y)) {
        fprintf(stderr, "sub: invalid bigInts supplied\n");
        exit(4);
    }

    bigInt *subBigger = x;
    bigInt *subSmaller = y;
    if (compareBigIntArrays(x, y) == -1) {
        bigInt *tmp = subBigger;
        subBigger = subSmaller;
        subSmaller = tmp;
    }

    if (x->negative != y->negative) {
        if (x == subBigger)
            return add_Asm(subBigger, subSmaller, subBigger->negative);
        else
            return add_Asm(subBigger, subSmaller, subSmaller->negative);
    } else {
        if (x == subBigger)
            return sub_Asm(subBigger, subSmaller, subBigger->negative);
        else
            return sub_Asm(subBigger, subSmaller, !subBigger->negative);
    }
}
