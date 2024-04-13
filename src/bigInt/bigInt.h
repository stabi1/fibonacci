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

// create/delete
bigInt *newBigInt(size_t len);

void freeBigInt(bigInt *toDelete);

bigInt *copyBigInt(bigInt *x);

//compare
int compareBigInt(bigInt *a, bigInt *b);

//print
void printBigIntHex(bigInt *x);

void printBigIntDec(bigInt *x);

// calc methods
bigInt *smartSub(bigInt *x, bigInt *y);

bigInt *smartAdd(bigInt *x, bigInt *y);

bigInt *shiftLeft(bigInt *x, size_t n);

bigInt *shiftRight(bigInt *x, size_t n);

bigInt *multiplyToomCook3(bigInt *a, bigInt *b);

bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth);

bigInt *divide(bigInt *dividend, bigInt *divisor);

bigInt *divideMod(bigInt *dividend, bigInt *divisor, bigInt **reminder);

// String methods
bigInt *hexStringToBigInt(char hex[]);

char *bigIntToDecString(bigInt *x);

char *bigIntToHexString(bigInt *x);

//misc
size_t custom_lzcnt(uint64_t n);


#endif //IMPLEMENTATION_BETTERBIGINT_H
