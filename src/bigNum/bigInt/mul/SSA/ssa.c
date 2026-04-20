#include "ssa.h"

#include "ssaHelper.h"
#include "../../bigIntMethods.h"
#include "../../../misc.h"

#include <math.h>
#include <stdio.h>

bigInt **FFT_modF(bigInt **a, const size_t N, const size_t fermatIndex) {
    if (N == 1) {
        bigInt **Y = malloc(sizeof(bigInt *));
        mallocCheck(Y);
        Y[0] = copyBigInt(a[0]);
        return Y;
    }
    const size_t half = N / 2;
    const size_t depth = custom_tzcnt(N);

    // split even/odd
    bigInt **even = malloc(half * sizeof(bigInt *));
    bigInt **odd = malloc(half * sizeof(bigInt *));
    for (size_t i = 0; i < half; ++i) {
        even[i] = a[2 * i];
        odd[i] = a[2 * i + 1];
    }
    bigInt **Ye = FFT_modF(even, half, fermatIndex);
    bigInt **Yo = FFT_modF(odd, half, fermatIndex);
    // Butterfly
    bigInt **Y = malloc(half * sizeof(bigInt *));
    mallocCheck(Y);
    const size_t tmp = 1ULL << (fermatIndex - (depth - 1));
    for (size_t k = 0; k < half; k += 2) {
        const size_t rev = bit_reverse(k / 2, depth - 1) + 1;
        const size_t x = tmp * rev;
        bigInt *t = rotateLeftModF(Yo[k / 2], x, 1ULL << (fermatIndex + 1));
        freeBigInt(Yo[k / 2]);
        // Y[k]    = Ye[k] + t
        // Y[k+1]  = Ye[k] - t
        if (N == 2) {
            Y[k] = subModF(Ye[k / 2], t, fermatIndex);
        } else {
            Y[k] = addModF(Ye[k / 2], t, fermatIndex);
            Y[k + 1] = subModF(Ye[k / 2], t, fermatIndex);
        }
        freeBigInt(t);
        freeBigInt(Ye[k / 2]);
    }
    // cleanup arrays
    free(even);
    free(odd);
    free(Ye);
    free(Yo);
    return Y;
}

bigInt **iFFT_modF(bigInt **a, size_t N, const size_t fermatIndex) {
    if (N == 1) {
        bigInt **Y = malloc(sizeof(bigInt *));
        mallocCheck(Y);
        Y[0] = copyBigInt(a[0]);
        return Y;
    }
    size_t half = N / 2;
    size_t depth = custom_tzcnt(N);

    // Butterfly
    bigInt **Y = malloc(N * sizeof(bigInt *));
    mallocCheck(Y);
    const size_t tmp = 1ULL << (fermatIndex - depth);
    for (size_t k = 0; k < N; k += 2) {
        const size_t rev = bit_reverse(k / 2, depth) + 1;
        const size_t x = (tmp * rev);

        bigInt *add = addModF(a[k], a[k + 1], fermatIndex);
        bigInt *sub = subModF(a[k], a[k + 1], fermatIndex);

        Y[k] = rotateRightModF(add, 1, 1ULL << (fermatIndex + 1));
        freeBigInt(add);
        const size_t toRotate = x + 1; // + 1 because 2^(-1)
        Y[k + 1] = rotateRightModF(sub, toRotate, 1ULL << (fermatIndex + 1));
        freeBigInt(sub);
    }

    // split even/odd
    bigInt **even = malloc(half * sizeof(bigInt *));
    bigInt **odd = malloc(half * sizeof(bigInt *));
    for (size_t i = 0; i < half; ++i) {
        even[i] = Y[2 * i];
        odd[i] = Y[2 * i + 1];
    }
    free(Y);
    bigInt **Ye = iFFT_modF(even, half, fermatIndex);
    bigInt **Yo = iFFT_modF(odd, half, fermatIndex);

    for (size_t i = 0; i < half; ++i) {
        freeBigInt(even[i]);
        freeBigInt(odd[i]);
    }

    // recombine
    bigInt **res = malloc(N * sizeof(bigInt *));
    mallocCheck(res);
    for (size_t i = 0; i < N; i += 2) {
        res[i] = Ye[i / 2];
        res[i + 1] = Yo[i / 2];
    }

    // cleanup arrays
    free(even);
    free(odd);
    free(Ye);
    free(Yo);
    return res;
}

