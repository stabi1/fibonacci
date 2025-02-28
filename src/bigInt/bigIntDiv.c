#include "bigIntDiv.h"
#include "bigIntMul.h"

size_t D4FASTER = 70;
size_t PARALLEL_DIV_FASTER = 1000;

//To be used when the number is known to be exactly divisible by 3
bigInt *exactDivideBy3(const bigInt *x) {
    size_t len = getLen(x);
    bigInt *result = newBigIntNotZeroed(len);
    result->negative = x->negative;
    unsigned long borrow;
    unsigned __int128 q, xx, w;
    borrow = 0;
    size_t j = x->start;
    for (size_t i = 0; i < len; i++, j++) {
        xx = x->bigIntArray[j];
        w = xx - borrow;
        if (borrow > xx) {
            borrow = 1;
        } else {
            borrow = 0;
        }
        q = (unsigned __int128) (w * 0xAAAAAAAAAAAAAAAB);
        result->bigIntArray[i] = (uint64_t) q;

        if ((unsigned long long) q >= 0x5555555555555556) {
            borrow++;
            if ((unsigned long long) q >= 0xAAAAAAAAAAAAAAAB) {
                borrow++;
            }
        }
    }
    if (result->bigIntArray[result->end - 1] == 0 && getLen(result) > 1) {
        result->end -= 1;
    }
    return result;
}

bigInt *divide(const bigInt *dividend, const bigInt *divisor) {
    bigInt *res = divideHelper((bigInt *) dividend, (bigInt *) divisor, NULL, false, global_config.parallel, global_config.mulDepth, false);
    res->negative = dividend->negative ^ divisor->negative;
    return res;
}

bigInt *divideMod(const bigInt *dividend, const bigInt *divisor, bigInt **reminder) {
    bigInt *res = divideHelper((bigInt *) dividend, (bigInt *) divisor, reminder, false, global_config.parallel, global_config.mulDepth, false);
    res->negative = dividend->negative ^ divisor->negative;
    (*reminder)->negative = dividend->negative;
    return res;
}

bigInt *divideSingleThread(bigInt *dividend, bigInt *divisor) {
    bigInt *res = divideHelper(dividend, divisor, NULL, false, false, 0, false);
    res->negative = dividend->negative ^ divisor->negative;
    return res;
}

bigInt *divideModSingleThread(bigInt *dividend, bigInt *divisor, bigInt **reminder, bool freeArguments) {
    bool tmp1 = dividend->negative ^ divisor->negative;
    bool tmp2 = dividend->negative;
    bigInt *res = divideHelper(dividend, divisor, reminder, false, false, 0, freeArguments);
    res->negative = tmp1;
    (*reminder)->negative = tmp2;
    return res;
}

bigInt *divideModMultiThread(bigInt *dividend, bigInt *divisor, bigInt **reminder, size_t mulDepth, bool freeArguments) {
    bool tmp1 = dividend->negative ^ divisor->negative;
    bool tmp2 = dividend->negative;
    bigInt *res = divideHelper(dividend, divisor, reminder, false, true, mulDepth, freeArguments);
    res->negative = tmp1;
    (*reminder)->negative = tmp2;
    return res;
}

bigInt *divideHelper(bigInt *dividend, bigInt *divisor, bigInt **reminder, bool noBurnikelZiegler, bool multithread, size_t mulDepth, bool freeArguments) {
    uint64_t m = dividend->end - dividend->start;
    uint64_t n = divisor->end - divisor->start;

    const uint64_t *u = dividend->bigIntArray + dividend->start;
    const uint64_t *v = divisor->bigIntArray + divisor->start;

    int comp = compareBigInt(dividend, divisor);
    if (comp < 0) {
        bigInt *quotient = getZeroBigInt();
        if (reminder != NULL) {
            *reminder = newBigIntNotZeroed(m);
            memcpy((*reminder)->bigIntArray + (*reminder)->start, u, m * 8);
        }
        if (freeArguments) {
            freeBigInt(dividend);
            freeBigInt(divisor);
        }
        return quotient;
    } else if (comp == 0) {
        if (freeArguments) {
            freeBigInt(dividend);
            freeBigInt(divisor);
        }
        bigInt *quotient = getBigIntFromUnsignedInteger(1);
        if (reminder != NULL) {
            *reminder = getZeroBigInt();
        }
        return quotient;
    } else if (m <= 1) {
        bigInt *quotient = newBigIntNotZeroed(1);
        quotient->bigIntArray[0] = u[0] / v[0];
        if (reminder != NULL) {
            *reminder = newBigIntNotZeroed(n);
            (*reminder)->bigIntArray[(*reminder)->start] = u[0] % v[0];
        }
        if (freeArguments) {
            freeBigInt(dividend);
            freeBigInt(divisor);
        }
        return quotient;
    } else if (n <= 1) {
        bigInt *quotient = newBigInt(m - n + 1);
        if (reminder != NULL) {
            *reminder = newBigIntNotZeroed(n);
            divideOneWord(dividend, v[0], quotient, *reminder);
        } else {
            divideOneWord(dividend, v[0], quotient, NULL);
        }
        if (freeArguments) {
            freeBigInt(dividend);
            freeBigInt(divisor);
        }
        return quotient;
    }

    if (n < D4FASTER || noBurnikelZiegler) {
        bigInt *quotient = newBigInt(m - n + 1);
        if (reminder == NULL) {
            divideD4(dividend, divisor, quotient, NULL);
        } else {
            *reminder = newBigInt(n);
            divideD4(dividend, divisor, quotient, *reminder);
        }
        if (freeArguments) {
            freeBigInt(dividend);
            freeBigInt(divisor);
        }
        return quotient;
    } else {
        bool doMultiThread = true;
        if (n < PARALLEL_DIV_FASTER || !multithread)
            doMultiThread = false;
        if (reminder == NULL) {
            bigInt *reminderTmp;
            bigInt *qTmp = divideBurnikelZiegler(dividend, divisor, &reminderTmp, doMultiThread, mulDepth, freeArguments);
            freeBigInt(reminderTmp);
            return qTmp;
        } else {
            return divideBurnikelZiegler(dividend, divisor, reminder, doMultiThread, mulDepth, freeArguments);
        }
    }
}


