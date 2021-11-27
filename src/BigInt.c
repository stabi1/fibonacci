#include "BigInt.h"
#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"
#include "BigIntAsm.h"

extern jmp_buf exceptionJump; //Jump point when a error occurs

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

bigInt *smartSub(bigInt *x, bigInt *y) {
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    if (!x->negative && y->negative) {
        return add_Asm(x, y, false);
    } else if (x->negative && !y->negative) {
        return add_Asm(x, y, true);
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