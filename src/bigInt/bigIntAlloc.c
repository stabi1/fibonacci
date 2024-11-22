#include <stdio.h>
#include <string.h>
#include "bigIntAlloc.h"
#include "bigIntMethods.h"

// Global variables
pthread_key_t bigIntStruct_stack_key;
pthread_once_t bigIntStruct_stack_key_once = PTHREAD_ONCE_INIT;

pthread_key_t bigIntArrayStack_2KB_stack_key;
pthread_once_t bigIntArrayStack_2KB_stack_key_once = PTHREAD_ONCE_INIT;

BigIntStack *create_stack();

bigInt *allocBigIntStruct();

uint64_t *allocBigIntArray(size_t len, size_t* completeLen);

//allocates memory for a new bigInt of the given size
bigInt *newBigInt(size_t len) {
    if (len == 0) {
        fprintf(stderr, "newBigInt: len can not be zero!\n");
        exit(4);
    }
    bigInt *res = allocBigIntStruct();
    size_t completeLen;
    res->bigIntArray = allocBigIntArray(len, &completeLen);
    res->start = 0;
    res->end = len;
    res->arrayOwner = true;
    res->negative = false;
    res->completeLength = completeLen;
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
        if (pushBigIntArrayStack(toDelete->bigIntArray, toDelete->completeLength) == -1) {
            free(toDelete->bigIntArray);
        }
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

bigInt *allocBigIntStruct() {
    bigInt *res;

    if (popBigIntStack(&res) == -1) {
        res = malloc(sizeof(bigInt));
        mallocCheck(res);
    }
    return res;
}

uint64_t *allocBigIntArray(size_t len, size_t* completeLen) {
    uint64_t *res;
    if (popBigIntArrayStack(&res, len, completeLen) == -1) {
        res = calloc(len, sizeof(uint64_t));
        mallocCheck(res);
        *completeLen = len;
    } else {
        memset(res, 0, len*8);
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

int pushBigIntArrayStack(uint64_t *x, size_t len) {
    BigIntArrayStack_2KB *stack;
    if (len < 5000 && len >= 1000) {
        stack = get_thread_bigIntArrayStack_2KB();
    } else {
        return -1;
    }

    if (stack->top >= MAX_SIZE - 1) {
        return -1; // Stack is full
    } else {
        stack->array[++(stack->top)] = x;
        return 0;
    }
}

int popBigIntArrayStack(uint64_t **x, size_t len, size_t* completeLen) {
    BigIntArrayStack_2KB *stack;
    if (len <= 1000) {
        *completeLen = 1000;
        stack = get_thread_bigIntArrayStack_2KB();
    } else {
        return -1;
    }

    if (stack->top < 0) {
        return -1; // Stack is empty
    } else {
        *x = stack->array[stack->top--];
        return 0;
    }
}

BigIntStack *create_stack() {
    BigIntStack *stack = (BigIntStack *) malloc(sizeof(BigIntStack));
    mallocCheck(stack);
    stack->top = -1;  // Initially, the stack is empty
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


void init_stack_key_bigIntStruct() {
    pthread_key_create(&bigIntStruct_stack_key, free_BigIntStack);
}

void init_stack_key_bigIntArrayStack_2KB() {
    pthread_key_create(&bigIntArrayStack_2KB_stack_key, free_BigIntStack);
}

BigIntStack *get_thread_BigIntStack() {
    pthread_once(&bigIntStruct_stack_key_once, init_stack_key_bigIntStruct);

    BigIntStack *stack = (BigIntStack *) pthread_getspecific(bigIntStruct_stack_key);
    if (!stack) {
        stack = create_stack();
        pthread_setspecific(bigIntStruct_stack_key, stack);
    }
    return stack;
}

BigIntArrayStack_2KB *get_thread_bigIntArrayStack_2KB() {
    pthread_once(&bigIntArrayStack_2KB_stack_key_once, init_stack_key_bigIntArrayStack_2KB);

    BigIntArrayStack_2KB *stack = (BigIntArrayStack_2KB *) pthread_getspecific(bigIntArrayStack_2KB_stack_key);
    if (!stack) {
        stack = (BigIntArrayStack_2KB *) create_stack();
        pthread_setspecific(bigIntArrayStack_2KB_stack_key, stack);
        for(int i = 0; i<10; i++) {
            uint64_t *array = malloc(sizeof(uint64_t) * 1100);
            mallocCheck(array);
            stack->array[++(stack->top)] = array;
        }
    }
    return stack;
}