void divideOneWord(bigInt *dividend, uint64_t divisor, bigInt *quotient, bigInt *reminder) {
    uint64_t *q = quotient->bigIntArray + quotient->start;
    uint64_t *d = dividend->bigIntArray + dividend->start;
    size_t m = dividend->end - dividend->start;
    unsigned __int128 divisor128 = (unsigned __int128) divisor;

    uint64_t rem = d[m - 1];
    unsigned __int128 rem128 = (unsigned __int128) rem;
    if (rem < divisor) {
        q[m - 1] = 0;
    } else {
        uint64_t tmp = rem / divisor;
        q[m - 1] = tmp;
        rem = (uint64_t) (rem128 - (tmp * divisor128));
        rem128 = (unsigned __int128) rem;
    }
    size_t xLen = 1;
    while (++xLen <= m) {
        unsigned __int128 dividendEstimate = (rem128 << 64) | (unsigned __int128) (d[m - xLen]);
        uint64_t qAct = dividendEstimate / divisor;
        rem = dividendEstimate % divisor;

        q[m - xLen] = qAct;
        rem128 = (unsigned __int128) rem;
    }
    if (reminder != NULL) {
        reminder->bigIntArray[reminder->start] = rem;
    }
    quotient->end = quotient->start + getOccupiedBlocks(quotient);
}

//returns dividend/divisor
//adapted from https://raw.githubusercontent.com/hcs0/Hackers-Delight/master/divmnu64.c.txt
void divideD4(const bigInt *dividend, const bigInt *divisor, bigInt *quotient, bigInt *reminder) {
    uint64_t *q = quotient->bigIntArray + quotient->start;
    const uint64_t *restrict u = dividend->bigIntArray + dividend->start;
    const uint64_t *restrict v = divisor->bigIntArray + divisor->start;
    long m = (long) (dividend->end - dividend->start);
    long n = (long) (divisor->end - divisor->start);

    unsigned __int128 b = 18446744073709551615U; // Number base (2**32).
    b++;
    uint64_t *restrict un, *restrict vn; // Normalized form of u, v.
    unsigned __int128 qhat; // Estimated quotient digit.
    unsigned __int128 rhat; // A remainder.
    unsigned __int128 p; // Product of two digits.
    __int128 t, k;
    long s, i, j;
    if (v[n - 1] == 0) {//should not happen
        fprintf(stderr, "v[n-1] is zero!\n");
        exit(5);
    }
    //Normalize by shifting v left just enough so that its high-order
    //bit is on, and shift u left the same amount. We may have to append a
    //high-order digit on the dividend; we do that unconditionally.
    s = (long) custom_lzcnt(v[n - 1]); // 0 <= s <= 63.
    vn = malloc(sizeof(uint64_t) * n);
    mallocCheck(vn);
    for (i = n - 1; i > 0; i--)
        vn[i] = (v[i] << s) | ((unsigned __int128) v[i - 1] >> (64 - s));
    vn[0] = v[0] << s;
    un = malloc(sizeof(uint64_t) * (m + 1));
    mallocCheck(un);
    un[m] = (unsigned __int128) u[m - 1] >> (64 - s);
    for (i = m - 1; i > 0; i--)
        un[i] = (u[i] << s) | ((unsigned __int128) u[i - 1] >> (64 - s));
    un[0] = u[0] << s;
    for (j = m - n; j >= 0; j--) { // Main loop.
        // Compute estimate qhat of q[j].
        qhat = (un[j + n] * b + un[j + n - 1]) / vn[n - 1];
        rhat = (un[j + n] * b + un[j + n - 1]) % vn[n - 1];
        again:
        if (qhat >= b || qhat * vn[n - 2] > b * rhat + un[j + n - 2]) {
            qhat = qhat - 1;
            rhat = rhat + vn[n - 1];
            if (rhat < b) goto again;
        }
        // Multiply and subtract.
        k = 0;
        for (i = 0; i < n; i++) {
            p = qhat * vn[i];
            t = (__int128) (un[i + j] - k - (p & 0xFFFFFFFFFFFFFFFF));
            un[i + j] = t;
            k = (__int128) (p >> 64) - (t >> 64);
        }
        t = un[j + n] - k;
        un[j + n] = t;
        q[j] = qhat; // Store quotient digit.
        if (t < 0) { // If we subtracted too
            q[j] = q[j] - 1; // much, add back.
            k = 0;
            for (i = 0; i < n; i++) {
                t = un[i + j] + vn[i] + k;
                un[i + j] = t;
                k = t >> 64;
            }
            un[j + n] = un[j + n] + k;
        }
    } // End j.
    free(vn);
    // If the caller wants the remainder, unnormalize
    // it and pass it back.
    if (reminder != NULL) {
        uint64_t *r = reminder->bigIntArray + reminder->start;
        for (i = 0; i < n - 1; i++)
            r[i] = (un[i] >> s) | ((unsigned __int128) un[i + 1] << (64 - s));
        r[n - 1] = un[n - 1] >> s;
        reminder->end = reminder->start + getOccupiedBlocks(reminder);
    }
    free(un);
    quotient->end = quotient->start + getOccupiedBlocks(quotient);
}

