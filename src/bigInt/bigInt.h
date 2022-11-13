#ifndef IMPLEMENTATION_BETTERBIGINT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define IMPLEMENTATION_BETTERBIGINT_H

typedef struct BigIntTag { //The datastructure that represents a large number
    size_t start; // in qWords(64bit)
    size_t end;
    bool arrayOwner; //if the bigInt is the "owner" of the array. If false the array is not freed with the bigInt
    uint64_t *bigIntArray;
    bool negative;
} bigInt;

bigInt *newBigInt(size_t len);
void freeBigInt(bigInt *toDelete);

bigInt *smartSub(bigInt *x, bigInt *y);
bigInt *smartAdd(bigInt *x, bigInt *y);
bigInt *shiftLeft(bigInt *x, size_t n);
bigInt *shiftRight(bigInt *x, size_t n);

bigInt *hexStringToBigInt(char hex[]);
char *bigIntToDecString(bigInt *x);
char *bigIntToHexString(bigInt *x);

bigInt *multiplyToomCook3(bigInt *a, bigInt *b);
bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth);

#endif //IMPLEMENTATION_BETTERBIGINT_H
