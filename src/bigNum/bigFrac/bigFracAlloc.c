#include "bigFracAlloc.h"

#include "../misc.h"

#include <stdio.h>
#include <stdlib.h>

bigFrac *newBigFrac(size_t len) {
    if (len == 0) {
        fprintf(stderr, "newBigFrac: len can not be zero!\n");
        exit(4);
    }

    bigFrac *res = malloc(sizeof(bigFrac));
    mallocCheck(res);
    res->bigIntPart = newBigInt(len);
    return res;
}

bigFrac *newBigFracFromBigInt(bigInt *x, bool copy) {
    bigFrac *res = malloc(sizeof(bigFrac));
    mallocCheck(res);
    if (copy) {
        res->bigIntPart = copyBigInt(x);
    } else {
        res->bigIntPart = x;
    }
    res->fractionBits = 0;
    return res;
}

void freeBigFrac(bigFrac *toDelete) {
    freeBigInt(toDelete->bigIntPart);
    free(toDelete);
}
