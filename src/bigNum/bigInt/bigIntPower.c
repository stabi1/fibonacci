#include "bigIntPower.h"

bigInt *powBigInt(const bigInt *base, uint64_t exponent) {
    if (isZero(base)) return getBigIntFromUnsignedInteger(1);

    bigInt *y = getBigIntFromUnsignedInteger(1);
    bigInt *expTmp = copyBigInt(base);

    while (exponent > 1) {
        if (exponent % 2 != 0) {
            bigInt *yTmp = mul(expTmp, y);
            freeBigInt(y);
            y = yTmp;
            exponent = exponent - 1;
        }
        bigInt *tmp = mul(expTmp, expTmp);
        freeBigInt(expTmp);
        expTmp = tmp;

        exponent = exponent / 2;
    }
    bigInt* res =  mul(expTmp, y);
    freeBigInt(expTmp);
    freeBigInt(y);
    return res;
}