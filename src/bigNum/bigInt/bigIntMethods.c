#include "bigIntMethods.h"

#include "bigIntString.h"
#include "bigIntAsm.h"
#include "bigIntAlloc.h"
#include "../misc.h"

#include <string.h>
#include <stdio.h>
#include <immintrin.h>

bigInt *add_helper(const bigInt *x, const bigInt *y, bool negative);

bigInt *sub_helper(const bigInt *x, const bigInt *y, bool negative);

void shiftAddSameNumberHelper(bigInt *x, const bigInt *toShift, size_t n, size_t xLenOld);

bool isValidBigInt(const bigInt *x) {
    if (x == NULL || x->bigIntArray == NULL) return false;
    if (isZero(x)) return true;
    if (x->start >= x->end) return false;
    if (x->bigIntArray[x->end - 1] == 0) return false;
    return true;
}

// Can modify and change bigInt/pointer
void stripLeadingZeros(bigInt *x) {
    size_t blocks = getOccupiedBlocks(x);
    blocks = blocks == 0 ? 1 : blocks;
    x->end = x->start + blocks;
}

//Returns the bigInt with value 0
bigInt *getZeroBigInt() {
    return newBigInt(1);
}

bigInt *getBigIntFromUnsignedInteger(uint64_t integer) {
    bigInt *res = newBigIntNotZeroed(1);
    res->bigIntArray[0] = integer;
    return res;
}

bigInt *getBigIntFromSignedInteger(int64_t integer) {
    bigInt *res = newBigIntNotZeroed(1);
    if (integer < 0) {
        res->negative = true;
        integer = -integer;
    }
    res->bigIntArray[0] = integer;
    return res;
}

bool isZero(const bigInt *x) {
    if (getLen(x) == 1 && x->bigIntArray[x->start] == 0) {
        return true;
    }
    return false;
}

size_t getLen(const bigInt *x) {
    return (x->end) - (x->start);
}

// -> max length 2000 petabytes
size_t bitLength(const bigInt *x) {
    size_t xLen = x->end - x->start;
    return xLen * 64 - custom_lzcnt(x->bigIntArray[x->end - 1]);
}

size_t getOccupiedBlocks(const bigInt *x) {
    long i = (long) x->end;
    size_t n = 0;
    while (i > (long) x->start + 1) {
        if (x->bigIntArray[i - 1] != 0) break;
        i--;
        n++;
    }
    return x->end - n - x->start;
}

size_t getTrailingZeroBlocks(const bigInt *x) {
    size_t i = 0;
    for (; i < x->end; i++) {
        if (x->bigIntArray[x->start + i] != 0) {
            break;
        }
    }
    return i;
}

void negateBigInt(bigInt *x) {
    x->negative = !x->negative;
}

// Returns -1, 0 or 1 as a is numerically less than, equal to, or greater than b
int compareBigInt(const bigInt *a, const bigInt *b) {
    if (isZero(a) && isZero(b)) {
        return 0;
    }
    if (isZero(a)) {
        return b->negative ? 1 : -1;
    }
    if (isZero(b)) {
        return a->negative ? -1 : 1;
    }

    if (a->negative == b->negative) {
        if (a->negative) {
            return compareBigIntArrays(b, a);
        } else {
            return compareBigIntArrays(a, b);
        }
    }
    return a->negative ? 1 : -1;
}

int compareBigIntArrays(const bigInt *a, const bigInt *b) {
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
        if (aVal < bVal) {
            return -1;
        }
        if (aVal > bVal) {
            return 1;
        }
    }
    return 0;
}

