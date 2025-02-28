#include <stdio.h>
#include <time.h>

#include "config.h"
#include "bigIntHigherFunctions.h"

typedef struct {
    bigInt *fib; // Fibonacci value
    bigInt *luc; // Lucas value
} FibLucPair;


bigInt *fibonacci(uint64_t n) {
    return fibWithLucas(n);
}


// from https://www.nayuki.io/page/fast-fibonacci-algorithms
bigInt *fibExpFastDoubling(uint64_t n) {
    bigInt *a = getZeroBigInt();
    bigInt *b = getBigIntFromUnsignedInteger(1);
    unsigned int shift = 64 - custom_lzcnt(n) - 1;
    uint64_t nBinary = ((n >> shift) << shift);

    //for verbose
    unsigned long iterations = 64 - custom_lzcnt(nBinary);
    int counter = 1;
    struct timespec start, end;
    if (global_config.verbose) {
        printf("\n");
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }

    for (; nBinary != 0; nBinary >>= 1) {
        if (global_config.verbose) {
            if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
            size_t sizeInBytes = (a->end - a->start) * 8;
            double sizeInMB = ((double) sizeInBytes) / 1000000;
            char *localTime = getCurrentDateTime();
            printf("Iteration ongoing %d/%lu; Current size: %f MB; Time needed for previous iteration: %f s; Time: %s\n",
                   counter, iterations, sizeInMB, time, localTime);
            free(localTime);
            counter++;
            if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
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
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time needed for last Iteration: %fs\n\n", time);
    }
    return a;
}

FibLucPair *fibLuc(int64_t n);

bigInt *fibWithLucas(uint64_t n) {
    if (n == 0) {
        return getBigIntFromUnsignedInteger(0);
    }
    if (n > INT64_MAX) {
        fprintf(stderr, "This implementation only goes up to the signed 64bit max (%ld)\n", INT64_MAX);
        exit(EXIT_FAILURE);
    }
    FibLucPair *resStrct = fibLuc((int64_t) n);
    bigInt *res = resStrct->fib;
    freeBigInt(resStrct->luc);
    free(resStrct);
    return res;
}

// from https://www.reddit.com/r/compsci/comments/1im1jd1/20000000th_fibonacci_number_in_1_second/
// also this here: https://ii.uni.wroc.pl/~lorys/IPL/article75-6-1.pdf
FibLucPair *fibLuc(int64_t n) {
    FibLucPair *result;

    // Base case: n == 0: return (0, 2)
    if (n == 0) {
        result = malloc(sizeof(FibLucPair));
        result->fib = getBigIntFromUnsignedInteger(0);
        result->luc = getBigIntFromUnsignedInteger(2);
        return result;
    }

    // If n is negative, compute fibLuc(-n) and then adjust by k = (n % 2)*2 - 1
    if (n < 0) {
        n = -n;
        result = fibLuc(n);
        // k = (n % 2) * 2 - 1  → if n is even, k = -1; if odd, k = 1.
        int64_t k = ((n % 2) * 2) - 1;
        if (k < 0) {
            // Multiply both results by -1.
            negateBigInt(result->fib);
            negateBigInt(result->luc);
        }
        // If k is 1, no change is needed.
        return result;
    }

    result = malloc(sizeof(FibLucPair));
    // If n is odd (n & 1 == 1)
    if (n & 1) {
        // Compute fibLuc(n-1)
        FibLucPair *prev = fibLuc(n - 1);

        // new_fib = (prev->fib + prev->luc) >> 1
        bigInt *sum_fib = add(prev->fib, prev->luc);
        bigInt *fib_new = shiftRight(sum_fib, 1);
        freeBigInt(sum_fib);

        // new_luc = (5 * prev->fib + prev->luc) >> 1
        bigInt *five = getBigIntFromUnsignedInteger(5);
        bigInt *five_mul_fib = mul(five, prev->fib);
        freeBigInt(prev->fib);
        freeBigInt(five);
        bigInt *sum_luc = add(five_mul_fib, prev->luc);
        freeBigInt(prev->luc);
        freeBigInt(five_mul_fib);
        bigInt *luc_new = shiftRight(sum_luc, 1);
        freeBigInt(sum_luc);

        free(prev);

        result->fib = fib_new;
        result->luc = luc_new;
        return result;
    } else {
        // n is even:
        // First, halve n.
        n = n >> 1;
        // k = (n % 2) * 2 - 1; (again, if n is even then k will be -1; if odd then 1)
        int64_t k = ((n % 2) * 2) - 1;
        // Compute fibLuc(n)
        FibLucPair *half = fibLuc(n);
        // new_fib = half.fib * half.luc
        bigInt *fib_new = mul(half->fib, half->luc);
        freeBigInt(half->fib);
        // new_luc = (half.luc * half.luc) + (2 * k)
        bigInt *luc_sq = mul(half->luc, half->luc);
        freeBigInt(half->luc);
        free(half);
        bigInt *two_k = getBigIntFromSignedInteger(2 * k);
        bigInt *luc_new = add(luc_sq, two_k);
        freeBigInt(two_k);
        freeBigInt(luc_sq);

        result->fib = fib_new;
        result->luc = luc_new;
        return result;
    }
}