// implements the Schönhage–Strassen algorithm from the original paper https://link.springer.com/article/10.1007/BF02242355
bigInt *SSA_modular(const bigInt *A, const bigInt *B) {
    const size_t bitLenA = bitLength(A);
    const size_t bitLenB = bitLength(B);
    const size_t M = bitLenA + bitLenB;

    // 1) Parameter
    const uint64_t m = (uint64_t) floor(log2((double) M - 1)) + 1;
    const bool mOdd = m % 2 == 1;
    const uint64_t n = mOdd ? (m + 1) / 2 : (m + 2) / 2;
    const uint64_t chunkLengthBits = 1ULL << (n - 1);
    const uint64_t chunkLength = chunkLengthBits / 64;
    const uint64_t numChunks = mOdd ? 1ULL << (n + 1) : 1ULL << n;

    if (chunkLength < 1 || n < 6) {
        // 2^(n-1) >= 64
        fprintf(stderr, "Error: SSA called but parameters do not work | chunkLength: %lu, n: %lu\n", chunkLength, n);
        exit(EXIT_FAILURE);
    }

    // 2) blockwise Splitting in a[0..numChunks-1], b[0..numChunks-1], each block has 2n-1 bits
    bigInt **a = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(a);
    bigInt **b = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(b);
    for (size_t i = 0; i < numChunks; ++i) {
        a[i] = sliceBigInt(A, i * chunkLength, chunkLength);
        b[i] = sliceBigInt(B, i * chunkLength, chunkLength);
    }

    // 3) integer convolution for z_j mod 2^(n+2)
    const size_t mod2_bits = n + 2;
    // a_i mod 2^(n+2), b_i mod 2^(n+2)
    bigInt **alpha = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(alpha);
    bigInt **beta = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(beta);
    for (size_t i = 0; i < numChunks; ++i) {
        alpha[i] = getFirstNBits(a[i], mod2_bits); // mod 2^(n+2)
        beta[i] = getFirstNBits(b[i], mod2_bits);
    }

    // Numbers u, v to the convolution product
    const size_t gap = 3 * n + 5;

    bigInt *u = getZeroBigInt();
    bigInt *v = getZeroBigInt();
    for (size_t i = 0; i < numChunks; ++i) {
        bigInt *t1 = shiftLeft(alpha[i], i * gap);
        freeBigInt(alpha[i]);
        bigInt *t2 = shiftLeft(beta[i], i * gap);
        freeBigInt(beta[i]);
        bigInt *tmp1 = add(u, t1);
        freeBigInt(u);
        freeBigInt(t1);
        bigInt *tmp2 = add(v, t2);
        freeBigInt(v);
        freeBigInt(t2);
        u = tmp1;
        v = tmp2;
    }
    free(alpha);
    free(beta);
    // u * v
    bigInt *uv = mulSingleThread(u, v);
    freeBigInt(u);
    freeBigInt(v);

    // split in γ[0..2K-2], chunk size = gap
    const size_t gammaSize = 2 * numChunks;
    bigInt **gamma = malloc(gammaSize * sizeof(bigInt *));
    mallocCheck(gamma);
    for (size_t i = 0; i < gammaSize; ++i) {
        gamma[i] = getFirstNBits(uv, gap);
        bigInt *tmp = shiftRight(uv, gap);
        freeBigInt(uv);
        uv = tmp;
    }
    freeBigInt(uv);

    // z2[i] = (γ[i] + γ[i+2*2^n] - γ[i+2^n] - γ[i+3*2^n]) mod 2^(n+2)
    bigInt **z2 = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(z2);
    uint64_t TwoPowN = numChunks / 2;
    for (size_t j = 0; j < numChunks / 2; ++j) {
        // pure integer sub/add, then mod (by cutting of the high bits)
        bigInt *sum1 = add(gamma[j], gamma[j + 2 * TwoPowN]);
        bigInt *sum2 = add(gamma[j + TwoPowN], gamma[j + 3 * TwoPowN]);
        bigInt *diff = sub(sum1, sum2);
        freeBigInt(sum1);
        freeBigInt(sum2);
        // mod 2^(n+2):
        z2[j] = getFirstNBits(diff, mod2_bits);
        freeBigInt(diff);
    }
    for (size_t i = 0; i < 2 * numChunks; ++i) {
        freeBigInt(gamma[i]);
    }
    free(gamma);

    // 4) FFT in Z_F_n, to calculate z_j mod F_n
    bigInt **Ahat = FFT_modF(a, numChunks, n);
    for (size_t i = 0; i < numChunks; ++i) {
        freeBigInt(a[i]);
    }
    free(a);

    bigInt **Bhat = FFT_modF(b, numChunks, n);
    for (size_t i = 0; i < numChunks; ++i) {
        freeBigInt(b[i]);
    }
    free(b);

    // Pointwise multiplication in the ring mod F
    bigInt **Chat = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(Chat);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        bigInt *AhatReduced = reduceModF(Ahat[i], n);
        freeBigInt(Ahat[i]);
        bigInt *BhatReduced = reduceModF(Bhat[i], n);
        freeBigInt(Bhat[i]);

        Chat[i] = mulSingleThread(AhatReduced, BhatReduced); // mod F_n because reduced beforehand
        freeBigInt(AhatReduced);
        freeBigInt(BhatReduced);
    }
    free(Ahat);
    free(Bhat);

    // Inverse FFT
    bigInt **c = iFFT_modF(Chat, numChunks / 2, n);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        freeBigInt(Chat[i]);
    }
    free(Chat);

    // reduce c mod F_n to get the zF
    bigInt **zF = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(zF);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        zF[i] = reduceModF(c[i], n);
        freeBigInt(c[i]);
    }
    free(c);

    // 5) CRT‑Combination: z[j] = combine(z2[j] mod 2^(n+2), zF[j] mod F_n)
    bigInt **z = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(z);
    bigInt *TwoPowNPlus2 = getBigIntFromUnsignedInteger(1ULL << (n + 2));
    bigInt *zero = getBigIntFromUnsignedInteger(0);
    const size_t k = n + 2;
    for (size_t j = 0; j < numChunks / 2; ++j) {
        // δ = (z2[j] - zF[j]) mod 2^(n+2)
        bigInt *deltaNoMod = sub(z2[j], zF[j]);
        if (deltaNoMod->negative) {
            reduceToFirstNBits(deltaNoMod, k);
            if (compareBigInt(deltaNoMod, zero) != 1) {
                // a > b
                bigInt *tmp = add(deltaNoMod, TwoPowNPlus2);
                freeBigInt(deltaNoMod);
                deltaNoMod = tmp;
            }
        }

        freeBigInt(z2[j]);
        reduceToFirstNBits(deltaNoMod, mod2_bits);
        bigInt *delta = deltaNoMod;

        // z[j] = zF[j] + δ * F
        bigInt *shift = rotateLeftModF(delta, 1ULL << n, 1ULL << (m + 1)); // shift and add equals mul by fermat number
        bigInt *t_mul = add(shift, delta);

        freeBigInt(shift);
        freeBigInt(delta);

        z[j] = add(zF[j], t_mul);
        freeBigInt(t_mul);
        freeBigInt(zF[j]);
    }
    freeBigInt(zero);
    freeBigInt(TwoPowNPlus2);
    free(z2);
    free(zF);

    // 6) assembling the result
    bigInt *result = newBigInt(getLen(A) + getLen(B));
    for (size_t j = 0; j < numChunks / 2; ++j) {
        shiftAddSameNumberSafe(result, z[j], j * chunkLength);
        freeBigInt(z[j]);
    }
    free(z);

    // final reduce by F_m
    bigInt *resultMod = reduceModF(result, m);
    freeBigInt(result);
    stripLeadingZeros(resultMod);

    return resultMod;
}
