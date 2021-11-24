#include "BigInt.h"
#include "util.h"
#include "BigIntAsm.h"
#include <immintrin.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

extern jmp_buf exceptionJump; //Jump point when a error occurs

void shiftAddHelper(bigInt *bigger, bigInt *smaller, bigInt *res, size_t biggerCount, size_t smallerCount, size_t i);

void addHelper(bigInt *bigger, bigInt *smaller, bigInt *res);

//allocates memory for a new BigInt of the given size
bigInt *newBigInt(size_t len) {
    bigInt *res = malloc(sizeof(bigInt));
    if (res == NULL) {
        exception();
    }

    res->bigIntArray = calloc(len, sizeof(uint64_t));
    if (res->bigIntArray == NULL) {
        exception();
    }
    res->start = 0;
    res->end = len;
    res->arrayOwner = true;
    res->negative = false;
    return res;
}

//creates a new bigInt Struct with the array of another bigInt->not the owner of the array
bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray) {
    bigInt *res = malloc(sizeof(bigInt));
    if (res == NULL) {
        exception();
    }
    res->bigIntArray = bigIntArray;
    res->start = start;
    res->end = end;
    res->arrayOwner = false;
    res->negative = false;
    return res;
}

void exception() {
    printf("Malloc Failed\n");
    longjmp(exceptionJump, 1);
}

//Frees the memory of the BigInteger
void freeBigInt(bigInt *toDelete) {
    if (toDelete->arrayOwner) { //if the bigInt is the owner free the array
        free(toDelete->bigIntArray);
    }
    free(toDelete);
}

//converts a bigInt to a hex-string
char *bigIntToStr(bigInt *x) {
    return bigIntToHexString(x);
    //TODO decimal output
}

//prints the given bigInt
void printBigInt(bigInt *x) {
    char *str = bigIntToStr(x);
    if (x->negative) {
        printf("-0x%s\n", str);
    } else {
        printf("0x%s\n", str);
    }
    free(str);
}

//fills the char array with the hex presentation of the bigInt
char *bigIntToHexString(bigInt *x) {
    size_t lenInBytes = (x->end - x->start) * 8;
    return uint64tToHexString(x->bigIntArray, lenInBytes, x->start);
}

//fills the char array with the dec presentation of the bigInt
char *bigIntToDecString(bigInt *x) {
    size_t lenInBytes = (x->end - x->start) * 8;
    return uint64tToDecString(x->bigIntArray, lenInBytes);
}

//returns the bigInt of the HexString, hex is being freed
bigInt *hexStringToBigInt(char hex[]) {
    char *paddedHex = extendHexString(hex);
    size_t length = strlen(paddedHex);
    bigInt *res = newBigInt(length / 16);

    size_t j = 0;
    uint64_t akt = 0;
    size_t resBigIntCounter = res->end - 1;
    uint8_t akt8;
    size_t i = 0;
    akt8 = hexToNibble(paddedHex[i]);
    akt8 <<= 4;
    i++;
    akt8 += hexToNibble(paddedHex[i]);
    akt += akt8;
    i++;
    for (; i < length - 1; i++) {
        if (j == 7) {
            res->bigIntArray[resBigIntCounter] = akt;
            akt = 0;
            resBigIntCounter--;
            j = 0;
        } else {
            akt <<= 8;
            j++;
        }
        akt8 = hexToNibble(paddedHex[i]);
        akt8 <<= 4;
        i++;
        akt8 += hexToNibble(paddedHex[i]);
        akt += akt8;
    }
    if (i < length) {
        if (j == 7) {
            res->bigIntArray[resBigIntCounter] = akt;
            resBigIntCounter--;
            j = 0;
        } else {
            akt <<= 4;
            j++;
        }
        akt8 = hexToNibble(paddedHex[i]);
        akt += akt8;
    }
    if (j == 7) {
        res->bigIntArray[resBigIntCounter] = akt;
    }
    if (res->bigIntArray[res->end - 1] == 0 && res->end - res->start > 1) {
        res->end -= 1;
    }
    free(paddedHex);
    return res;
}

