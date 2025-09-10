#include "bigFracAlloc.h"

#include <stdio.h>
#include <stdlib.h>

#include "../misc.h"
#include "../bigInt/bigIntAlloc.h"


pthread_key_t bigFracStruct_stack_key;
pthread_once_t bigFracStruct_stack_key_once = PTHREAD_ONCE_INIT;

bigFrac *newBigFrac(size_t len) {
    if (len == 0) {
        fprintf(stderr, "newBigFrac: len can not be zero!\n");
        exit(4);
    }

    bigFrac *res = allocBigFracStruct();
    res->bigIntPart = newBigInt(len);
    return res;
}

bigFrac *newBigFracFromBigInt(bigInt *x, bool copy) {
    bigFrac *res = allocBigFracStruct();
    if (copy) {
        res->bigIntPart = copyBigInt(x);
    } else {
        res->bigIntPart = x;
    }
    res->fractionBlocks = 0;
    return res;
}

void freeBigFrac(bigFrac *toDelete) {
    freeBigInt(toDelete->bigIntPart);

    if (pushBigFracStack(toDelete) == -1) {
        free(toDelete);
    }
}


bigFrac *allocBigFracStruct() {
    bigFrac *res;

    if (popBigFracStack(&res) == -1) {
        res = malloc(sizeof(bigFrac));
        mallocCheck(res);
    }
    return res;
}

// Push an element onto the stack
int pushBigFracStack(bigFrac *x) {
    if (global_config.deactivateCaches) { return -1; }
    BigFracStack *stack = get_thread_BigFracStack();
    if (stack->top >= stack->size - 1) {
        return -1; // Stack is full
    } else {
        stack->array[++(stack->top)] = x;
        return 0;
    }
}

// Pop an element from the stack
int popBigFracStack(bigFrac **x) {
    if (global_config.deactivateCaches) { return -1; }
    BigFracStack *stack = get_thread_BigFracStack();
    if (stack->top < 0) {
        return -1; // Stack is empty
    } else {
        *x = stack->array[stack->top--];
        return 0;
    }
}

void init_stack_key_bigFracStack() {
    if (pthread_key_create(&bigFracStruct_stack_key, free_Stack) != 0) {
        perror("Error creating stack key");
        exit(EXIT_FAILURE);
    }
}

BigFracStack *get_thread_BigFracStack() {
    static _Thread_local BigFracStack *cached_stack = NULL;
    if (!cached_stack) {
        if (pthread_once(&bigFracStruct_stack_key_once, init_stack_key_bigFracStack) != 0) {
            perror("Error in pthread_once");
            exit(EXIT_FAILURE);
        }


        BigFracStack *stack = (BigFracStack *) pthread_getspecific(bigFracStruct_stack_key);
        if (!stack) {
            stack = ( BigFracStack *) create_stack(MAX_SIZE_BIG_FRAC_STACK);
            if (pthread_setspecific(bigFracStruct_stack_key, stack) != 0) {
                perror("Error in pthread_setspecific");
                exit(EXIT_FAILURE);
            }
        }
        cached_stack = stack;
    }

    return cached_stack;
}