//compares like compareBigInt, but b is shifted n blocks (n*64 bit) to the left
int compareShiftedBigInt(const bigInt *a, const bigInt *b, size_t n) {
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
bigInt *shiftLeft(const bigInt *x, const size_t n) {
    if (isZero(x)) {
        return getZeroBigInt();
    }
    if (n == 0) {
        return copyBigInt(x);
    }
    size_t toShift64 = n / 64;
    if (toShift64 == 0) {
        bigInt *resTmp = shiftLeft_Asm(x, n);
        resTmp->negative = x->negative;
        return resTmp;
    }
    size_t xLen = x->end - x->start;
    bigInt *resTmp = newBigInt(xLen + toShift64);
    memcpy(resTmp->bigIntArray + toShift64, x->bigIntArray + x->start, xLen * 8);
    if (n % 64 == 0) {
        resTmp->negative = x->negative;
        return resTmp;
    }
    bigInt *res = shiftLeft_Asm(resTmp, n % 64);
    freeBigInt(resTmp);
    res->negative = x->negative;
    return res;
}

//shifts bigInt to the right
bigInt *shiftRight(const bigInt *x, const size_t n) {
    if (isZero(x)) {
        return getZeroBigInt();
    }
    if (n == 0) {
        return copyBigInt(x);
    }
    size_t toShift64 = n / 64;
    if (toShift64 == 0) {
        bigInt *resTmp = shiftRight_Asm(x, n);
        resTmp->negative = x->negative;
        stripLeadingZeros(resTmp);
        return resTmp;
    }
    size_t xLen = x->end - x->start;
    if (xLen <= toShift64) {
        return getZeroBigInt();
    }
    bigInt *resTmp = newBigInt(xLen - toShift64);
    memcpy(resTmp->bigIntArray, x->bigIntArray + x->start + toShift64, (xLen - toShift64) * 8);
    if (n % 64 == 0) {
        resTmp->negative = x->negative;
        return resTmp;
    }
    bigInt *res = shiftRight_Asm(resTmp, n % 64);
    freeBigInt(resTmp);
    res->negative = x->negative;
    stripLeadingZeros(res);
    return res;
}

bigInt *shiftAdd(const bigInt *x, const bigInt *toShift, const size_t n) {
    if ((x->negative && !isZero(x)) || (toShift->negative && !isZero(toShift))) {
        fprintf(stderr, "shiftAdd is only defined for non negative integers\n");
        exit(EXIT_FAILURE);
    }
    if (isZero(toShift)) {
        // toShift == 0
        return copyBigInt(x);
    }
    if (n == 0) {
        return add(x, toShift);
    }
    return shiftAdd_Asm(x, toShift, n);
}

// adds toShift to x; with toShift shifted n blocks to the left, the result will be in x after the call
// Changes x->bigIntArray, will resize x->bigIntArray if there is not enough space
// DO NOT USE WHEN THE bigIntArray IS STILL USED SOMEWHERE ELSE
void shiftAddSameNumberSafe(bigInt *x, const bigInt *toShift, const size_t n) {
    if (!x->arrayOwner) {
        fprintf(stderr, "shiftAddSameNumberSafe: x is not array owner\n");
        exit(EXIT_FAILURE);
    }
    if ((x->negative && !isZero(x)) || (toShift->negative && !isZero(toShift))) {
        fprintf(stderr, "shiftAddSameNumber is only defined for non negative integers\n");
        exit(EXIT_FAILURE);
    }
    if (isZero(toShift)) {
        return;
    }

    const size_t xLen = getLen(x);
    const size_t toShiftLen = getLen(toShift);
    if (xLen < toShiftLen + n) {
        // resize
        void *tmp = realloc(x->bigIntArray, (x->start + toShiftLen + n + 1) * 8);
        mallocCheck(tmp);
        x->bigIntArray = tmp;
        x->completeLength = x->start + toShiftLen + n + 1;
        x->end = x->start + toShiftLen + n + 1;
        x->bigIntArray[x->end - 1] = 0;

        shiftAddSameNumberHelper(x, toShift, n, xLen);
        stripLeadingZeros(x);
    } else {
        unsigned char carry = 1;
        unsigned long long out;
        carry = _addcarry_u64(carry, x->bigIntArray[x->start + n + toShiftLen - 1],
                              toShift->bigIntArray[toShift->start + toShiftLen - 1], &out);
        if (carry != 0) {
            for (size_t i = x->start + n + toShiftLen; i < x->end; i++) {
                if (_popcnt64(x->bigIntArray[i]) != 64) {
                    // no resize
                    shiftAddSameNumberHelper(x, toShift, n, xLen);
                    stripLeadingZeros(x);
                    return;
                }
            }
            // resize
            void* tmp = realloc(x->bigIntArray, (x->start + xLen + 1) * 8);
            mallocCheck(tmp);
            x->bigIntArray = tmp;
            x->completeLength = x->start + xLen + 1;
            x->end = x->start + toShiftLen + n + 1;
            x->bigIntArray[x->end - 1] = 0;
            shiftAddSameNumberHelper(x, toShift, n, xLen);
            stripLeadingZeros(x);
            return;
        }
        // no resize
        shiftAddSameNumberHelper(x, toShift, n, xLen);
        stripLeadingZeros(x);
    }
}

void shiftAddSameNumberHelper(bigInt *x, const bigInt *toShift, const size_t n, const size_t xLenOld) {
    if (xLenOld <= n) {
        if (xLenOld < n) {
            memset(x->bigIntArray + x->start + xLenOld, 0, (n - xLenOld) * 8);
        }
        memcpy(x->bigIntArray + x->start + n, toShift->bigIntArray + toShift->start, getLen(toShift) * 8);
        return;
    }
    if (xLenOld < getLen(toShift) + n) {
        memset(x->bigIntArray + x->start + xLenOld, 0, (getLen(toShift) + n - xLenOld) * 8);
    }

    shiftAddSameNumber_Asm(x, toShift, n);
}

// the caller must guarantee that x is big enough to hold the result
// x can have leading zero blocks. If x had leading zero blocks, if not filled by the addition they will remain
void shiftAddSameNumber(bigInt *x, const bigInt *toShift, const size_t n) {
    if (!x->arrayOwner) {
        fprintf(stderr, "shiftAddSameNumber: x is not array owner\n");
        exit(EXIT_FAILURE);
    }
    if ((x->negative && !isZero(x)) || (toShift->negative && !isZero(toShift))) {
        fprintf(stderr, "shiftAddSameNumber is only defined for non negative integers\n");
        exit(EXIT_FAILURE);
    }

    if (isZero(toShift)) {
        return;
    }
    shiftAddSameNumber_Asm(x, toShift, n);
}

//get the lower half of the bigInt (same array, new Struct with different pointers)
bigInt *getLowerFrom(const bigInt *x, size_t n) {
    if (x->end < x->start + n) {
        n = getLen(x);
    }
    bigInt *res = newBigIntStruct(x->start, x->start + n, x->bigIntArray);
    stripLeadingZeros(res);
    return res;
}

//get the upper half of the bigInt (same array, new Struct with different pointers)
bigInt *getUpperFrom(const bigInt *x, size_t n) {
    if (x->start + n >= x->end) {
        return getZeroBigInt();
    }
    bigInt *res = newBigIntStruct(x->start + n, x->end, x->bigIntArray);
    stripLeadingZeros(res);
    return res;
}

//Returns a slice of a bigInt for Toom-Cook
void getToomSlice(const bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt *erg[]) {
    size_t len = x->end - x->start;
    size_t offset = fullSize - len;
    long long start0, end0, start1, end1, start2, end2;

    //slice 0
    start0 = (long long) x->start - (long long) (offset);
    end0 = (long long) ((long long) x->start + upperSize - 1 - offset);

    start1 = (long long) x->start + (long long) (upperSize - offset);
    end1 = start1 + (long long) lowerSize - 1;

    start2 = (long long) x->start + (long long) (upperSize + lowerSize - offset);
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

    bigInt *s0 = newBigIntStruct(x->start, x->start + size2, x->bigIntArray);
    stripLeadingZeros(s0);
    erg[0] = s0;
    bigInt *s1 = newBigIntStruct(x->start + size2, x->start + size2 + size1, x->bigIntArray);
    stripLeadingZeros(s1);
    erg[1] = s1;
    bigInt *s2 = newBigIntStruct(x->start + size2 + size1, x->start + size2 + size1 + size0, x->bigIntArray);
    stripLeadingZeros(s2);
    erg[2] = s2;
}

bigInt *getBlock(const bigInt *x, size_t index, size_t numBlocks, size_t blockLength) {
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
    bigInt *res = newBigIntStruct(x->start + blockStart, x->start + blockEnd, x->bigIntArray);
    stripLeadingZeros(res);
    return res;
}

bigInt *add(const bigInt *x, const bigInt *y) {
    const bigInt *addBigger = x;
    const bigInt *addSmaller = y;
    if (compareBigIntArrays(x, y) == -1) {
        const bigInt *tmp = addBigger;
        addBigger = addSmaller;
        addSmaller = tmp;
    }
    if (isZero(addSmaller)) {
        return copyBigInt(addBigger);
    }

    if (x->negative == y->negative) {
        return add_helper(addBigger, addSmaller, addBigger->negative);
    } else {
        return sub_helper(addBigger, addSmaller, addBigger->negative);
    }
}

bigInt *add_helper(const bigInt *x, const bigInt *y, bool negative) {
    bigInt *res = newBigIntNotZeroed(getLen(x) + 1);
    res->bigIntArray[res->end - 1] = 0;
    do_add_asm(x, y, res);
    res->negative = negative;
    if (getLen(res) > 1 && res->bigIntArray[res->end - 1] == 0)
        res->end -= 1;
    return res;
}

bigInt *sub(const bigInt *x, const bigInt *y) {
    const bigInt *subBigger = x;
    const bigInt *subSmaller = y;
    if (compareBigIntArrays(x, y) == -1) {
        const bigInt *tmp = subBigger;
        subBigger = subSmaller;
        subSmaller = tmp;
    }
    if (isZero(subSmaller)) {
        bigInt *res = copyBigInt(subBigger);
        if (subBigger == y) {
            res->negative = !y->negative;
        }
        return res;
    }

    if (x->negative != y->negative) {
        if (x == subBigger) {
            return add_helper(subBigger, subSmaller, subBigger->negative);
        } else {
            return add_helper(subBigger, subSmaller, subSmaller->negative);
        }
    } else {
        if (x == subBigger) {
            return sub_helper(subBigger, subSmaller, subBigger->negative);
        } else {
            return sub_helper(subBigger, subSmaller, !subBigger->negative);
        }
    }
}

bigInt *sub_helper(const bigInt *x, const bigInt *y, bool negative) {
    bigInt *res = newBigIntNotZeroed(getLen(x));
    do_sub_asm(x, y, res);
    res->negative = negative;
    const size_t blocks = getOccupiedBlocks(res);
    res->end = res->start + blocks;
    return res;
}
