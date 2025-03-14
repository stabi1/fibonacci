#include "bigFracIO.h"

#include "bigFracString.h"

#include <stdio.h>

//prints bigInt in Dec
void printBigFracDec(const bigFrac *x, bool exactPrecision) {
    char *tmp = bigFracToDecString(x, exactPrecision);
    printf("%s\n", tmp);
    free(tmp);
}