#ifndef FIBONACCI_BIGINTALLOC_H
#define FIBONACCI_BIGINTALLOC_H

#include <pthread.h>

#include "bigInt.h"

#define MAX_SIZE 30

typedef struct {
    bigInt *array[MAX_SIZE]; // Fixed size stack
    int top;                 // Stack pointer (-1 means empty)
} BigIntStack;

int pushBigIntStack(bigInt *x);

int popBigIntStack(bigInt **x);

BigIntStack *get_thread_BigIntStack();

void free_BigIntStack(void *ptr);

bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray);

#endif //FIBONACCI_BIGINTALLOC_H