//returns the number of arrayFields the BigInt is using
size_t getOccupiedFields(bigInt *x) {
    size_t num = x->end;
    while (num > x->start + 1 && x->bigIntArray[num - 1] == 0) {
        num--;
    }
    return num - x->start;
}

//get the lower half of the bigInt (same array, new Struct with different pointers)
bigInt *getLowerFrom(bigInt *x, size_t n) {
    if (x->end < n) {
        n = x->end;
    }
    bigInt *res = newBigIntStruct(x->start, n, x->bigIntArray);
    return res;
}

//get the upper half of the bigInt (same array, new Struct with different pointers)
bigInt *getUpperFrom(bigInt *x, size_t n) {
    if (n >= x->end) {
        return newBigInt(1);
    }
    bigInt *res = newBigIntStruct(n, x->end, x->bigIntArray);
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

    //printf("\n0 : X-start:%zu X-end:%zu\n", x->start, x->end);
    //printf("0 : start:%lld end:%lld\n", start0, end0);
    //printf("1 : start:%lld end:%lld\n", start1, end1);
    //printf("2 : start:%lld end:%lld\n", start2, end2);

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
    //printf("Lower:%zu Upper:%zu fullsize:%zu xLen:%zu\n", lowerSize, upperSize, fullSize, len);
    //printf("Slice:%zu : start:%lu end:%lu\n", size2, x->start, x->start + size2);
    //printf("Slice:%zu : start:%lu end:%lu\n", size1, x->start + size2, x->start + size2 + size1);
    //printf("Slice:%zu : start:%lu end:%lu\n", size0, x->start + size2 + size1, x->start + size2 + size1 + size0);

    bigInt *s0 = newBigIntStruct(x->start, x->start + size2, x->bigIntArray);
    erg[0] = s0;
    bigInt *s1 = newBigIntStruct(x->start + size2, x->start + size2 + size1, x->bigIntArray);
    erg[1] = s1;
    bigInt *s2 = newBigIntStruct(x->start + size2 + size1, x->start + size2 + size1 + size0, x->bigIntArray);
    erg[2] = s2;
}

bigInt *smartAdd(bigInt *x, bigInt *y) {
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    if (x->negative && y->negative) {
        return add_Asm(x, y, true);
    } else if (!x->negative && !y->negative) {
        return add_Asm(x, y, false);
    } else if (x->negative && !y->negative) {
        if (xLen < yLen) {
            return sub_Asm(y, x, false);
        } else if (yLen < xLen) {
            return sub_Asm(x, y, true);
        } else {
            if (x->bigIntArray[x->end - 1] < y->bigIntArray[y->end - 1]) {
                return sub_Asm(y, x, false);
            } else {
                return sub_Asm(x, y, true);
            }
        }
    } else {
        if (xLen > yLen) {
            return sub_Asm(y, x, false);
        } else if (yLen > xLen) {
            return sub_Asm(x, y, true);
        } else {
            if (x->bigIntArray[x->end - 1] > y->bigIntArray[y->end - 1]) {
                return sub_Asm(y, x, false);
            } else {
                return sub_Asm(x, y, true);
            }
        }
    }
}

//adds the the BigIntegers x and y and returns the result as a new BigInteger
bigInt *add(bigInt *x, bigInt *y, bool negative) {
    bigInt *res;
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    if (xLen < yLen) {
        res = newBigInt(yLen + 1);
        addHelper(y, x, res);
    } else {
        res = newBigInt(xLen + 1);
        addHelper(x, y, res);
    }
    if (res->bigIntArray[res->end - 1] == 0 && res->end - res->start > 1) {
        res->end -= 1;
    }
    res->negative = negative;
    return res;
}

