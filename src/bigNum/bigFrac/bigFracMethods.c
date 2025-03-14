#include "bigFracMethods.h"

#include "../bigInt/bigIntMethods.h"
#include "../bigInt/bigIntAsm.h"
#include "../misc.h"

#include <string.h>

bigFrac *addBigFrac_helper(const bigFrac *x, const bigFrac *y, bool negative);

bigFrac *subBigFrac_helper(const bigFrac *x, const bigFrac *y, bool negative);


bigFrac *getBigFracFromUnsignedInteger(uint64_t integer) {
    bigInt *bigIntPart = getBigIntFromUnsignedInteger(integer);
    return newBigFracFromBigInt(bigIntPart, false);
}

bigFrac *getBigFracFromSignedInteger(int64_t integer) {
    bigInt *bigIntPart = getBigIntFromSignedInteger(integer);
    return newBigFracFromBigInt(bigIntPart, false);
}

void removeLeadingAndTrailingZeroBlocks(bigFrac *x) {
    size_t xLen = getLen(x->bigIntPart);

    //Leading Zeros
    size_t holeBlocks = xLen - x->fractionBlocks;
    size_t leadingZeroBlocks = xLen - getOccupiedBlocks(x->bigIntPart);
    x->bigIntPart->end -= min(leadingZeroBlocks, holeBlocks);

    //Trailing
    size_t trailingZeroBlocks = getTrailingZeroBlocks(x->bigIntPart);
    x->bigIntPart->start += min(trailingZeroBlocks, x->fractionBlocks);
    x->fractionBlocks -= min(trailingZeroBlocks, x->fractionBlocks);
}

bigFrac *shiftLeftBigFrac(const bigFrac *x, const size_t n) {
    bigInt *shiftedBigInt = shiftLeft(x->bigIntPart, n);
    bigFrac *res = newBigFracFromBigInt(shiftedBigInt, false);
    res->fractionBlocks = x->fractionBlocks;
    return res;
}

bigFrac *shiftRightBigFrac(const bigFrac *x, const size_t n) {
    bigInt *shiftedBigInt = shiftRight(x->bigIntPart, n);
    bigFrac *res = newBigFracFromBigInt(shiftedBigInt, false);
    res->fractionBlocks = x->fractionBlocks;
    return res;
}

// Returns -1, 0 or 1 as a is numerically less than, equal to, or greater than b
int compareBigFrac(const bigFrac *a, const bigFrac *b) {
    if (isZero(a->bigIntPart) && isZero(b->bigIntPart))
        return 0;

    if (a->bigIntPart->negative == b->bigIntPart->negative) {
        if (a->bigIntPart->negative)
            return compareBigIntArraysBigFrac(b, a);
        else
            return compareBigIntArraysBigFrac(a, b);
    }
    return a->bigIntPart->negative ? 1 : -1;
}

int compareBigIntArraysBigFrac(const bigFrac *a, const bigFrac *b) {
    if (isZero(a->bigIntPart) && isZero(b->bigIntPart)) {
        return 0;
    }

    size_t aTotalLen = getLen(a->bigIntPart);
    size_t bTotalLen = getLen(b->bigIntPart);
    size_t aHoleLen = aTotalLen - a->fractionBlocks;
    size_t bHoleLen = bTotalLen - b->fractionBlocks;

    if (aHoleLen < bHoleLen) {
        return -1;
    } else if (aHoleLen > bHoleLen) {
        return 1;
    }

    size_t longerLen = min(aTotalLen, bTotalLen);
    size_t i = 0;
    for (long j = (long) a->bigIntPart->end - 1, k = (long) b->bigIntPart->end - 1; i < longerLen; j--, k--, i++) {
        uint64_t aVal = a->bigIntPart->bigIntArray[j];
        uint64_t bVal = b->bigIntPart->bigIntArray[k];
        if (aVal < bVal)
            return -1;
        if (aVal > bVal)
            return 1;
    }
    if (a->fractionBlocks < b->fractionBlocks) {
        return -1;
    } else if (a->fractionBlocks > b->fractionBlocks) {
        return 1;
    }
    return 0;
}

