#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bigIntMethods.h"
#include "bigIntUtil.h"
#include "bigIntAsm.h"
#include "mulAsm.h"

//allocates memory for a new bigInt of the given size
bigInt *newBigInt(size_t len) {
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
    bigInt *res = malloc(sizeof(bigInt));
    mallocCheck(res);
    res->bigIntArray = bigIntArray;
    res->start = start;
    res->end = end;
    res->arrayOwner = false;
    res->negative = false;
    return res;
}

bigInt *copyBigInt(bigInt *x) {
    size_t xLen = x->end - x->start;
    bigInt *res = newBigInt(xLen);
    memcpy(res->bigIntArray, x->bigIntArray, xLen * 8);
    res->negative = x->negative;
    return res;
}

//Frees the memory of the BigInteger
void freeBigInt(bigInt *toDelete) {
    if (toDelete->arrayOwner) { //if the bigInt is the owner free the array
        free(toDelete->bigIntArray);
    }
    free(toDelete);
}

//Returns the bigInt with value 0
bigInt *getZeroBigInt() {
    return newBigInt(1);
}

// Returns -1, 0 or 1 as a is numerically less than, equal to, or greater than b
int compareBigInt(bigInt *a, bigInt *b) {
    size_t aLen = a->end - a->start;
    size_t bLen = b->end - b->start;
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

//compares like compareBigInt, but b is shifted n*64 to the left
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

//Removes all leading zero blocks
void bigIntRemoveLeadingZeroBlocks(bigInt *x) {
    x->end = x->start + getOccupiedFields_Asm(x);
}

//prints bigInt in Hex
void printBigIntHex(bigInt *x) {
    char *tmp = bigIntToHexString(x);
    printf("%s\n", tmp);
    free(tmp);
}

//prints bigInt in Dec
void printBigIntDec(bigInt *x) {
    char *tmp = bigIntToDecString(x);
    printf("%s\n", tmp);
    free(tmp);
}

//shifts bigInt to the left
bigInt *shiftLeft(bigInt *x, size_t n) {
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

//fills the char array with the hex presentation of the bigInt
char *bigIntToHexString(bigInt *x) {
    size_t lenInBytes = (x->end - x->start) * 8;
    if (x->negative) lenInBytes += 1;
    char *resChar = uint64tToHexString(x->bigIntArray, lenInBytes, x->start);
    if (x->negative) resChar[0] = '-';
    return resChar;
}

//fills the char array with the dec presentation of the bigInt
char *bigIntToDecString(bigInt *x) {
    return bigIntToDecStringHelper(x);
}

//returns the bigInt of the HexString, hex is being freed
bigInt *hexStringToBigInt(char hex[]) { //TODO support sign and fix crash when length is multiple of 16
    char *paddedHex = extendHexString(hex);
    size_t length = strlen(paddedHex);
    size_t resLength = length / 16;
    if (length % 16 != 0) resLength++;
    bigInt *res = newBigInt(resLength);

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
    return newBigIntStruct(x->start + blockStart, x->start + blockEnd, x->bigIntArray);
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
