#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <unistd.h>

#include "../util.h"

#include "../bigNum/bigInt/bigIntString.h"
#include "../bigNum/bigInt/bigIntDiv.h"
#include "../bigNum/bigInt/bigIntHigherFunctions.h"

#include "../bigNum/bigFrac/bigFracString.h"
#include "../bigNum/bigFrac/bigFrac.h"
#include "../bigNum/bigInt/SSA/ssa.h"
#include "../bigNum/bigInt/SSA/ssaHelper.h"
#include "../bigNum/bigInt/NTT/ntt.h"

char *randomHex(uint64_t n);

void testDivision();

void testTmp();

void testBenchMark();

void testSSA() {

    char* c1 = randomHex(64*16);
    char* c2 = randomHex(64*16);
    bigInt *tmp1 = hexStringToBigInt(c1);
    bigInt *tmp2 = hexStringToBigInt(c2);
    //printBigIntHex(tmp1);
    //printBigIntHex(tmp2);
    free(c1);
    free(c2);
    //bigInt *tmp1 = getBigIntFromSignedInteger(11830);
    //bigInt *tmp2 = getBigIntFromSignedInteger(8955);

    //printBigIntBinary(tmp1);
    //printBigIntBinary(tmp2);

    bigInt *res1 = mulSingleThread(tmp1, tmp2);
    bigInt *res2 = ntt_mul(tmp1, tmp2);

    printf("\n--------------------------------------------\n");
    if (compareBigInt(res1, res2) != 0) {
        printBigIntHex(res1);
        printBigIntHex(res2);
        printf("NOT EQUAL\n");
    } else {
        printf("Equal\n");
    }
    freeBigInt(tmp1);
    freeBigInt(tmp2);
    freeBigInt(res1);
    freeBigInt(res2);

}

void customTest() {
    // testSSA();
    // testTmp();
    testBenchMark();
}

void testTmp() {
    char* c1 = randomHex(5000000*16); // Size 40MB
    char* c2 = randomHex(1000000*16); // Size 8MB
    bigInt *tmp1 = hexStringToBigInt(c1);
    bigInt *tmp2 = hexStringToBigInt(c2);
    free(c1);
    free(c2);

    bigInt *res = SSA_modular(tmp1, tmp2);
    printf("Len: %zu\n", getLen(res));
}

void findBestValues() {
    size_t nFast;
    size_t kFast;
    double bestTime = DBL_MAX;
    uint64_t fibN = 10000000;
    printf("Beginning testing\n");
    for (size_t n = 20; n < 1000; n += 5) {
        for (size_t k = n + 2; k < n + 5000; k += 30) {
            printf("\rTesting %lu %lu", n, k);
            //naiveMulFaster = n;
            //karatsubaFaster = k;
            struct timespec start, end;
            getCurrentTime(&start);

            bigInt *res = fibonacci(fibN);

            getCurrentTime(&end);
            double time = calcTimeDiff(&start, &end);
            if (time < bestTime) {
                bestTime = time;
                nFast = n;
                kFast = k;
                printf("\nFound new best! n: %lu, k: %lu, time: %f\n", nFast, kFast, bestTime);
            }
            freeBigInt(res);
        }
    }
}

void testBenchMark() {
    size_t iterations = 1;

    size_t n = 400000;
    char* c1 = randomHex(n*16);
    bigInt *tmp = hexStringToBigInt(c1);
    free(c1);

    struct timespec start;
    char *res;
    getCurrentTime(&start);
    for (size_t i = 0; i < iterations; i++) {
        res = bigIntToDecString(tmp, true);
    }
    struct timespec end;
    getCurrentTime(&end);
    double time = calcTimeDiff(&start, &end);
    printf("Time in code 1: %f\n", time);

    printf("%lu\n", strlen(res));
    free(res);
}

void benchMark() {
    size_t iterations = 1;
    printf("Benchmark with Iterations: %ld\n", iterations);

    //size_t n = 200000;
    size_t n = 4000000;

    char* c1 = randomHex(n*16);
    char* c2 = randomHex(n*16);
    bigInt *tmp1 = hexStringToBigInt(c1);
    bigInt *tmp2 = hexStringToBigInt(c2);
    free(c1);
    free(c2);

    //code1
    struct timespec start;
    bigInt *res1;
    getCurrentTime(&start);
    for (size_t i = 0; i < iterations; i++) {
        res1 = mulSingleThread(tmp1, tmp2);
    }
    struct timespec end;
    getCurrentTime(&end);
    double time = calcTimeDiff(&start, &end);
    printf("Time in code 1: %f\n", time);
    printf("----------------------------------------------\n");

    //code2
    struct timespec start2;
    bigInt *res2;
    getCurrentTime(&start2);
    for (size_t i = 0; i < iterations; i++) {
        res2 = ntt_mul(tmp1, tmp2);
    }
    struct timespec end2;
    getCurrentTime(&end2);
    double time2 = calcTimeDiff(&start2, &end2);
    printf("Time in code 2: %f\n", time2);
    printf("----------------------------------------------\n");

    if (compareBigInt(res1, res2) != 0) {
        printf("Numbers not equal!\n");
    } else {
        printf("Numbers equal\n");
    }
    freeBigInt(res1);
    freeBigInt(res2);
    freeBigInt(tmp1);
    freeBigInt(tmp2);
}

void bruteForceDebug() {
    bool multiThread = global_config.parallel;
    global_config.parallel = false;
    if (multiThread) {
        printf("Bruteforce debug for multi-thread and single-thread\n");
    } else {
        printf("Bruteforce debug for single-thread\n");
    }
    //edge cases
    bigInt *res = newBigInt(1);
    res->bigIntArray[0] = 0;
    bigInt *res2 = fibonacci(0);
    bigInt *res3 = nullptr;
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibonacci(0);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 0 for single-thread\n");
        printf("%s\n", bigIntToDecString(res, true));
        printf("%s\n", bigIntToDecString(res2, true));
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }

    if (multiThread && compareBigInt(res, res3) != 0) {
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
    res2 = fibonacci(1);
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibonacci(1);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 1 for single-thread\n");
        printf("%s\n", bigIntToDecString(res, true));
        printf("%s\n", bigIntToDecString(res2, true));
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }
    if (multiThread && compareBigInt(res, res3) != 0) {
        printf("Failed at 1 for multi-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
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
            printf("\rTesting %luth fibonacci number", i + 1);
            fib = add(fibMinus1, fibMinus2);
            freeBigInt(fibMinus2);
            fibMinus2 = fibMinus1;
            fibMinus1 = fib;
            res2 = fibonacci(i + 1);
            if (compareBigInt(fibMinus1, res2) != 0) {
                printf("Failed at %lu for single-thread\n", i + 1);

                printf("%s\n", bigIntToHexString(fibMinus1, false));
                printf("%s\n", bigIntToHexString(res2, false));

                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                if (multiThread) {
                    freeBigInt(res3);
                }
                return;
            }
            if (multiThread) {
                global_config.parallel = true;
                res3 = fibonacci(i + 1);
                global_config.parallel = false;
                if (compareBigInt(fibMinus1, res3) != 0) {
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

char *randomHex(uint64_t n) {
    char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char *str = malloc(n + 1);
    uint64_t i = 0;
    str[i] = hex[(random() % 15) + 1]; // no leading zero
    i++;
    for (; i < n - 1; ++i) {
        str[i] = hex[random() % 16];
    }
    str[i] = '\0';
    return str;
}
