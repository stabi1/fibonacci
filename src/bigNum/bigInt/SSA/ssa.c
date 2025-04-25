#include "ssa.h"

#include "ssaHelper.h"
#include "../bigIntMethods.h"
#include "../../misc.h"

#include <math.h>
#include <stdio.h>

const uint64_t NORMAL_MUL_FASTER = 0;

bigInt **FFT_modF_helper(bigInt **a, size_t N, const size_t fermatIndex);

bigInt **iFFT_modF_helper(bigInt **a, size_t N, const size_t fermatIndex);

bigInt **FFT_modF(bigInt **a, size_t N, const size_t fermatIndex) {
    bigInt **fftRes = FFT_modF_helper(a, N, fermatIndex);
    bigInt **res = malloc(N / 2 * sizeof(bigInt *));
    mallocCheck(res);

    size_t depth = custom_tzcnt(N);

    for (size_t i = 0; i < N / 2; i++) {
        size_t resIndex = 2 * i + 1;
        size_t fftIndex = bit_reverse(resIndex, depth);
        res[i] = fftRes[fftIndex - N / 2];
    }

    free(fftRes);
    return res;
}

bigInt **FFT_modF_helper(bigInt **a, size_t N, const size_t fermatIndex) {
    if (N == 1) {
        bigInt **Y = malloc(sizeof(bigInt *));
        mallocCheck(Y);
        Y[0] = copyBigInt(a[0]);
        return Y;
    }
    size_t half = N / 2;
    size_t depth = custom_tzcnt(N);

    // split even/odd
    bigInt **even = malloc(half * sizeof(bigInt *));
    bigInt **odd = malloc(half * sizeof(bigInt *));
    for (size_t i = 0; i < half; ++i) {
        even[i] = a[2 * i];
        odd[i] = a[2 * i + 1];
    }
    bigInt **Ye = FFT_modF_helper(even, half, fermatIndex);
    bigInt **Yo = FFT_modF_helper(odd, half, fermatIndex);
    // Butterfly
    bigInt **Y = malloc(half * sizeof(bigInt *));
    mallocCheck(Y);
    size_t tmp = 1ULL << (fermatIndex - (depth - 1));
    for (size_t k = 0; k < half; k += 2) {
        size_t rev = bit_reverse(k / 2, depth - 1) + 1;
        size_t x = tmp * rev;
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

    bigInt **ifftA = malloc(N * sizeof(bigInt *));
    mallocCheck(ifftA);

    size_t depth = custom_tzcnt(N * 2);
    for (size_t i = 0; i < N; i++) {
        size_t resIndex = 2 * i + 1;
        size_t fftIndex = bit_reverse(resIndex, depth);
        ifftA[fftIndex - N] = a[i];
    }

    bigInt **res = iFFT_modF_helper(ifftA, N, fermatIndex);
    free(ifftA);
    return res;
}

bigInt **iFFT_modF_helper(bigInt **a, size_t N, const size_t fermatIndex) {
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
    size_t tmp = 1ULL << (fermatIndex - depth);
    for (size_t k = 0; k < N; k += 2) {
        size_t rev = bit_reverse(k / 2, depth) + 1;
        size_t x = tmp * rev;

        bigInt *add = addModF(a[k], a[k + 1], fermatIndex);
        bigInt *sub = subModF(a[k], a[k + 1], fermatIndex);

        Y[k] = rotateRightModF(add, 1, 1ULL << (fermatIndex + 1));
        freeBigInt(add);
        Y[k + 1] = rotateRightModF(sub, x + 1, 1ULL << (fermatIndex + 1));
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
    bigInt **Ye = iFFT_modF_helper(even, half, fermatIndex);
    bigInt **Yo = iFFT_modF_helper(odd, half, fermatIndex);

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

// return a bigInt that contains the slice offset * chunkSize with size chunkSize all in bits
bigInt *sliceBigIntBits(const bigInt *x, size_t offset, size_t chunkSize) {
    bigInt *shifted = shiftRight(x, offset * chunkSize);
    bigInt *result = getFirstNBits(shifted, chunkSize);
    freeBigInt(shifted);
    return result;
}


bigInt *SSA_modular(const bigInt *A, const bigInt *B) {
    size_t bitLenA = bitLength(A), bitLenB = bitLength(B);
    size_t M = bitLenA > bitLenB ? bitLenA : bitLenB;
    // Base case
    if (M <= NORMAL_MUL_FASTER) {
        return mulSingleThread(A, B);
    }

    // 1) Parameter
    uint64_t m = floor(log2(2 * M - 1)) + 1; //TODO *64
    uint64_t n = m % 2 == 0 ? (m + 2) / 2 : (m + 1) / 2;
    uint64_t paddedLengthBits = 1ULL << (m + 1);
    uint64_t paddedLength = paddedLengthBits / 64;
    uint64_t chunkLengthBits = 1ULL << (n - 1);
    uint64_t numChunks = m % 2 == 0 ? 1ULL << n : 1ULL << (n + 1);
    uint64_t chunkLength = chunkLengthBits / 64;

    printf("M: %lu, m: %lu, n: %lu, paddedLengthBits: %lu, chunkLengthBits: %lu, paddedLength: %lu, chunkLength: %lu, numChunks: %lu\n", M, m, n, paddedLengthBits,
           chunkLengthBits, paddedLength, chunkLength, numChunks);

    // Build: F_n = 2^(2^n) + 1
    bigInt *fermatBase = getBigIntFromUnsignedInteger(1);
    bigInt *power = shiftLeft(fermatBase, (1ULL << n)); // 2^(2^n)
    bigInt *F_n = add(power, fermatBase); // +1
    freeBigInt(fermatBase);
    freeBigInt(power);

    // Build: F_m = 2^(2^m) + 1
    fermatBase = getBigIntFromUnsignedInteger(1);
    power = shiftLeft(fermatBase, (1ULL << m)); // 2^(2^m)
    bigInt *F_m = add(power, fermatBase); // +1
    freeBigInt(fermatBase);
    freeBigInt(power);

    // 2) blockwise Splitting in a[0..numChunks-1], b[0..numChunks-1], each block has 2n-1 bits
    bigInt **a = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(a);
    bigInt **b = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(b);
    for (size_t i = 0; i < numChunks; ++i) {
        a[i] = sliceBigIntBits(A, i, chunkLengthBits);
        b[i] = sliceBigIntBits(B, i, chunkLengthBits);
    }

    for (long i = numChunks - 1; i >= 0; --i) {
        printf("%s|", bigIntToBinaryString(a[i], false));
    }
    printf("\n");
    for (long i = numChunks - 1; i >= 0; --i) {
        printf("%s|", bigIntToBinaryString(b[i], false));
    }
    printf("\n");


    // 3) integer convolution for z_j mod 2^(n+2)
    size_t mod2_bits = n + 2;
    // a_i mod 2^(n+2), b_i mod 2^(n+2)
    bigInt **alpha = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(alpha);
    bigInt **beta = malloc(numChunks * sizeof(bigInt *));
    mallocCheck(beta);
    for (size_t i = 0; i < numChunks; ++i) {
        alpha[i] = getFirstNBits(a[i], mod2_bits);   // mod 2^(n+2)
        beta[i] = getFirstNBits(b[i], mod2_bits);
    }

    // Numbers u, v to the convolution product
    size_t gap = 3 * n + 5;
    //printf("gap: %lu\n", gap);

    bigInt *u = getZeroBigInt();
    bigInt *v = getZeroBigInt();
    for (size_t i = 0; i < numChunks; ++i) {
        bigInt *t1 = shiftLeft(alpha[i], i * gap);
        freeBigInt(alpha[i]);
        bigInt *t2 = shiftLeft(beta[i], i * gap);
        freeBigInt(beta[i]);
        bigInt *tmp1 = add(u, t1);
        bigInt *tmp2 = add(v, t2);
        freeBigInt(u);
        freeBigInt(v);
        freeBigInt(t1);
        freeBigInt(t2);
        u = tmp1;
        v = tmp2;
    }
    free(alpha);
    free(beta);
    //printBigIntBinary(u);
    //printBigIntBinary(v);
    // u * v
    bigInt *uv = mulSingleThread(u, v); //TODO Recursion?
    freeBigInt(u);
    freeBigInt(v);
    //printBigIntBinary(uv);

    // split in γ[0..2K-2], chunk size = gap
    size_t gammaSize = 2 * numChunks;
    bigInt **gamma = malloc(gammaSize * sizeof(bigInt *));
    mallocCheck(gamma);
    for (size_t i = 0; i < gammaSize; ++i) {
        gamma[i] = getFirstNBits(uv, gap);
        bigInt *tmp = shiftRight(uv, gap);
        freeBigInt(uv);
        uv = tmp;
    }
    freeBigInt(uv);

    /*for (long i = numChunks - 1; i >= 0; --i) {
        printf("%s|", bigIntToBinaryString(gamma[i], false));
    }
    printf("\n");*/


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

    /*printf("Z2:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        //printf("%s\n", bigIntToBinaryString(z2[i], false));
    }*/

    // 4) FFT in Z_F_n, to calculate z_j mod F_n
    bigInt **Ahat = FFT_modF(a, numChunks, n);
    /*printf("Ahat:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntBinary(Ahat[i]);
    }*/
    for (size_t i = 0; i < numChunks; ++i) {
        freeBigInt(a[i]);
    }
    free(a);

    bigInt **Bhat = FFT_modF(b, numChunks, n);
    /*printf("Bhat:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntBinary(Bhat[i]);
    }*/
    for (size_t i = 0; i < numChunks; ++i) {
        freeBigInt(b[i]);
    }
    free(b);

    // Pointwise multiplication in the ring mod F
    bigInt **Chat = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(Chat);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        bigInt *AhatReduced = reduceModF(Ahat[i], F_n, n);
        freeBigInt(Ahat[i]);
        bigInt *BhatReduced = reduceModF(Bhat[i], F_n, n);
        freeBigInt(Bhat[i]);

        /*printf("\n--Reduced: index %lu\n", 2 * i + 1);
        printBigIntBinary(AhatReduced);
        printBigIntBinary(BhatReduced);*/

        Chat[i] = mulSingleThread(AhatReduced, BhatReduced);  // mod F because reduced beforehand
        freeBigInt(AhatReduced);
        freeBigInt(BhatReduced);
    }
    free(Ahat);
    free(Bhat);

    /*printf("\nChat:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntBinary(Chat[i]);
    }*/

    // Inverse FFT
    bigInt **c = iFFT_modF(Chat, numChunks / 2, n);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        freeBigInt(Chat[i]);
    }
    free(Chat);

    /*printf("c:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntBinary(c[i]);
    }*/

    // reduce c mod F_n to get the zF
    bigInt **zF = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(zF);
    for (size_t i = 0; i < numChunks / 2; ++i) {
        zF[i] = reduceModF(c[i], F_n, n);
        freeBigInt(c[i]);
    }
    free(c);
    freeBigInt(F_n);

    /*printf("zF:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntBinary(zF[i]);
    }*/

    // 5) CRT‑Combination: z[j] = combine(z2[j] mod 2^(n+2), zF[j] mod F_n)
    bigInt **z = malloc(numChunks / 2 * sizeof(bigInt *));
    mallocCheck(z);
    for (size_t j = 0; j < numChunks / 2; ++j) {
        // δ = (z2[j] - zF[j]) mod 2^(n+2)
        bigInt *tmp = sub(z2[j], zF[j]);
        freeBigInt(z2[j]);
        bigInt *delta = getFirstNBits(tmp, mod2_bits);
        freeBigInt(tmp);

        // z[j] = zF[j] + δ * F
        bigInt *shift = rotateLeftModF(delta, 1ULL << n, 1ULL << (m + 1)); // shift and add equals mul by fermat number
        bigInt *t_mul = addModF(shift, delta, m);
        freeBigInt(shift);
        freeBigInt(delta);

        z[j] = addModF(zF[j], t_mul, m);
        freeBigInt(t_mul);
        freeBigInt(zF[j]);
    }
    free(z2);
    free(zF);

    /*printf("z_j:\n");
    for (size_t i = 0; i < numChunks / 2; ++i) {
        printBigIntDec(z[i]);
    }
    printf("\n");*/

    // 6) assembling the result
    bigInt *result = getZeroBigInt();
    for (size_t j = 0; j < numChunks / 2; ++j) {
        // rotate by j * 2^(n-1) bits
        bigInt *t = rotateLeftModF(z[j], j * 1ULL << (n - 1), 1ULL << (m + 1));
        freeBigInt(z[j]);
        bigInt *tmp = add(result, t);
        freeBigInt(result);
        freeBigInt(t);
        result = tmp;
    }
    free(z);


    // final reduce by F_m
    //printBigIntHex(result);
    bigInt *resultMod = reduceModF(result, F_m, m);
    freeBigInt(F_m);
    freeBigInt(result);

    return resultMod;
}