//shifts x by n bits, n must be smaller than 64
bigInt *shiftLeft(bigInt *x, size_t n) {
    bigInt *result = newBigInt(x->end - x->start + 1);
    result->negative = x->negative;
    size_t j = 0;
    for (size_t i = x->start; i < x->end; ++i, ++j) {
        //put "lower" part of shifted bits in result
        result->bigIntArray[j] |= (x->bigIntArray[i] << n);
        //put "higher" part of shifted bits in result
        result->bigIntArray[j + 1] |= (x->bigIntArray[i] >> (64 - n));
    }

    if (result->bigIntArray[result->end - 1] == 0 && result->end - result->start > 1) {
        result->end -= 1;
    }
    return result;
}

//shifts x by n bits, n must be smaller than 64
bigInt *shiftRight(bigInt *x, size_t n) {
    bigInt *result = newBigInt(x->end - x->start);
    result->negative = x->negative;
    size_t j = 0;
    size_t i = x->start;
    if (i < x->end) {
        result->bigIntArray[j] |= (x->bigIntArray[i] >> n);
        ++i;
        ++j;
    }

    for (; i < x->end; ++i, ++j) {
        //put "higher" part of shifted bits in result
        result->bigIntArray[j] |= (x->bigIntArray[i] >> n);
        //put "lower" part of shifted bits in result
        result->bigIntArray[j - 1] |= (x->bigIntArray[i] << (64 - n));
    }

    if (result->bigIntArray[result->end - 1] == 0 && result->end - result->start > 1) {
        result->end -= 1;
    }
    return result;
}

//shifts toShift by n QuadWords and add it to x returning a new bigInt with the result
bigInt *shiftAdd(bigInt *x, bigInt *toShift, size_t n) {
    size_t xLen = x->end - x->start;
    size_t toShiftLen = toShift->end - toShift->start + n;
    bigInt *res = newBigInt(xLen > toShiftLen ? xLen + 1 : toShiftLen + 1);

    //for n copy x or fill result with 0
    size_t xCount = x->start;
    size_t i = 0;
    size_t nCounter = 0;
    for (; nCounter < n && xCount < x->end; i++) { //copy x
        res->bigIntArray[i] = x->bigIntArray[xCount];
        xCount++;
        nCounter++;
    }
    for (; nCounter < n; i++) { //fill with zero
        res->bigIntArray[i] = 0;
        nCounter++;
    }
    //the "shift" is finished, now add but with the allready used counters(xCount ,i)
    if (xLen < toShiftLen) {
        shiftAddHelper(toShift, x, res, toShift->start, xCount, i);
    } else {
        shiftAddHelper(x, toShift, res, xCount, toShift->start, i);
    }

    if (res->bigIntArray[res->end - 1] == 0 && res->end - res->start > 1) {
        res->end -= 1;
    }
    return res;
}

bigInt *smartSub(bigInt *x, bigInt *y) {
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    if (!x->negative && y->negative) {
        return add_Asm(x, y, false);
    } else if (x->negative && !y->negative) {
        return add_Asm(x, y, true);
    } else if (x->negative && !y->negative) {
        if (xLen < yLen){
            return sub_Asm(y, x, false);
        } else if (yLen < xLen) {
        return sub_Asm(x, y, true);
        } else {
            if (x->bigIntArray[x->end - 1] < y->bigIntArray[y->end - 1]) {
                return sub_Asm(y, x, false);
            } else {
                return sub_Asm(x, y, true);
            }
        }
    } else {
        if (xLen < yLen) {
            return sub_Asm(y, x, true);
        } else if (yLen < xLen) {
            return sub_Asm(x, y, false);
        } else {
            if (x->bigIntArray[x->end - 1] < y->bigIntArray[y->end - 1]) {
                return sub_Asm(y, x, true);
            } else {
                return sub_Asm(x, y, false);
            }
        }
    }
}

