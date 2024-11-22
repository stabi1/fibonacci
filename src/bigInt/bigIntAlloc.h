#ifndef FIBONACCI_BIGINTALLOC_H
#define FIBONACCI_BIGINTALLOC_H

#include <pthread.h>

#include "bigInt.h"

#define MAX_SIZE 50

typedef struct {
    bigInt *array[MAX_SIZE]; // Fixed size stack
    int top;                 // Stack pointer (-1 means empty)
} BigIntStack;

typedef struct {
    uint64_t *array[MAX_SIZE]; // Fixed size stack
    int top;                 // Stack pointer (-1 means empty)
} BigIntArrayStack_2KB;

int pushBigIntStack(bigInt *x);

int popBigIntStack(bigInt **x);

int popBigIntArrayStack(uint64_t **x, size_t len, size_t* completeLen);

int pushBigIntArrayStack(uint64_t *x, size_t len);

BigIntStack *get_thread_BigIntStack();

BigIntArrayStack_2KB *get_thread_bigIntArrayStack_2KB();

void free_BigIntStack(void *ptr);

uint64_t *allocBigIntArray(size_t len, size_t* completeLen, bool setZero);

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

#endif //FIBONACCI_BIGINTALLOC_H