//return A/B
//adapted from Java Jdk8 (divideBurnikelZiegler, divide2n1n and divide3n2n)
bigInt *divideBurnikelZiegler(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth, bool freeArguments) {
    long s = (long) (B->end - B->start); //s

    // step 1: let m = min{2^k | (2^k)*D4FASTER > s}
    long m = 1 << (64 - custom_lzcnt(s / D4FASTER)); //m

    long j = (s + m - 1) / m;      // step 2a: j = ceil(s/m)
    long n = j * m;             // step 2b: block length in 64-bit units
    long n64 = 64L * n;         // block length in bits
    long sigma = (long) MAX((long) 0, (long) (n64 - bitLength(B)));   // step 3: sigma = max{T | (2^T)*B < beta^n}

    bigInt *bShifted = shiftLeft(B, sigma);// step 4a: shift B so its length is a multiple of n
    if (freeArguments) freeBigInt(B);
    bigInt *aShifted = shiftLeft(A, sigma);    // step 4b: shift A by the same amount
    if (freeArguments) freeBigInt(A);

    // step 5: t is the number of blocks needed to accommodate this plus one additional bit
    long t = (long) ((bitLength(aShifted) + n64) / n64);
    if (t < 2) {
        t = 2;
    }

    // step 6: conceptually split A into blocks a[t-1], ..., a[0]
    bigInt *a1 = getBlock(aShifted, t - 1, t, n);   // the most significant block of A
    // step 7: z[t-2] = [a[t-1], a[t-2]]
    bigInt *z = getBlock(aShifted, t - 2, t, n);    // the second to most significant block
    bigInt *zTmp = shiftAdd(z, a1, n);   // z[t-2]
    freeBigInt(a1);
    freeBigInt(z);
    z = zTmp;
    bigInt *quotient = getZeroBigInt();
    char *filename_quotient = storeBigIntInSwap(quotient);
    bigInt *qi = NULL;
    bigInt *ri = NULL;
    for (int i = (int) t - 2; i > 0; i--) {
        // step 8a: compute (qi,ri) such that z=b*qi+ri
        qi = divide2n1n(z, bShifted, &ri, multithread, mulDepth);

        // step 8b: z = [ri, a[i-1]]
        zTmp = getBlock(aShifted, i - 1, t, n);   // a[i-1]
        freeBigInt(z);
        z = zTmp;
        zTmp = shiftAdd(z, ri, n);
        freeBigInt(ri);
        freeBigInt(z);
        z = zTmp;
        quotient = loadBigIntFromSwap(quotient, filename_quotient);
        bigInt *qTmp = shiftAdd(quotient, qi, i * n);   // update q (part of step 9)
        freeBigInt(quotient);
        quotient = qTmp;
        filename_quotient = storeBigIntInSwap(quotient);
        freeBigInt(qi);

    }
    // final iteration of step 8: do the loop one more time for i=0 but leave z unchanged
    qi = divide2n1n(z, bShifted, &ri, multithread, mulDepth);
    freeBigInt(z);
    freeBigInt(bShifted);
    quotient = loadBigIntFromSwap(quotient, filename_quotient);
    bigInt *qTmp = add(quotient, qi);
    freeBigInt(quotient);
    quotient = qTmp;
    freeBigInt(qi);

    *reminder = shiftRight(ri, sigma);   // step 9: A and B were shifted, so shift back
    freeBigInt(ri);
    freeBigInt(aShifted);

    return quotient;
}

