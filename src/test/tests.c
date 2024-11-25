#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include "../bigInt/config.h"
#include "../bigInt/bigIntUtil.h"
#include "../bigInt/bigIntDiv.h"
#include "../bigInt/bigIntHigherFunctions.h"

char *randomHex(uint64_t n);

void testDivision();

void testTmp();

void test_3n2n();

void test() {
    /*char *s = "-9698221053161177687290477425859177932382231164913342457988942565290603202032650977539673794871485524578715841560821371184254768182683230358929880858382521293289634064686775240148439910622723626693351342619237245018481973741897215406417692743811995376601444856451017397396417899698609191911359966057798066039205146074555347057423885652715598354116597263475575225013948848826913119642585626149915370382407333373204309776889362896964546553585125389218785193583749079779612087805721514940976263748453986289509059119186233922071511696667252145264712463351571840908051208005640995123972835129592728270908144152982173929578671815352373605079181589988683452734666617071587964740803149982508004635806882850317235137399006314035263560229394694346066643749060723111271285246484494116318732807711306211369572790833509025425730662481323570713984822850116880925502130609069130981079649673794577278532426425064207683625366677767555628333393957865436271874960335826862703692492693265732759980111000515532341595977524";
    printf("%s\n", s);
    bigInt* a = decStringToBigInt(s);
    printBigIntDec(a);
    freeBigInt(a);*/

    testDivision();

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
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    bigInt *a = readBigIntDecFromFile("output.txt");

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("%lu %lu\n", a->start, a->end);
    printf("Time in div: %f\n", time);
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

    bigInt* a = hexStringToBigInt(randomHex(150001*16));
    bigInt* b = hexStringToBigInt(randomHex(13502*16));

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
    bigInt *res1 = divideModSingleThread(a, b, &q1);
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

    if(compareBigInt(res1, res2) != 0) printf("RES not equal\n");
    else printf("RES equal\n");
    if(compareBigInt(q1, q2) != 0) printf("Reminder not equal\n");
    else printf("Reminder equal\n");

    //printBigIntHex(add(multiplyToomCook3(b, res1), q1)); //check

    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(q1);
    freeBigInt(q2);
    freeBigInt(res1);
    freeBigInt(res2);
}

void test_3n2n() {
    bigInt *a = hexStringToBigInt("F9D02BBFDE1A0FA314FB0C68B24506AC66BFC5DEA6B0A78AEE182AAB079732907B03BFA75EC9EB73BEAC6A4D5A01563E03FA58C146597FD46B8DDB82052D6067D5929B3C1F40A039542E1ABC5C61BAA52E053B4C3643F204EF259D2E98042A948AAC5E884CB3EC7DB925643FD34FDD467E2CC");
    bigInt *b = hexStringToBigInt("56DC304E875C9D4B3FB2125AE3D0CD3130D6114989517ACA97DAA2485181EB31C07D2C6A5BCC587E048A6D2BEACD6FE206F225C708461B41FDB5AD087C5DC4FCAEEC3A3437A42E51B065D6");

    printBigIntHex(a);
    printBigIntHex(b);

    printf("TEST: %ld %ld %ld %ld\n", a->end - a->start, b->end - b->start, custom_lzcnt(a->bigIntArray[a->end - 1]),
           custom_lzcnt(b->bigIntArray[b->end - 1]));

    bigInt *q1;
    bigInt *q2;
    bigInt *res1 = divideD4Helper(a, b, &q1);
    bigInt *res2 = divideD4Helper(a, b, &q2);

    printf("\nResults:\n");
    printBigIntHex(res1);
    printBigIntHex(res2);
    printf("\nRemainders:\n");
    printBigIntHex(q1);
    printBigIntHex(q2);

    if(compareBigInt(res1, res2) != 0) printf("RES not equal\n");
    else printf("RES equal\n");
    if(compareBigInt(q1, q2) != 0) printf("Reminder not equal\n");
    else printf("Reminder equal\n");

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

    bigInt *test1 = hexStringToBigInt(randomHex(1000*16));
    bigInt *test2 = hexStringToBigInt(randomHex(1000*16));

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
        bigInt *res2 = multiplyToomCook3MultiThread(test1, test2, 1);
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
    bool print = false;
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
    if (compareBigInt(res, res2)  != 0) {
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
        printf("%s\n", bigIntToDecString(res));
        printf("%s\n", bigIntToDecString(res2));
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
            if (print) {
                char *resString = bigIntToDecString(res2);
                printf("%s\n", resString);
                free(resString);
            }
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
