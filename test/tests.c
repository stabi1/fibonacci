#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include "../src/main.h"
#include "../src/mulAsm.h"
//#include "../src/mul.h"

char *randomHex(uint64_t n);

void test() {
    findBestValues();
    /*uint64_t n = 40000;
    bigInt *res1 = fibExpFastDoubling(n);
    bigInt *res2 = fibExpFastDoublingMultiThread(n);
    if (!compareBigInts(res1, res2)) {
        printf("Fault\n");
        return;
    }
    printf("Correct; Length of result in 8 byte: %lu\n", res1->end - res1->start);
    freeBigInt(res1);
    freeBigInt(res2);*/
}

void findBestValues() {
    size_t nFast;
    size_t kFast;
    double bestTime = DBL_MAX;
    uint64_t fibN = 10000000;
    printf("Beginning testing\n");
    for(size_t n = 20; n<1000; n += 5) {
        for(size_t k = n + 2; k<n + 5000; k += 30) {
            printf("\rTesting %lu %lu", n, k);
            //naivMulFaster = n;
            //karatsubaFaster = k;
            struct timespec start;
            clock_gettime(CLOCK_MONOTONIC, &start);

            bigInt *res = fibExpFastDoubling(fibN);

            struct timespec end;
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
            if(time < bestTime) {
                bestTime = time;
                nFast = n;
                kFast = k;
                printf("\nFound new best! n: %lu, k: %lu, time: %f\n", nFast, kFast, bestTime);
            }
            freeBigInt(res);
        }
    }
}

void benchMark() {
    size_t n = 1; //iterations

    bigInt *test1 = hexStringToBigInt(randomHex(1000000));
    bigInt *test2 = hexStringToBigInt(randomHex(1000000));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        bigInt *res1 = naiveMul_Asm(test1, test2);
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
        bigInt *res2 = naiveMul_Asm(test1, test2);
        freeBigInt(res2);
    }
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    freeBigInt(test1);
    freeBigInt(test2);
}

void bruteForceDebug(bool multiThread) {
    if (multiThread) {
        printf("Bruteforce debug for multi-thread and single-thread\n");
    } else {
        printf("Bruteforce debug for single-thread\n");
    }
    //edge cases
    bigInt *res = newBigInt(1);
    res->bigIntArray[0] = 0;
    bigInt *res2 = fibExpFastDoubling(0);
    bigInt *res3 = NULL;
    if (multiThread) {
        res3 = fibExpFastDoublingMultiThread(0);
    }
    if (!compareBigInts(res, res2)) {
        printf("Failed at 0 for single-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }

    if (multiThread && !compareBigInts(res, res3)) {
        printf("Failed at 0 for multi-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res2);
    if (multiThread) {
        freeBigInt(res3);
    }
    res->bigIntArray[0] = 1;
    res2 = fibExpFastDoubling(1);
    if (multiThread) {
        res3 = fibExpFastDoublingMultiThread(1);
    }
    if (!compareBigInts(res, res2)) {
        printf("Failed at 1 for single-thread\n");
        printBigInt(res);
        printBigInt(res2);
        freeBigInt(res);
        freeBigInt(res2);
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }
    if (multiThread && !compareBigInts(res, res3)) {
        printf("Failed at 1 for multi-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res);
    freeBigInt(res2);
    if (multiThread) {
        freeBigInt(res3);
    }

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
                printf("Failed at %lu for single-thread\n", i + 1);

                printBigInt(fibMinus1);
                printBigInt(res2);

                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                if (multiThread) {
                    freeBigInt(res3);
                }
                return;
            }
            if (multiThread) {
                res3 = fibExpFastDoublingMultiThread(i + 1);
                if (!compareBigInts(fibMinus1, res3)) {
                    printf("Failed at %lu for multi-thread\n", i + 1);
                    freeBigInt(fibMinus2);
                    freeBigInt(fibMinus1);
                    freeBigInt(res2);
                    freeBigInt(res3);
                    return;
                }
            }
            freeBigInt(res2);
            if (multiThread) {
                freeBigInt(res3);
            }
            i++;
        }
    }
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
