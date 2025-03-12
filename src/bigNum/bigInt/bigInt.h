#ifndef IMPLEMENTATION_BETTERBIGINT_H
#define IMPLEMENTATION_BETTERBIGINT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>

#include "../config.h"

typedef struct BigIntTag { //The datastructure that represents a large number
    size_t start; // in qWords(64bit)
    size_t end;
    bool arrayOwner; //if the bigInt is the "owner" of the array. If false the array is not freed with the bigInt
    uint64_t *bigIntArray;
    bool negative;
    size_t completeLength; // the complete length of the bigIntArray, is used when a bigInt struct is recycled
} bigInt;

// create/delete
bigInt *newBigInt(size_t len);

bigInt *newBigIntNotZeroed(size_t len);

void freeBigInt(bigInt *toDelete);

bigInt *copyBigInt(const bigInt *x);

bigInt *getZeroBigInt();

bigInt* getBigIntFromUnsignedInteger(uint64_t integer);

bigInt* getBigIntFromSignedInteger(int64_t integer);

//compare
int compareBigInt(const bigInt *a, const bigInt *b);

bool isZero(const bigInt *x);

// I/O methods
void printBigIntHex(const bigInt *x);

void printBigIntDec(const bigInt *x);

void writeBigIntHexToFile(const bigInt *x, const char *path);

void writeBigIntDecToFile(bigInt *x, const char *path, bool doFree);

bigInt *readBigIntHexFromFile(const char *path);

bigInt *readBigIntDecFromFile(const char *path);

char *storeBigIntInSwap(bigInt *x);

bigInt *loadBigIntFromSwap(bigInt *x, char *filename);

// calc methods
bigInt *sub(const bigInt *x, const bigInt *y);

bigInt *add(const bigInt *x, const bigInt *y);

bigInt *shiftLeft(const bigInt *x, size_t n);

bigInt *shiftRight(const bigInt *x, size_t n);

bigInt *mul(const bigInt *x, const bigInt *y);

bigInt *mulSingleThread(const bigInt *x, const bigInt *y);

bigInt *mulParallel(const bigInt *x, const bigInt *y, size_t depth);

bigInt *divide(const bigInt *dividend, const bigInt *divisor);

bigInt *divideMod(const bigInt *dividend, const bigInt *divisor, bigInt **reminder);

// String methods
bigInt *hexStringToBigInt(const char *hex);

bigInt *decStringToBigInt(const char *dec);

char *bigIntToDecString(bigInt *x, bool doFree);

char *bigIntToHexString(const bigInt *x);

//misc
size_t bitLength(const bigInt *x);

size_t getLen(const bigInt *x);

void negateBigInt(bigInt* x);

//misc utility
void mallocCheck(void *p);

size_t custom_lzcnt(uint64_t n);

char *readFile(const char *path);

int writeFile(const char *path, const char *string, bool append);

char *getCurrentDateTime();

void handleSignals(int sig, siginfo_t *info, void *context);

//higherFunctions
bigInt *fibonacci(uint64_t n);


#endif //IMPLEMENTATION_BETTERBIGINT_H
