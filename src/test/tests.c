#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#include "../bigNum/bigInt/bigIntString.h"
#include "../bigNum/bigInt/bigIntDiv.h"
#include "../bigNum/bigInt/bigIntHigherFunctions.h"

#include "../bigNum/bigFrac/bigFracString.h"
#include "../bigNum/bigFrac/bigFrac.h"

char *randomHex(uint64_t n);

void testDivision();

void testTmp();

void customTest() {

    bigFrac *t = decStringToBigFrac("1111000000000000000009999.081168362269707909223138302439316426245231129529004592563155000691687396818589967117254645335071333252165770094757883981895706734867764623701447815216189092333922218128499333957269850715834306202206684974916003684536979732624269370220027679396858384812642435823717642555455708503236173678906654601057717516645699553961372418703127616547636062830048429775468899548346022638546855060457682821263214095354232210617368780895007033257347415006578548653037235084929529720735446894497934663126289700692955769880568602487512103410474818893979636728149713154835692317880769669072384259907758715741009274653409211645643460229505407249907629868415805462450244342617", true, 5);

    printBigFracDec(t, false);

    freeBigFrac(t);

    //testTmp();
}

void testTmp() {
    char *rand = randomHex(15001 * 16);
    bigInt *a = hexStringToBigInt(rand);
    free(rand);
    char *res1 = bigIntToDecString(a, false);

    global_config.parallel = true;
    global_config.mulDepth = 1;
    global_config.convertDepth = 2;
    global_config.swap = true;
    global_config.swapThreshold = 1;
    printf("Second one\n");
    char *res2 = bigIntToDecString(a, false);

    if (strcmp(res1, res2) != 0) {
        printf("Strings not equal!!!!\n");
    } else {
        printf("Strings are equal\n");
    }
    free(res1);
    free(res2);
    freeBigInt(a);
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
            struct timespec start;
            if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

            bigInt *res = fibonacci(fibN);

            struct timespec end;
            if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
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

void benchMark() {
    size_t iterations = 1; //iterations
    uint64_t n = 400000000;

    printf("Benchmark with Iterations: %ld\n", iterations);

    //code1
    struct timespec start;
    bigInt *res1;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    for (size_t i = 0; i < iterations; i++) {
        res1 = fibExpFastDoubling(n);
        freeBigInt(res1);
    }
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);
    printf("----------------------------------------------\n");

    //code2
    struct timespec start2;
    bigInt *res2;
    if (clock_gettime(CLOCK_MONOTONIC, &start2) == -1) perror("Error measuring time!");
    for (size_t i = 0; i < iterations; i++) {
        res2 = fibWithLucas(n);
        freeBigInt(res2);
    }
    struct timespec end2;
    if (clock_gettime(CLOCK_MONOTONIC, &end2) == -1) perror("Error measuring time!");
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    /*if (compareBigInt(res1, res2) != 0) {
        printf("Numbers not equal!\n");
    } else {
        printf("Numbers equal\n");
    }
    freeBigInt(res1);
    freeBigInt(res2);*/
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
    bigInt *res3 = NULL;
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

                printf("%s\n", bigIntToHexString(fibMinus1));
                printf("%s\n", bigIntToHexString(res2));

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
    for (; i < n; ++i) {
        str[i] = hex[random() % 16];
    }
    str[i] = '\0';
    return str;
}
