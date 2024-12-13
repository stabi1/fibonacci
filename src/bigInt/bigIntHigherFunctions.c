#include <stdio.h>
#include <time.h>

#include "config.h"
#include "bigIntHigherFunctions.h"

bigInt *fibonacci(uint64_t n) {
    return fibExpFastDoubling(n);
}

bigInt *fibExpFastDoubling(uint64_t n) {
    bigInt *a = newBigInt(1);
    bigInt *b = newBigInt(1);
    b->bigIntArray[0] = 1;
    unsigned int shift = 64 - custom_lzcnt(n) - 1;
    uint64_t nBinary = ((n >> shift) << shift);

    //for verbose
    unsigned long iterations = 64 - custom_lzcnt(nBinary);
    int counter = 1;
    struct timespec start, end;
    if (global_config.verbose) {
        printf("\n");
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    for (; nBinary != 0; nBinary >>= 1) {
        if (global_config.verbose) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
            size_t sizeInBytes = (a->end - a->start) * 8;
            double sizeInMB = ((double) sizeInBytes) / 1000000;
            char *localTime = getCurrentDateTime();
            printf("Iteration ongoing %d/%lu; Current size: %f MB; Time needed for previous iteration: %f s; Time: %s\n", counter, iterations, sizeInMB, time, localTime);
            free(localTime);
            counter++;
            clock_gettime(CLOCK_MONOTONIC, &start);
        }
        bigInt *temp1 = shiftLeft(b, 1);
        bigInt *temp2 = sub(temp1, a);
        freeBigInt(temp1);

        bigInt *d;
        bigInt *temp3;
        bigInt *temp4;

        //to swap: b
        char *filename_b = storeBigIntInSwap(b);
        d = mul(a, temp2);
        if (global_config.verbose) {
            printf("First mul done");
            fflush(stdout);
        }
        freeBigInt(temp2);

        //to swap: b, d
        char *filename_d = storeBigIntInSwap(d);
        temp3 = mul(a, a);
        if (global_config.verbose) {
            printf("; Second mul done");
            fflush(stdout);
        }
        freeBigInt(a);


        //to swap: d, temp3
        char *filename_temp3 = storeBigIntInSwap(temp3);
        b = loadBigIntFromSwap(b, filename_b);
        temp4 = mul(b, b);
        if (global_config.verbose) {
            printf("; Third mul done\n");
            fflush(stdout);
        }
        freeBigInt(b);
        temp3 = loadBigIntFromSwap(temp3, filename_temp3);

        bigInt *e = add(temp3, temp4);
        freeBigInt(temp3);
        freeBigInt(temp4);

        d = loadBigIntFromSwap(d, filename_d);
        a = d;
        b = e;

        // Advance by one conditionally
        if ((n & nBinary) != 0) {
            bigInt *c = add(a, b);
            freeBigInt(a);
            a = b;
            b = c;
        }
    }
    freeBigInt(b);

    if (global_config.verbose) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time needed for last Iteration: %fs\n\n", time);
    }
    return a;
}