#ifndef FIBONACCI_BIGFRACALLOC_H
#define FIBONACCI_BIGFRACALLOC_H

#include <pthread.h>

#include "bigFrac.h"

#define MAX_SIZE_BIG_FRAC_STACK 50

typedef struct {
    bigFrac **array; // Fixed size stack
    int top;       // Stack pointer (-1 means empty)
    long size;
} BigFracStack;

bigFrac *allocBigFracStruct();

int pushBigFracStack(bigFrac *x);

int popBigFracStack(bigFrac **x);

BigFracStack *get_thread_BigFracStack();


#endif //FIBONACCI_BIGFRACALLOC_H
