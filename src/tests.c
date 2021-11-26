#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "tests.h"
#include "main.h"
#include "mulAsm.h"
#include "BigIntAsm.h"

char *randomHex(uint64_t n);

void test() {
    uint64_t n = 18428;
    bigInt* res1 = fibExpFastDoubling(n);
    bigInt* res2 = fibExpFastDoublingMultiThread(n);
    if(!compareBigInts(res1, res2)){
        printf("Fault\n");
        return;
    }
    printf("Correct; Length of result in 8 byte: %lu\n", res1->end - res1->start);
    freeBigInt(res1);
    freeBigInt(res2);
}

void benchMark() {
    size_t n = 1; //wiederholungen

    bigInt *test1 = hexStringToBigInt(randomHex(1000000));
    bigInt *test2 = hexStringToBigInt(randomHex(1000000));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        bigInt *res1 = naivMul_AsmVergleich(test1, test2);
        freeBigInt(res1);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);

    //code2
    struct timespec start2;
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for (size_t i = 0; i < n; i++) {
        bigInt *res2 = naivMul_AsmArbeit(test1, test2);
        freeBigInt(res2);
    }
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    freeBigInt(test1);
    freeBigInt(test2);
}

void bruteForceDebug() {
    //edge cases
    bigInt *res = newBigInt(1);
    res->bigIntArray[0] = 0;
    bigInt *res2 = fibExpFastDoubling(0);
    bigInt *res3 = fibExpFastDoublingMultiThread(0);
    if (!compareBigInts(res, res2)) {
        printf("Failed at 0 for normal\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    if (!compareBigInts(res, res3)) {
        printf("Failed at 0 for Multithread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res2);
    freeBigInt(res3);
    res->bigIntArray[0] = 1;
    res2 = fibExpFastDoubling(1);
    res3 = fibExpFastDoublingMultiThread(1);
    if (!compareBigInts(res, res2)) {
        printf("Failed at 1 for normal\n");
        printBigInt(res);
        printBigInt(res2);
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    if (!compareBigInts(res, res3)) {
        printf("Failed at 1 for Multithread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res);
    freeBigInt(res2);
    freeBigInt(res3);

    while (true) {
        bigInt *fibMinus2 = newBigInt(1);
        bigInt *fibMinus1 = newBigInt(1);
        bigInt *fib;
        fibMinus2->bigIntArray[0] = 0;
        fibMinus1->bigIntArray[0] = 1;

        size_t i = 1;
        while (i < 0xffffffffffffffff) {
            printf("\rTesting %luth fibonacci number", i);
            fib = smartAdd(fibMinus1, fibMinus2);
            freeBigInt(fibMinus2);
            fibMinus2 = fibMinus1;
            fibMinus1 = fib;
            res2 = fibExpFastDoubling(i + 1);
            if (!compareBigInts(fibMinus1, res2)) {
                printf("Failed at %lu for normal\n", i + 1);

                printBigInt(fibMinus1);
                printBigInt(res2);

                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                freeBigInt(res3);
                return;
            }
            res3 = fibExpFastDoublingMultiThread(i + 1);
            if (!compareBigInts(fibMinus1, res3)) {
                printf("Failed at %lu for multithread\n", i + 1);
                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                freeBigInt(res3);
                return;
            }
            freeBigInt(res2);
            freeBigInt(res3);
            i++;
        }
    }
}

void benchMarkAdd() {
    size_t n = 5000; //wiederholungen

    bigInt *test1 = hexStringToBigInt(randomHex(10000000));
    bigInt *test2 = hexStringToBigInt(randomHex(10000000));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        //bigInt *res1 = add_AsmAlt(test1, test2, false);
        //freeBigInt(res1);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);

    //code2
    struct timespec start2;
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for (size_t i = 0; i < n; i++) {
        bigInt *res2 = add_Asm(test1, test2, false);
        freeBigInt(res2);
    }
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    freeBigInt(test1);
    freeBigInt(test2);
}

bool compareBigInts(bigInt *x, bigInt *y) {
    if (x->start != y->start && x->end != y->end && x->negative != y->negative) {
        return false;
    }
    for (size_t i = x->start; i < x->end; i++) {
        if (x->bigIntArray[i] != y->bigIntArray[i]) {
            return false;
        }
    }
    return true;
}

char *randomHex(uint64_t n) {
    char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char *str = malloc(n + 1);
    uint64_t i = 0;
    for (; i < n; ++i) {
        str[i] = hex[random() % 16];
    }
    str[i] = '\0';
    return str;
}
