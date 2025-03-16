#include "bigFracHigherFunctions.h"

#include "../misc.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

double goldenRatioDouble = 1.61803399;

bigFrac *goldenRatio(size_t binaryDigits) {
    struct timespec start, end;
    size_t nFib = ((double) binaryDigits + log2(sqrt(5))) / log2(goldenRatioDouble); // accurate Estimate
    nFib += 10; // to be safe

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigInt *fibN = fibonacci(nFib);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForFibN = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for fibN: %f s\n", timeForFibN);
    }

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigInt *fibNMinus1 = fibonacci(nFib - 1);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForFibNMinus1 = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for fibNMinus1: %f s\n", timeForFibNMinus1);
    }

    bigFrac *fibNFrac = newBigFracFromBigInt(fibN, false);
    bigFrac *fibNMinus1Frac = newBigFracFromBigInt(fibNMinus1, false);

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigFrac *res = divideBigFrac(fibNFrac, fibNMinus1Frac, getLen(fibNMinus1));
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForDivision = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for the division: %f s\n", timeForDivision);
    }

    freeBigFrac(fibNFrac);
    freeBigFrac(fibNMinus1Frac);
    return res;
}

// Pi algorithm adapted from https://www.craig-wood.com/nick/articles/pi-chudnovsky/
struct chudnovskyBinarySplittingReturn {
    bigInt *P;
    bigInt *Q;
    bigInt *T;
};


struct chudnovskyBinarySplittingReturn *chudnovskyBinarySplitting(uint64_t a, uint64_t b, bigInt *C3Over24) {
    bigInt *Pab;
    bigInt *Qab;
    bigInt *Tab;

    if (b - a == 1) {
        // Directly compute P(a,a+1), Q(a,a+1) and T(a,a+1)
        if (a == 0) {
            Pab = getBigIntFromUnsignedInteger(1);
            Qab = getBigIntFromUnsignedInteger(1);
        } else {
            bigInt *tmp1 = getBigIntFromUnsignedInteger(6 * a - 5);
            bigInt *tmp2 = getBigIntFromUnsignedInteger(2 * a - 1);
            bigInt *tmp3 = getBigIntFromUnsignedInteger(6 * a - 1);
            bigInt *tmp4 = mul(tmp1, tmp2);
            freeBigInt(tmp1);
            freeBigInt(tmp2);
            Pab = mul(tmp4, tmp3);
            freeBigInt(tmp3);
            freeBigInt(tmp4);

            tmp1 = getBigIntFromUnsignedInteger(a);
            tmp2 = mul(tmp1, tmp1);
            tmp3 = mul(tmp1, C3Over24);
            freeBigInt(tmp1);
            Qab = mul(tmp2, tmp3);
            freeBigInt(tmp2);
            freeBigInt(tmp3);
        }
        bigInt *tmp1 = getBigIntFromUnsignedInteger(545140134);
        bigInt *tmp2 = getBigIntFromUnsignedInteger(a);
        bigInt *tmp3 = mul(tmp1, tmp2);
        freeBigInt(tmp1);
        freeBigInt(tmp2);
        bigInt *tmp4 = getBigIntFromUnsignedInteger(13591409);
        bigInt *tmp5 = add(tmp4, tmp3);
        freeBigInt(tmp3);
        freeBigInt(tmp4);
        Tab = mul(Pab, tmp5);  // a(a) * p(a);
        freeBigInt(tmp5);
        if (a & 1) {
            negateBigInt(Tab);
        }
    } else {
        // Recursively compute P(a, b), Q(a, b) and T(a, b)
        // m is the midpoint of a and b
        uint64_t m = (a + b) / 2;
        // Recursively calculate P(a, m), Q(a, m) and T(a, m)
        struct chudnovskyBinarySplittingReturn *amRes = chudnovskyBinarySplitting(a, m, C3Over24);
        bigInt *Pam = amRes->P;
        bigInt *Qam = amRes->Q;
        bigInt *Tam = amRes->T;
        free(amRes);
        // Recursively calculate P(m, b), Q(m, b) and T(m, b)
        struct chudnovskyBinarySplittingReturn *mbRes = chudnovskyBinarySplitting(m, b, C3Over24);
        bigInt *Pmb = mbRes->P;
        bigInt *Qmb = mbRes->Q;
        bigInt *Tmb = mbRes->T;
        free(mbRes);

        // Now combine
        Pab = mul(Pam, Pmb);
        freeBigInt(Pmb);

        Qab = mul(Qam, Qmb);
        freeBigInt(Qam);

        bigInt *tmp1 = mul(Qmb, Tam);
        freeBigInt(Qmb);
        freeBigInt(Tam);
        bigInt *tmp2 = mul(Pam, Tmb);
        freeBigInt(Pam);
        freeBigInt(Tmb);
        Tab = add(tmp1, tmp2);
        freeBigInt(tmp1);
        freeBigInt(tmp2);
    }
    struct chudnovskyBinarySplittingReturn *res = malloc(sizeof(struct chudnovskyBinarySplittingReturn));
    mallocCheck(res);
    res->Q = Qab;
    res->P = Pab;
    res->T = Tab;
    return res;
}


