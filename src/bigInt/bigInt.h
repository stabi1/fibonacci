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

// I/O methods
void printBigIntHex(bigInt *x);

void printBigIntDec(bigInt *x);

void writeBigIntHexToFile(bigInt *x, char *path);

void writeBigIntDecToFile(bigInt *x, char *path);

bigInt *readBigIntHexFromFile(char *path);

bigInt *readBigIntDecFromFile(char *path);

// calc methods
bigInt *sub(bigInt *x, bigInt *y);

bigInt *add(bigInt *x, bigInt *y);

bigInt *shiftLeft(bigInt *x, size_t n);

bigInt *shiftRight(bigInt *x, size_t n);

bigInt *mul(bigInt *x, bigInt *y);

bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth); //TODO remove

bigInt *divide(bigInt *dividend, bigInt *divisor);

bigInt *divideMod(bigInt *dividend, bigInt *divisor, bigInt **reminder);

// String methods
bigInt *hexStringToBigInt(const char *hex);

bigInt *decStringToBigInt(const char *dec);

char *bigIntToDecString(bigInt *x);

char *bigIntToHexString(bigInt *x);

//misc
size_t bitLength(bigInt *x);

size_t getLen(bigInt *x);

//misc utility
size_t custom_lzcnt(uint64_t n);

char *readFile(const char *path);

int writeFile(const char *path, const char *string, bool append);

//higherFunctions
bigInt *fibonacci(uint64_t n);


#endif //IMPLEMENTATION_BETTERBIGINT_H
