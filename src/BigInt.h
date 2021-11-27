#ifndef IMPLEMENTATION_BETTERBIGINT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdint-gcc.h>

#define IMPLEMENTATION_BETTERBIGINT_H
typedef struct BigIntTag { //The datastructure that represents a large number
    size_t start; // in qWords(64bit)
    size_t end;
    bool arrayOwner; //if the bigInt is the "owner" of the array. If false the array is not freed with the bigInt
    uint64_t *bigIntArray;
    bool negative;
} bigInt;

bigInt *newBigInt(size_t len);

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

void freeBigInt(bigInt *toDelete);

void exception();

size_t getOccupiedFields(bigInt *x);

char *bigIntToStr(bigInt *x);

void printBigInt(bigInt *x);

bigInt *hexStringToBigInt(char hex[]);

char *bigIntToDecString(bigInt *x);

char *bigIntToHexString(bigInt *x);

bigInt *smartSub(bigInt *x, bigInt *y);

bigInt *getLowerFrom(bigInt *x, size_t n);

bigInt *getUpperFrom(bigInt *x, size_t n);

void getToomSlice(bigInt *x, size_t lowerSize, size_t upperSize, size_t fullSize, bigInt* erg[]);

bigInt *smartAdd(bigInt *x, bigInt *y);

bigInt *exactDivideBy3(bigInt *x);

#endif //IMPLEMENTATION_BETTERBIGINT_H