bigFrac *pi(size_t binaryDigits) {
    size_t wantedFractionBlocks = (binaryDigits / 64) + 1;
    struct timespec start, end;

    bigInt *C = getBigIntFromUnsignedInteger(640320);
    bigInt *tmp = mul(C, C);
    bigInt *C3 = mul(tmp, C);
    freeBigInt(tmp);
    freeBigInt(C);
    bigInt *twentyFour = getBigIntFromUnsignedInteger(24);
    bigInt *C3Over24 = divide(C3, twentyFour);
    freeBigInt(twentyFour);
    freeBigInt(C3);

    double C3Over24Double = 10939058860032000;
    double bitsPerTerm = log2(C3Over24Double / 6 / 2 / 6);
    size_t n = (size_t) ((double) binaryDigits / bitsPerTerm) + 1;

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    struct chudnovskyBinarySplittingReturn *binarySplittingRes = chudnovskyBinarySplitting(0, n, C3Over24);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForBinarySplitting = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for Binary Splitting: %f s\n", timeForBinarySplitting);
    }

    freeBigInt(binarySplittingRes->P);
    freeBigInt(C3Over24);

    bigFrac *radicand = getBigFracFromUnsignedInteger(10005);

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigFrac *sqrtC = sqrt2(radicand, wantedFractionBlocks);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForRoot = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for root calculation: %f s\n", timeForRoot);
    }
    freeBigFrac(radicand);

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    tmp = getBigIntFromUnsignedInteger(426880);
    bigInt *tmp2 = mul(binarySplittingRes->Q, tmp);
    freeBigInt(tmp);
    freeBigInt(binarySplittingRes->Q);
    bigFrac *tmpFrac = newBigFracFromBigInt(tmp2, false);

    bigFrac *tmpFrac2 = mulBigFrac(tmpFrac, sqrtC);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForMuls = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for timeForMuls: %f s\n", timeForMuls);
    }

    freeBigFrac(tmpFrac);
    freeBigFrac(sqrtC);

    bigFrac *T_Frac = newBigFracFromBigInt(binarySplittingRes->T, false);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigFrac *res = divideBigFrac(tmpFrac2, T_Frac, wantedFractionBlocks);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForDivision = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for division: %f s\n", timeForDivision);
    }

    freeBigFrac(tmpFrac2);
    freeBigFrac(T_Frac);

    free(binarySplittingRes);
    return res;
}

struct eTaylorBinarySplittingReturn {
    bigInt *P;
    bigInt *Q;
};


struct eTaylorBinarySplittingReturn *eTaylorBinarySplitting(uint64_t a, uint64_t b) {
    bigInt *P;
    bigInt *Q;
    if (b - a == 1) {
        if (a == 0) {
            P = getBigIntFromUnsignedInteger(1);
            Q = getBigIntFromUnsignedInteger(1);
        } else {
            // Here we represent the term 1/a! as a rational number.
            // At the base case, we output (1, a) so that when combined,
            // the denominator correctly accumulates the factorial factors.
            P = getBigIntFromUnsignedInteger(1);
            Q = getBigIntFromUnsignedInteger(a);
        }
    } else {
        uint64_t m = (a + b) / 2;
        struct eTaylorBinarySplittingReturn *amRes = eTaylorBinarySplitting(a, m);
        struct eTaylorBinarySplittingReturn *mbRes = eTaylorBinarySplitting(m, b);

        // Combine the two halves:
        // sum_{n=a}^{b-1} 1/n! = P1/Q1 + P2/Q2 = (P1 * Q2 + P2) / (Q1 * Q2)
        bigInt* tmp = mul(amRes->P, mbRes->Q);
        freeBigInt(amRes->P);
        P = add(tmp, mbRes->P);
        freeBigInt(tmp);
        freeBigInt(mbRes->P);
        Q = mul(amRes->Q, mbRes->Q);
        freeBigInt(amRes->Q);
        freeBigInt(mbRes->Q);

        free(amRes);
        free(mbRes);
    }

    struct eTaylorBinarySplittingReturn *res = malloc(sizeof(struct eTaylorBinarySplittingReturn));
    mallocCheck(res);
    res->P = P;
    res->Q = Q;
    return res;
}


bigFrac *e(size_t binaryDigits) {
    size_t wantedFractionBlocks = (binaryDigits / 64) + 1;
    struct timespec start, end;

    // Estimate n:
    double total_log = 0.0;
    size_t n = 0;
    // Increase T until log2((T+1)!) > prec
    while (total_log <= binaryDigits) {
        n += 1;
        total_log += log2((double) n);
    }


    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    struct eTaylorBinarySplittingReturn *binarySplittingRes = eTaylorBinarySplitting(0, n);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForDivision = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for binary splitting: %f s\n", timeForDivision);
    }

    bigFrac *P_Frac = newBigFracFromBigInt(binarySplittingRes->P, false);
    bigFrac *Q_Frac = newBigFracFromBigInt(binarySplittingRes->Q, false);
    free(binarySplittingRes);

    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    }
    bigFrac *res = divideBigFrac(P_Frac, Q_Frac, wantedFractionBlocks);
    if (global_config.verbose) {
        if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
        double timeForDivision = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time for division: %f s\n", timeForDivision);
    }

    freeBigFrac(P_Frac);
    freeBigFrac(Q_Frac);

    return res;
}