//subtracts the the BigIntegers y from x and returns the result as a new BigInteger
bigInt *sub(bigInt *x, bigInt *y, bool negative) {
    bigInt *res;
    unsigned char carry = '\0';
    size_t i = 0;
    size_t xCount = x->start;
    size_t yCount = y->start;
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    res = newBigInt(xLen);

    for (; i < yLen; i++) { //sub
        carry = _subborrow_u64(carry, x->bigIntArray[xCount], y->bigIntArray[yCount],
                               (unsigned long long *) &res->bigIntArray[i]);
        xCount++;
        yCount++;
    }
    for (; i < xLen; i++) { //copySub
        carry = _subborrow_u64(carry, x->bigIntArray[xCount], 0, (unsigned long long *) &res->bigIntArray[i]);
        xCount++;
    }

    if (res->bigIntArray[res->end - 1] == 0 && res->end - res->start > 1) {
        res->end -= 1;
    }
    res->negative = negative;
    return res;
}

//adds the the BigIntegers bigger and smaller and stores the result in res
//uses the passed parameters from the shiftAdd
void shiftAddHelper(bigInt *bigger, bigInt *smaller, bigInt *res, size_t biggerCount, size_t smallerCount, size_t i) {
    unsigned char carry = '\0';

    for (; smallerCount < smaller->end; i++) { //add
        carry = _addcarry_u64(carry, smaller->bigIntArray[smallerCount], bigger->bigIntArray[biggerCount],
                              (unsigned long long *) &res->bigIntArray[i]);
        smallerCount++;
        biggerCount++;
    }
    for (; biggerCount < bigger->end; i++) { //copyAdd
        carry = _addcarry_u64(carry, 0, bigger->bigIntArray[biggerCount], (unsigned long long *) &res->bigIntArray[i]);
        biggerCount++;
    }
    _addcarry_u64(carry, 0, 0, (unsigned long long *) &res->bigIntArray[i]); //carry

}

//adds the the BigIntegers bigger and smaller and stores the result in res
void addHelper(bigInt *bigger, bigInt *smaller, bigInt *res) {
    unsigned char carry = '\0';
    size_t i = 0;
    size_t smallerCount = smaller->start;
    size_t biggerCount = bigger->start;
    for (; smallerCount < smaller->end; i++) { //add
        carry = _addcarry_u64(carry, smaller->bigIntArray[smallerCount], bigger->bigIntArray[biggerCount],
                              (unsigned long long *) &res->bigIntArray[i]);
        smallerCount++;
        biggerCount++;
    }
    for (; biggerCount < bigger->end; i++) { //copyAdd
        carry = _addcarry_u64(carry, 0, bigger->bigIntArray[biggerCount], (unsigned long long *) &res->bigIntArray[i]);
        biggerCount++;
    }
    _addcarry_u64(carry, 0, 0, (unsigned long long *) &res->bigIntArray[i]); //carry
}

void print128(__int128 x) {
    uint64_t low = (uint64_t) x;
    uint64_t high = (x >> 64);
    printf("Num:%"PRIx64"%"PRIx64"\n", high, low);
}

bigInt *exactDivideBy3(bigInt *x) {
    __int128 LONG_MASK = 0xffffffffffffffff;
    long len = (long) x->end - (long) x->start;
    bigInt *result = newBigInt(x->end - x->start);
    result->negative = x->negative;
    unsigned long borrow;
    unsigned __int128 q, xx, w;
    borrow = 0;
    long j = (long) x->start;
    for (long i = 0; i < len; i++, j++) {
        xx = ((__int128) (x->bigIntArray[j])) & LONG_MASK;

        //print128(xx);
        //printf("Borrow: %ld\n", borrow);
        w = xx - borrow;
        if (borrow > xx) {
            borrow = 1;
        } else {
            borrow = 0;
        }
        q = (unsigned __int128) (w * 0xAAAAAAAAAAAAAAAB);// & LONG_MASK;
        //printf("Erg:%"PRIx64"\n", (uint64_t) q);
        result->bigIntArray[i] = (uint64_t) q;

        if ((unsigned long long) q >= 0x5555555555555556) {
            borrow++;
            if ((unsigned long long) q >= 0xAAAAAAAAAAAAAAAB) {
                borrow++;
            }
        }
    }
    if (result->bigIntArray[result->end - 1] == 0 && result->end - result->start > 1) {
        result->end -= 1;
    }
    return result;
}