//aLen <= 2*bLen
bigInt *divide2n1n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth) {
    size_t n = B->end - B->start;

    // step 1: base case
    if (n % 2 != 0 || n < D4FASTER || isZero(A)) {
        return divideHelper(A, B, reminder, true, multithread, mulDepth, false);
    }

    // step 2: view A as [a1,a2,a3,a4] where each ai is n/2 ints or less
    bigInt *aUpper = getUpperFrom(A, n / 2);

    // step 3: q1=aUpper/B, r1=aUpper%B
    bigInt *r1 = NULL;
    bigInt *q1 = divide3n2n(aUpper, B, &r1, multithread, mulDepth);
    freeBigInt(aUpper);
    char *filename_q1 = storeBigIntInSwap(q1);

    // step 4: quotient=[r1,aLower]/B, r2=[r1,aLower]%B
    bigInt *aLower = getLowerFrom(A, n / 2); //[a4]
    bigInt *aLower2 = shiftAdd(aLower, r1, n / 2);   // this = [r1,this]
    freeBigInt(r1);
    freeBigInt(aLower);
    bigInt *quotient = divide3n2n(aLower2, B, reminder, multithread, mulDepth);
    freeBigInt(aLower2);

    // step 5: let quotient=[q1,quotient] and return r2
    q1 = loadBigIntFromSwap(q1, filename_q1);
    bigInt *quotientRet = shiftAdd(quotient, q1, n / 2);
    freeBigInt(q1);
    freeBigInt(quotient);
    return quotientRet;
}

//2*aLen<=3*bLen
bigInt *divide3n2n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth) {
    if (isZero(A)) {
        return divideHelper(A, B, reminder, true, multithread, mulDepth, false);
    }

    size_t n = (B->end - B->start) / 2;   // half the length of b in ints

    // step 1: view A as [a1,a2,a3] where each ai is n ints or less; let a12=[a1,a2]
    bigInt *a12 = getUpperFrom(A, n);

    // step 2: view B as [b1,b2] where each bi is n ints or less
    bigInt *b1 = getUpperFrom(B, n);
    bigInt *b2 = getLowerFrom(B, n);

    bigInt *r;
    bigInt *d;
    bigInt *quotient;
    if (compareShiftedBigInt(A, B, n) < 0) {
        // step 3a: if a1<b1, let quotient=a12/b1 and r=a12%b1
        quotient = divide2n1n(a12, b1, &r, multithread, mulDepth);

        // step 4: d=quotient*b2
        if (multithread)
            d = mulParallel(quotient, b2, mulDepth);
        else
            d = mulSingleThread(quotient, b2);
    } else {
        // step 3b: if a1>=b1, let quotient=beta^n-1 and r=a12-b1*2^n+b1
        quotient = newBigInt(n);
        memset(quotient->bigIntArray, 0xFF, n * 8);
        bigInt *a12_2 = add(a12, b1);
        bigInt *b1_2 = shiftLeft(b1, 64 * n);
        r = sub(a12_2, b1_2);
        freeBigInt(a12_2);
        freeBigInt(b1_2);

        // step 4: d=quotient*b2=(b2 << 64*n) - b2
        bigInt *d_0 = shiftLeft(b2, 64 * n);
        d = sub(d_0, b2);
        freeBigInt(d_0);
    }
    freeBigInt(a12);
    freeBigInt(b1);
    freeBigInt(b2);

    // step 5: r = r*beta^n + a3 - d (paper says a4)
    // However, don't subtract d until after the while loop so r doesn't become negative
    bigInt *r_1 = shiftLeft(r, 64 * n);
    freeBigInt(r);
    bigInt *a_lower = getLowerFrom(A, n);
    bigInt *r_2 = add(r_1, a_lower);
    freeBigInt(r_1);
    freeBigInt(a_lower);

    // step 6: add B until r>=d
    bigInt *one = getBigIntFromUnsignedInteger(1);
    while (compareBigInt(r_2, d) < 0) {
        bigInt *r_2_tmp = add(r_2, B);
        freeBigInt(r_2);
        r_2 = r_2_tmp;
        bigInt *qTmp = sub(quotient, one);
        freeBigInt(quotient);
        quotient = qTmp;
    }
    *reminder = sub(r_2, d);
    freeBigInt(r_2);
    freeBigInt(d);
    freeBigInt(one);
    return quotient;
}
