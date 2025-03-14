#include "bigFracIO.h"

#include <stdio.h>

//prints bigInt in Hex
void printBigFracHex(const bigFrac *x) {
    char *tmp = bigFracToHexString(x);
    printf("%s\n", tmp);
    free(tmp);
}

//prints bigInt in Dec
void printBigFracDec(const bigFrac *x, bool exactPrecision) {
    char *tmp = bigFracToDecString(x, exactPrecision);
    printf("%s\n", tmp);
    free(tmp);
}