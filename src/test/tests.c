#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include "../bigInt/bigIntUtil.h"
#include "../bigInt/bigIntDiv.h"
#include "../bigInt/bigIntHigherFunctions.h"

char *randomHex(uint64_t n);

void testDivision();

void testTmp();

void customTest() {
    char *s = "-7879728336815204864708405324323";
    printf("%s\n", s);
    bigInt *a = decStringToBigInt(s);
    printBigIntDec(a);
    freeBigInt(a);

    //testDivision();

    //test_3n2n();
    //testTmp();

    //findBestValues();
    /*uint64_t n = 40000;
    bigInt *res1 = fibExpFastDoubling(n, 0);
    bigInt *res2 = fibExpFastDoubling(n, true, 0);
    if (compareBigInt(res1, res2) != 0) {
        printf("Fault\n");
        return;
    }
    printf("Correct; Length of result in 8 byte: %lu\n", res1->end - res1->start);
    freeBigInt(res1);
    freeBigInt(res2);*/
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

//segfault for default
//bigInt* a = hexStringToBigInt(randomHex(2000));//1600
//bigInt* b = hexStringToBigInt(randomHex(430));//800

void testDivision() {
    //bigInt* a = hexStringToBigInt(randomHex(480));//1600
    //bigInt *a = hexStringToBigInt("0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    //bigInt *a = hexStringToBigInt("0EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    // bigInt *b = hexStringToBigInt(randomHex(320));//800
    //bigInt* a = hexStringToBigInt("60AFFA1E64881027A5DFE87D0CF4C95C5A26F03C11261EDC1869940DB6F4AABB14399A3F971C6DE320F446838C54E7886E76355B9BEB41AFBF0AA77EE04C62E83D14403A961E6E2A4EAD428945EF8BD15F8CD243DDE45620C2CAA0EE405315E24C105DF5FF7AA1946D64BA41C08FFDD5FB117300");
    //bigInt* b = hexStringToBigInt("11F4E298E5F5C0EAD06AA4EE91C9BE5CE51C7B55F29D367E6E91381A9C25A808D252DA6ACE604FEBC9AE1AA96CD14F801F2D99647E4AC548D16CA142E3230C50B7C312770C3D165C7C93DD4BE");

    //Working for div
    //bigInt* a = hexStringToBigInt(randomHex(30000*16));
    //bigInt* b = hexStringToBigInt(randomHex(7000*16));

    bigInt *a = hexStringToBigInt(randomHex(15001 * 16));
    bigInt *b = hexStringToBigInt(randomHex(1302 * 16));

    bigInt *q1;
    bigInt *q2;

    //printBigIntHex(a);
    //printBigIntHex(b);
    printf("\n");
    printf("TEST: %ld %ld %ld %ld\n", a->end - a->start, b->end - b->start, custom_lzcnt(a->bigIntArray[a->end - 1]),
           custom_lzcnt(b->bigIntArray[b->end - 1]));

    printf("Begin of calculation!!!\n\n\n");


    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bigInt *res1 = divideModSingleThread(a, b, &q1, false);
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in div: %f\n", time);


    struct timespec start2;
    clock_gettime(CLOCK_MONOTONIC, &start2);
    bigInt *res2 = divideMod(a, b, &q2);
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in div: %f\n", time2);

    /*printf("\nResults:\n");
    printBigIntHex(res1);
    printBigIntHex(res2);
    printf("\n");
    printBigIntHex(q1);
    printBigIntHex(q2);*/

    if (compareBigInt(res1, res2) != 0) printf("RES not equal\n");
    else printf("RES equal\n");
    if (compareBigInt(q1, q2) != 0) printf("Reminder not equal\n");
    else printf("Reminder equal\n");

    //printBigIntHex(add(multiplyToomCook3(b, res1), q1)); //check

    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(q1);
    freeBigInt(q2);
    freeBigInt(res1);
    freeBigInt(res2);
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
            clock_gettime(CLOCK_MONOTONIC, &start);

            bigInt *res = fibExpFastDoubling(fibN);

            struct timespec end;
            clock_gettime(CLOCK_MONOTONIC, &end);
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
    size_t n = 500; //iterations

    bigInt *test1 = hexStringToBigInt(randomHex(1000 * 16));
    bigInt *test2 = hexStringToBigInt(randomHex(1000 * 16));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        bigInt *res1 = mul(test1, test2);
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
        bigInt *res2 = mulParallel(test1, test2, 1);
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
    bigInt *res2 = fibExpFastDoubling(0);
    bigInt *res3 = NULL;
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibExpFastDoubling(0);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 0 for single-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
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
    res2 = fibExpFastDoubling(1);
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibExpFastDoubling(1);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 1 for single-thread\n");
        printf("%s\n", bigIntToDecString(res, false));
        printf("%s\n", bigIntToDecString(res2, false));
        freeBigInt(res);
        freeBigInt(res2);
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
            res2 = fibExpFastDoubling(i + 1);
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
                res3 = fibExpFastDoubling(i + 1);
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
