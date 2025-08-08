#ifndef FIBONACCI_BIGINTALLOC_H
#define FIBONACCI_BIGINTALLOC_H

#include <pthread.h>

#include "bigInt.h"

#define MAX_SIZE_BIGINT_STACK 50
#define MAX_SIZE_ARRAY_STACK 30
#define MAX_SIZE_ARRAY_STACK_100KB 10

typedef struct {
    bigInt **array; // Fixed size stack
    int top;       // Stack pointer (-1 means empty)
    long size;
} BigIntStack;

typedef struct {
    uint64_t **array; // Fixed size stack
    int top;         // Stack pointer (-1 means empty)
    long size;
} BigIntArrayStack;

int pushBigIntStack(bigInt *x);

int popBigIntStack(bigInt **x);

int popBigIntArrayStack(uint64_t **x, size_t len, size_t *completeLen);

int pushBigIntArrayStack(uint64_t *x, size_t len);

BigIntStack *get_thread_BigIntStack();

BigIntArrayStack *get_thread_bigIntArrayStack_1KB();

BigIntArrayStack *get_thread_bigIntArrayStack_10KB();

BigIntArrayStack *get_thread_bigIntArrayStack_100KB();

BigIntStack *create_stack(long size);

void free_Stack(void *ptr);

uint64_t *allocBigIntArray(size_t len, size_t *completeLen, bool setZero);

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

#endif //FIBONACCI_BIGINTALLOC_H