bigFrac *addBigFrac(const bigFrac *x, const bigFrac *y) {
    const bigFrac *addBigger = x;
    const bigFrac *addSmaller = y;
    if (compareBigIntArraysBigFrac(x, y) == -1) {
        const bigFrac *tmp = addBigger;
        addBigger = addSmaller;
        addSmaller = tmp;
    }

    if (x->bigIntPart->negative == y->bigIntPart->negative) {
        return addBigFrac_helper(addBigger, addSmaller, addBigger->bigIntPart->negative);
    } else {
        return subBigFrac_helper(addBigger, addSmaller, addBigger->bigIntPart->negative);
    }
}

bigFrac *addBigFrac_helper(const bigFrac *x, const bigFrac *y, bool negative) {
    size_t maxLen = max(getLen(x->bigIntPart), getLen(y->bigIntPart));
    bigInt *resBigInt = newBigIntNotZeroed(maxLen + 1);

    size_t lenDif = 0;
    if (x->fractionBlocks < y->fractionBlocks) {
        lenDif = y->fractionBlocks - x->fractionBlocks;
        memcpy(resBigInt->bigIntArray, y->bigIntPart->bigIntArray, lenDif * 8);
        y->bigIntPart->start += lenDif;
    } else if (x->fractionBlocks > y->fractionBlocks) {
        lenDif = x->fractionBlocks - y->fractionBlocks;
        memcpy(resBigInt->bigIntArray, x->bigIntPart->bigIntArray, lenDif * 8);
        x->bigIntPart->start += lenDif;
    }
    resBigInt->start += lenDif;

    resBigInt->bigIntArray[resBigInt->end - 1] = 0;
    do_add_asm(x->bigIntPart, y->bigIntPart, resBigInt);
    resBigInt->negative = negative;
    resBigInt->start -= lenDif;

    if (x->fractionBlocks < y->fractionBlocks) {
        y->bigIntPart->start -= lenDif;
    } else if (x->fractionBlocks > y->fractionBlocks) {
        x->bigIntPart->start -= lenDif;
    }

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBlocks = max(x->fractionBlocks, y->fractionBlocks);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}

bigFrac *subBigFrac(const bigFrac *x, const bigFrac *y) {
    const bigFrac *subBigger = x;
    const bigFrac *subSmaller = y;
    if (compareBigIntArraysBigFrac(x, y) == -1) {
        const bigFrac *tmp = subBigger;
        subBigger = subSmaller;
        subSmaller = tmp;
    }

    if (x->bigIntPart->negative != y->bigIntPart->negative) {
        if (x == subBigger)
            return addBigFrac_helper(subBigger, subSmaller, subBigger->bigIntPart->negative);
        else
            return addBigFrac_helper(subBigger, subSmaller, subSmaller->bigIntPart->negative);
    } else {
        if (x == subBigger)
            return subBigFrac_helper(subBigger, subSmaller, subBigger->bigIntPart->negative);
        else
            return subBigFrac_helper(subBigger, subSmaller, !subBigger->bigIntPart->negative);
    }
}

bigFrac *subBigFrac_helper(const bigFrac *x, const bigFrac *y, bool negative) {
    size_t maxLen = max(getLen(x->bigIntPart), getLen(y->bigIntPart));
    bigInt *resBigInt = newBigIntNotZeroed(maxLen);

    size_t lenDif = 0;
    bigInt *xTmp = x->bigIntPart;
    if (x->fractionBlocks < y->fractionBlocks) {
        lenDif = y->fractionBlocks - x->fractionBlocks;
        xTmp = shiftLeft(x->bigIntPart, lenDif * 64);
    } else if (x->fractionBlocks > y->fractionBlocks) {
        lenDif = x->fractionBlocks - y->fractionBlocks;
        memcpy(resBigInt->bigIntArray, x->bigIntPart->bigIntArray, lenDif * 8);
        x->bigIntPart->start += lenDif;
        resBigInt->start += lenDif;
    }

    resBigInt->bigIntArray[resBigInt->end - 1] = 0;
    do_sub_asm(xTmp, y->bigIntPart, resBigInt);
    resBigInt->negative = negative;

    if (x->fractionBlocks < y->fractionBlocks) {
        freeBigInt(xTmp);
    } else if (x->fractionBlocks > y->fractionBlocks) {
        x->bigIntPart->start -= lenDif;
        resBigInt->start -= lenDif;
    }

    bigFrac *res = newBigFracFromBigInt(resBigInt, false);
    res->fractionBlocks = max(x->fractionBlocks, y->fractionBlocks);
    removeLeadingAndTrailingZeroBlocks(res);
    return res;
}
