#include <stdio.h>
#include <string.h>
#include "bigIntAlloc.h"
#include "bigIntMethods.h"

// Global variables
pthread_key_t stack_key;
pthread_once_t stack_key_once = PTHREAD_ONCE_INIT;

BigIntStack *create_stack();

bigInt* allocBigIntStruct();

//allocates memory for a new bigInt of the given size
bigInt *newBigInt(size_t len) {
    if (len == 0) {
        fprintf(stderr, "newBigInt: len can not be zero!\n");
        exit(4);
    }
    bigInt *res = allocBigIntStruct();
    res->bigIntArray = calloc(len, sizeof(uint64_t));
    mallocCheck(res->bigIntArray);
    res->start = 0;
    res->end = len;
    res->arrayOwner = true;
    res->negative = false;
    return res;
}

//creates a new bigInt Struct with the array of another bigInt->not the owner of the array
bigInt *newBigIntStruct(size_t start, size_t end, uint64_t *bigIntArray) {
    if (start - end == 0) {
        return getZeroBigInt();
    }

    bigInt *res = allocBigIntStruct();
    res->bigIntArray = bigIntArray;
    res->start = start;
    res->end = end;
    res->arrayOwner = false;
    res->negative = false;
    return res;
}


//Frees the memory of the BigInteger
void freeBigInt(bigInt *toDelete) {
    if (toDelete->arrayOwner) { //if the bigInt is the owner free the array
        free(toDelete->bigIntArray);
    }
    if (pushBigIntStack(toDelete) == -1) {
        free(toDelete);
    }
}

//deep copies BigInt
bigInt *copyBigInt(bigInt *x) {
    size_t xLen = x->end - x->start;
    bigInt *res = newBigInt(xLen);
    memcpy(res->bigIntArray, x->bigIntArray + x->start, xLen * 8);
    res->negative = x->negative;
    return res;
}

bigInt* allocBigIntStruct() {
    bigInt *res;
    if (popBigIntStack(&res) == -1) {
        res = malloc(sizeof(bigInt));
        mallocCheck(res);
    }
    return res;
}

// Push an element onto the stack
int pushBigIntStack(bigInt *x) {
    BigIntStack *stack = get_thread_BigIntStack();
    if (stack->top >= MAX_SIZE - 1) {
        return -1; // Stack is full
    } else {
        stack->array[++(stack->top)] = x;
        return 0;
    }
}

// Pop an element from the stack
int popBigIntStack(bigInt **x) {
    BigIntStack *stack = get_thread_BigIntStack();
    if (stack->top < 0) {
        return -1; // Stack is empty
    } else {
        *x = stack->array[stack->top--];
        return 0;
    }
}


void init_stack_key() {
    pthread_key_create(&stack_key, free_BigIntStack);
}

BigIntStack *create_stack() {
    BigIntStack *stack = (BigIntStack *) malloc(sizeof(BigIntStack));
    mallocCheck(stack);
    stack->top = -1;  // Initially, the stack is empty
    return stack;
}

BigIntStack *get_thread_BigIntStack() {
    pthread_once(&stack_key_once, init_stack_key);

    BigIntStack *stack = (BigIntStack *) pthread_getspecific(stack_key);
    if (!stack) {
        stack = create_stack();
        pthread_setspecific(stack_key, stack);
    }
    return stack;
}

// Frees the stack (used for cleanup when a thread finishes)
void free_BigIntStack(void *ptr) {
    BigIntStack *stack = (BigIntStack *) ptr;
    if (stack) {
        for (int i = 0; i <= stack->top; ++i) {
            free(stack->array[i]);
        }
        free(stack);
    }
}
