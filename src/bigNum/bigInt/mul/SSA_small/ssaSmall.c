#include "ssaSmall.h"

#include "../../../misc.h"
#include "../../bigIntMethods.h"
#include "ssaSmallHelper.h"

#include <stdio.h>


void fftModNPrime(bigInt **XParts, const uint64_t K, uint64_t **l, const uint64_t omega, const uint64_t nprime, const uint64_t inc,
                  const uint64_t start) {
    if (K == 2) {
        bigInt *a0 = XParts[start];
        bigInt *a1 = XParts[start + inc];
        bigInt *tmp1 = addModNPrime(a0, a1, nprime);
        bigInt *tmp2 = subModNPrime(a0, a1, nprime);
        freeBigInt(a0);
        freeBigInt(a1);
        XParts[start] = tmp1;
        XParts[start + inc] = tmp2;
    } else {
        const uint64_t K2 = K >> 1;
        fftModNPrime(XParts, K2, l, 2 * omega, nprime, inc * 2, start);
        fftModNPrime(XParts, K2, l, 2 * omega, nprime, inc * 2, start + inc);

        const uint64_t k_idx = 64 - custom_lzcnt(K) - 1;
        const uint64_t *lk = l[k_idx];

        for (size_t j = 0; j < K2; j++) {
            const size_t idx0 = start + 2 * j * inc;
            const size_t idx1 = start + (2 * j + 1) * inc;
            bigInt *a0 = XParts[idx0];
            bigInt *a1 = XParts[idx1];

            const uint64_t shift = lk[2 * j] * omega;
            bigInt *a1_twiddled = mul2ExpModNPrime(a1, shift, nprime);
            freeBigInt(a1);

            bigInt *tmp1 = addModNPrime(a0, a1_twiddled, nprime);
            bigInt *tmp2 = subModNPrime(a0, a1_twiddled, nprime);
            freeBigInt(a1_twiddled);
            freeBigInt(a0);
            XParts[idx0] = tmp1;
            XParts[idx1] = tmp2;
        }
    }
}

void ifftModNPrime(bigInt **XParts, const uint64_t K, const uint64_t omega, const uint64_t nprime, const uint64_t start) {
    if (K == 2) {
        bigInt *a0 = XParts[start];
        bigInt *a1 = XParts[start + 1];
        bigInt *tmp1 = addModNPrime(a0, a1, nprime);
        bigInt *tmp2 = subModNPrime(a0, a1, nprime);
        freeBigInt(a0);
        freeBigInt(a1);
        XParts[start] = tmp1;
        XParts[start + 1] = tmp2;
    } else {
        const uint64_t K2 = K >> 1;
        ifftModNPrime(XParts, K2, 2 * omega, nprime, start);
        ifftModNPrime(XParts, K2, 2 * omega, nprime, start + K2);

        for (size_t j = 0; j < K2; j++) {
            const size_t idx0 = start + j;
            const size_t idx1 = start + j + K2;
            bigInt *a0 = XParts[idx0];
            bigInt *a1 = XParts[idx1];

            const uint64_t shift = j * omega;
            bigInt *a1_twiddled = mul2ExpModNPrime(a1, shift, nprime);
            freeBigInt(a1);

            bigInt *tmp1 = addModNPrime(a0, a1_twiddled, nprime);
            bigInt *tmp2 = subModNPrime(a0, a1_twiddled, nprime);
            freeBigInt(a1_twiddled);
            freeBigInt(a0);
            XParts[idx0] = tmp1;
            XParts[idx1] = tmp2;
        }
    }
}

bigInt *SSA_small(const bigInt *A, const bigInt *B) {
    const uint64_t S = bitLength(A) + bitLength(B);
    const uint64_t k = 5; // TODO

    uint64_t pl = (S + 64 - 1) / 64;
    pl = ((pl + (1ULL << k) - 1) >> k) << k;
    const uint64_t N = pl * 64;
    uint64_t M = N >> k; // multiple of 64

    const uint64_t maxLK = lcm(64, 1ULL << k);
    const uint64_t Nprime = ((2 * M + k + 2) / maxLK + 1) * maxLK;
    const uint64_t Mp = Nprime >> k;
    const uint64_t K = 1ULL << k;

    // printf("N: %lu, M: %lu, Nprime: %lu, Mp: %lu, K: %lu\n", N, M, Nprime, Mp, K);

    uint64_t **l = fftInitL(k);

    bigInt **AParts = decompose(A, K, Nprime, M, Mp);
    bigInt **BParts = decompose(B, K, Nprime, M, Mp);

    fftModNPrime(AParts, K, l, 2 * Mp, Nprime, 1, 0);
    fftModNPrime(BParts, K, l, 2 * Mp, Nprime, 1, 0);

    for (size_t i = 0; i < K; i++) {
        bigInt *tmp = mulSingleThread(AParts[i], BParts[i]);
        freeBigInt(AParts[i]);
        freeBigInt(BParts[i]);
        AParts[i] = reduceModNPrime(tmp, Nprime);
        freeBigInt(tmp);
    }

    ifftModNPrime(AParts, K, 2 * Mp, Nprime, 0);

    // Post-inverse FFT division
    BParts[0] = div2ExpModNPrime(AParts[0], k, Nprime);
    freeBigInt(AParts[0]);
    for (size_t i = 1; i < K; i++) {
        BParts[i] = div2ExpModNPrime(AParts[i], k + (K - i) * Mp, Nprime);
        freeBigInt(AParts[i]);
    }
    free(AParts);

    bigInt *res = newBigInt(getLen(A) + getLen(B));
    for (size_t i = K; i-- > 0;) {
        const uint64_t j = (K - i) & (K - 1);
        bigInt *bj = BParts[j];
        shiftAddSameNumberSafe(res, bj, i * (M / 64));

        // Subtract modulus if wrap-around negative occurred
        bigInt *T_val = getBigIntFromUnsignedInteger(i + 1);
        if (compareShiftedBigInt(bj, T_val, 2 * M) == 1) {
            // bj > T_val
            // TODO: optimize
            printf("FUCCKSS\n");
            bigInt *one = getBigIntFromUnsignedInteger(1);
            bigInt *tmp = shiftLeft(one, Nprime);
            bigInt *mod = add(tmp, one);
            freeBigInt(one);
            freeBigInt(tmp);
            bigInt *shifted = shiftLeft(mod, i * M);
            freeBigInt(mod);
            tmp = sub(res, shifted);
            freeBigInt(shifted);
            freeBigInt(res);
            res = tmp;
        }
        freeBigInt(T_val);
        freeBigInt(bj);
    }
    free(BParts);


    for (size_t i = 0; i < k + 1; i++) {
        free(l[i]);
    }
    free(l);

    // Final normalization modulo 2^N + 1
    bigInt *finalRes = reduceModNPrime(res, N);
    freeBigInt(res);
    return finalRes;
}
