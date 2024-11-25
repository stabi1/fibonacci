#include <stdio.h>
#include <string.h>
#include "bigIntAlloc.h"
#include "bigIntMethods.h"

// Global variables
pthread_key_t bigIntStruct_stack_key;
pthread_once_t bigIntStruct_stack_key_once = PTHREAD_ONCE_INIT;

pthread_key_t bigIntArrayStack_1KB_stack_key;
pthread_once_t bigIntArrayStack_1KB_stack_key_once = PTHREAD_ONCE_INIT;

pthread_key_t bigIntArrayStack_10KB_stack_key;
pthread_once_t bigIntArrayStack_10KB_stack_key_once = PTHREAD_ONCE_INIT;

pthread_key_t bigIntArrayStack_100KB_stack_key;
pthread_once_t bigIntArrayStack_100KB_stack_key_once = PTHREAD_ONCE_INIT;

BigIntStack *create_stack();

bigInt *allocBigIntStruct();

bigInt *newBigIntHelper(size_t len, bool setZero);

//allocates memory for a new bigInt of the given size
bigInt *newBigInt(size_t len) {
    return newBigIntHelper(len, true);
}

bigInt *newBigIntNotZeroed(size_t len) {
    return newBigIntHelper(len, false);
}

bigInt *newBigIntHelper(size_t len, bool setZero) {
    if (len == 0) {
        fprintf(stderr, "newBigInt: len can not be zero!\n");
        exit(4);
    }
    bigInt *res = allocBigIntStruct();
    size_t completeLen;
    res->bigIntArray = allocBigIntArray(len, &completeLen, setZero);
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
    bigInt *res = newBigIntNotZeroed(xLen);
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

uint64_t *allocBigIntArray(size_t len, size_t *completeLen, bool setZero) {
    uint64_t *res;
    if (popBigIntArrayStack(&res, len, completeLen) == -1) {
        if (setZero)
            res = calloc(len, sizeof(uint64_t));
        else
            res = malloc(sizeof(uint64_t) * len);
        mallocCheck(res);
        *completeLen = len;
    } else {
        if (setZero)
            memset(res, 0, len * 8);
    }
    return res;
}

// Push an element onto the stack
int pushBigIntStack(bigInt *x) {
    if (global_config.deactivateCaches) return -1;
    BigIntStack *stack = get_thread_BigIntStack();
    if (stack->top >= stack->size - 1) {
        return -1; // Stack is full
    } else {
        stack->array[++(stack->top)] = x;
        return 0;
    }
}

// Pop an element from the stack
int popBigIntStack(bigInt **x) {
    if (global_config.deactivateCaches) return -1;
    BigIntStack *stack = get_thread_BigIntStack();
    if (stack->top < 0) {
        return -1; // Stack is empty
    } else {
        *x = stack->array[stack->top--];
        return 0;
    }
}

int pushBigIntArrayStack(uint64_t *x, size_t len) {
    if (global_config.deactivateCaches) return -1;
    BigIntArrayStack *stack;
    if (len < 125) {
        return -1;
    } else if (len < 1250) {
        stack = get_thread_bigIntArrayStack_1KB();
    } else if (len < 12500) {
        stack = get_thread_bigIntArrayStack_10KB();
    } else if (len < 13000) {
        stack = get_thread_bigIntArrayStack_100KB();
    } else {
        return -1;
    }

    if (stack->top >= stack->size - 1) {
        return -1; // Stack is full
    } else {
        stack->array[++(stack->top)] = x;
        return 0;
    }
}

int popBigIntArrayStack(uint64_t **x, size_t len, size_t *completeLen) {
    if (global_config.deactivateCaches) return -1;
    BigIntArrayStack *stack;
    if (len <= 125) {
        *completeLen = 125;
        stack = get_thread_bigIntArrayStack_1KB();
    } else if (len <= 1250) {
        *completeLen = 1250;
        stack = get_thread_bigIntArrayStack_10KB();
    } else if (len <= 12500) {
        *completeLen = 12500;
        stack = get_thread_bigIntArrayStack_100KB();
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

BigIntStack *create_stack(size_t size) {
    BigIntStack *stack = (BigIntStack *) malloc(sizeof(BigIntStack));
    mallocCheck(stack);
    stack->top = -1;  // Initially, the stack is empty
    stack->array = malloc(size * 8);
    mallocCheck(stack->array);
    stack->size = size;
    return stack;
}

// Frees the stack (used for cleanup when a thread finishes)
void free_BigIntStack(void *ptr) {
    BigIntStack *stack = (BigIntStack *) ptr;
    if (stack) {
        for (int i = 0; i <= stack->top; ++i) {
            free(stack->array[i]);
        }
        free(stack->array);
        free(stack);
    }
}


void init_stack_key_bigIntStruct() {
    pthread_key_create(&bigIntStruct_stack_key, free_BigIntStack);
}

void init_stack_key_bigIntArrayStack_1KB() {
    pthread_key_create(&bigIntArrayStack_1KB_stack_key, free_BigIntStack);
}

void init_stack_key_bigIntArrayStack_10KB() {
    pthread_key_create(&bigIntArrayStack_10KB_stack_key, free_BigIntStack);
}

void init_stack_key_bigIntArrayStack_100KB() {
    pthread_key_create(&bigIntArrayStack_100KB_stack_key, free_BigIntStack);
}

BigIntStack *get_thread_BigIntStack() {
    pthread_once(&bigIntStruct_stack_key_once, init_stack_key_bigIntStruct);

    BigIntStack *stack = (BigIntStack *) pthread_getspecific(bigIntStruct_stack_key);
    if (!stack) {
        stack = create_stack(MAX_SIZE_BIGINT_STACK);
        pthread_setspecific(bigIntStruct_stack_key, stack);
    }
    return stack;
}

BigIntArrayStack *get_thread_bigIntArrayStack_1KB() {
    pthread_once(&bigIntArrayStack_1KB_stack_key_once, init_stack_key_bigIntArrayStack_1KB);

    BigIntArrayStack *stack = pthread_getspecific(bigIntArrayStack_1KB_stack_key);
    if (!stack) {
        stack = (BigIntArrayStack *) create_stack(MAX_SIZE_ARRAY_STACK);
        pthread_setspecific(bigIntArrayStack_1KB_stack_key, stack);
        for (int i = 0; i < stack->size / 3; i++) {
            uint64_t *array = malloc(sizeof(uint64_t) * 127);
            mallocCheck(array);
            stack->array[++(stack->top)] = array;
        }
    }
    return stack;
}

BigIntArrayStack *get_thread_bigIntArrayStack_10KB() {
    pthread_once(&bigIntArrayStack_10KB_stack_key_once, init_stack_key_bigIntArrayStack_10KB);

    BigIntArrayStack *stack = pthread_getspecific(bigIntArrayStack_10KB_stack_key);
    if (!stack) {
        stack = (BigIntArrayStack *) create_stack(MAX_SIZE_ARRAY_STACK);
        pthread_setspecific(bigIntArrayStack_10KB_stack_key, stack);
        for (int i = 0; i < stack->size / 3; i++) {
            uint64_t *array = malloc(sizeof(uint64_t) * 1270);
            mallocCheck(array);
            stack->array[++(stack->top)] = array;
        }
    }
    return stack;
}

BigIntArrayStack *get_thread_bigIntArrayStack_100KB() {
    pthread_once(&bigIntArrayStack_100KB_stack_key_once, init_stack_key_bigIntArrayStack_100KB);

    BigIntArrayStack *stack = pthread_getspecific(bigIntArrayStack_100KB_stack_key);
    if (!stack) {
        stack = (BigIntArrayStack *) create_stack(MAX_SIZE_ARRAY_STACK_100KB);
        pthread_setspecific(bigIntArrayStack_100KB_stack_key, stack);
        for (int i = 0; i < (stack->size / 3) * 2; i++) {
            uint64_t *array = malloc(sizeof(uint64_t) * 12700);
            mallocCheck(array);
            stack->array[++(stack->top)] = array;
        }
    }
    return stack;
}

