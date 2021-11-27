#include "mul.h"
#include <stddef.h>
#include <immintrin.h>
#include <pthread.h>
#include <stdbool.h>
#include "mulAsm.h"
#include "BigIntAsm.h"

size_t naivMulFaster = 100; //Size when naivMul is faster than karatsuba // 100
size_t karatsubaFaster = 400; //Size when karatsuba is faster than toom-cook //400

void *multiplyToomCook3MultiThreadHelper(void *input);

//returns x * y with karatsuba
bigInt *karatsuba(bigInt *x, bigInt *y) {
    //termination condition
    size_t xLen = x->end - x->start;
    size_t yLen = y->end - y->start;
    //if smaller than naivMulFaster, use naivMul
    if (xLen <= naivMulFaster || yLen <= naivMulFaster) {
        if (xLen > yLen) {
            return naiveMul_Asm(x, y);
        } else {
            return naiveMul_Asm(y, x);
        }
    }
    //calulate m -> middle of the bigger bigInt
    size_t m;
    if (xLen > yLen) {
        m = xLen / 2;
    } else {
        m = yLen / 2;
    }
    //karatsuba according to the algorithm
    bigInt *x0 = getLowerFrom(x, x->start + m);
    bigInt *x1 = getUpperFrom(x, x->start + m);
    bigInt *y0 = getLowerFrom(y, y->start + m);
    bigInt *y1 = getUpperFrom(y, y->start + m);

    bigInt *x0y0 = karatsuba(x0, y0);
    bigInt *x1y1 = karatsuba(x1, y1);

    bigInt *temp1 = smartAdd(x0, x1);
    freeBigInt(x0);
    freeBigInt(x1);
    bigInt *temp2 = smartAdd(y0, y1);
    freeBigInt(y0);
    freeBigInt(y1);
    bigInt *x0x1y01 = karatsuba(temp1, temp2);
    freeBigInt(temp1);
    freeBigInt(temp2);

    bigInt *temp3 = smartSub(x0x1y01, x0y0);
    freeBigInt(x0x1y01);
    bigInt *b = smartSub(temp3, x1y1);
    freeBigInt(temp3);
    bigInt *temp4 = shiftAdd_Asm(x0y0, b, m);
    freeBigInt(b);
    freeBigInt(x0y0);
    bigInt *res = shiftAdd_Asm(temp4, x1y1, m * 2);
    freeBigInt(temp4);
    freeBigInt(x1y1);
    return res;
}

bigInt *multiplyToomCook3(bigInt *a, bigInt *b) {
    bool sign;
    if ((a->negative && b->negative) || (!a->negative && !b->negative)) {
        sign = false;
    } else {
        sign = true;
    }
    size_t alen = a->end - a->start;
    size_t blen = b->end - b->start;
    //if smaller than karatsubaFaster, use karatsuba
    if (alen <= karatsubaFaster || blen <= karatsubaFaster) {
        bigInt *res = karatsuba(a, b);
        res->negative = sign;
        return res;
    }
    size_t largest;
    if (alen < blen) {
        largest = blen;
    } else {
        largest = alen;
    }

    // k is the size (in qwords) of the lower-order slices.
    size_t k = (largest + 2) / 3;
    // r is the size (in qwords) of the highest-order slice.
    size_t r = largest - 2 * k;

    // Obtain slices of the numbers. a2 and b2 are the most significant
    // bits of the numbers a and b, and a0 and b0 the least significant.
    bigInt *slicesA[3];
    bigInt *slicesB[3];
    getToomSlice(a, k, r, largest, slicesA);
    getToomSlice(b, k, r, largest, slicesB);
    bigInt *a2 = slicesA[2];
    bigInt *a1 = slicesA[1];
    bigInt *a0 = slicesA[0];
    bigInt *b2 = slicesB[2];
    bigInt *b1 = slicesB[1];
    bigInt *b0 = slicesB[0];

    bigInt *v0 = multiplyToomCook3(a0, b0);
    bigInt *da1 = smartAdd(a2, a0);
    bigInt *db1 = smartAdd(b2, b0);
    bigInt *temp1 = smartSub(db1, b1);
    bigInt *temp2 = smartSub(da1, a1);
    bigInt *vm1 = multiplyToomCook3(temp1, temp2);
    freeBigInt(temp1);
    freeBigInt(temp2);

    bigInt *da2 = smartAdd(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    bigInt *db2 = smartAdd(db1, b1);
    freeBigInt(db1);
    freeBigInt(b1);
    bigInt *v1 = multiplyToomCook3(da2, db2);
    bigInt *temp3 = smartAdd(da2, a2);
    freeBigInt(da2);
    bigInt *temp4 = shiftLeft_Asm(temp3, 1);
    freeBigInt(temp3);
    bigInt *temp5 = smartSub(temp4, a0);
    freeBigInt(temp4);
    freeBigInt(a0);
    bigInt *temp6 = smartAdd(db2, b2);
    freeBigInt(db2);
    bigInt *temp7 = shiftLeft_Asm(temp6, 1);
    freeBigInt(temp6);
    bigInt *temp8 = smartSub(temp7, b0);
    freeBigInt(temp7);
    freeBigInt(b0);
    bigInt *v2 = multiplyToomCook3(temp5, temp8);
    freeBigInt(temp8);
    freeBigInt(temp5);
    bigInt *vinf = multiplyToomCook3(a2, b2);
    freeBigInt(a2);
    freeBigInt(b2);

    bigInt *temp9 = smartSub(v2, vm1);
    freeBigInt(v2);
    bigInt *t2 = exactDivideBy3(temp9);
    freeBigInt(temp9);
    bigInt *temp10 = smartSub(v1, vm1);
    freeBigInt(vm1);
    bigInt *tm1 = shiftRight_Asm(temp10, 1);
    freeBigInt(temp10);
    bigInt *t1 = smartSub(v1, v0);
    freeBigInt(v1);
    bigInt *temp11 = smartSub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = smartSub(t1, tm1);
    freeBigInt(t1);
    bigInt *t1_2 = smartSub(temp12, vinf);
    freeBigInt(temp12);
    bigInt *temp13 = shiftLeft_Asm(vinf, 1);
    bigInt *t2_3 = smartSub(t2_2, temp13);
    freeBigInt(t2_2);
    freeBigInt(temp13);
    bigInt *tm2 = smartSub(tm1, t2_3);
    freeBigInt(tm1);

    bigInt *temp14 = shiftAdd_Asm(t2_3, vinf, k);
    freeBigInt(t2_3);
    freeBigInt(vinf);
    bigInt *temp15 = shiftAdd_Asm(t1_2, temp14, k);
    freeBigInt(temp14);
    freeBigInt(t1_2);
    bigInt *temp16 = shiftAdd_Asm(tm2, temp15, k);
    freeBigInt(temp15);
    freeBigInt(tm2);
    bigInt *result = shiftAdd_Asm(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);

    result->negative = sign;
    return result;
}

struct toomCookArgs {
    bigInt *a;
    bigInt *b;
    size_t depth;
};

bigInt *multiplyToomCook3MultiThread(bigInt *a, bigInt *b, size_t depth) {
    bool sign;
    if ((a->negative && b->negative) || (!a->negative && !b->negative)) {
        sign = false;
    } else {
        sign = true;
    }
    size_t alen = a->end - a->start;
    size_t blen = b->end - b->start;
    //if smaller than karatsubaFaster, use karatsuba
    if (alen <= karatsubaFaster || blen <= karatsubaFaster) {
        bigInt *res = karatsuba(a, b);
        res->negative = sign;
        return res;
    }


    struct toomCookArgs *args = malloc(sizeof(struct toomCookArgs));
    args->a = a;
    args->b = b;
    args->depth = depth;
    return (bigInt *) multiplyToomCook3MultiThreadHelper((void *) args);
}

void *multiplyToomCook3MultiThreadHelper(void *input) {
    bigInt *a = ((struct toomCookArgs *) input)->a;
    bigInt *b = ((struct toomCookArgs *) input)->b;
    size_t depth = ((struct toomCookArgs *) input)->depth;
    bool sign;
    if ((a->negative && b->negative) || (!a->negative && !b->negative)) {
        sign = false;
    } else {
        sign = true;
    }
    size_t alen = a->end - a->start;
    size_t blen = b->end - b->start;
    //if smaller than karatsubaFaster, use karatsuba
    if (alen <= karatsubaFaster || blen <= karatsubaFaster) {
        bigInt *res = karatsuba(a, b);
        res->negative = sign;
        return res;
    }
    size_t largest;
    if (alen < blen) {
        largest = blen;
    } else {
        largest = alen;
    }

    // k is the size (in qwords) of the lower-order slices.
    size_t k = (largest + 2) / 3;
    // r is the size (in qwords) of the highest-order slice.
    size_t r = largest - 2 * k;

    // Obtain slices of the numbers. a2 and b2 are the most significant
    // bits of the numbers a and b, and a0 and b0 the least significant.
    bigInt *slicesA[3];
    bigInt *slicesB[3];
    getToomSlice(a, k, r, largest, slicesA);
    getToomSlice(b, k, r, largest, slicesB);
    bigInt *a2 = slicesA[2];
    bigInt *a1 = slicesA[1];
    bigInt *a0 = slicesA[0];
    bigInt *b2 = slicesB[2];
    bigInt *b1 = slicesB[1];
    bigInt *b0 = slicesB[0];

    pthread_t thread_idMul1;
    pthread_t thread_idMul2;
    pthread_t thread_idMul3;
    pthread_t thread_idMul4;
    pthread_t thread_idMul5;

    bigInt *v0;
    if (depth > 0) {
        struct toomCookArgs *argsMul1 = malloc(sizeof(struct toomCookArgs));
        argsMul1->a = a0;
        argsMul1->b = b0;
        argsMul1->depth = depth - 1;
        pthread_create(&thread_idMul1, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul1);
    } else {
        v0 = multiplyToomCook3(a0, b0);
    }
    bigInt *da1 = smartAdd(a2, a0);
    bigInt *db1 = smartAdd(b2, b0);
    bigInt *temp1 = smartSub(db1, b1);
    bigInt *temp2 = smartSub(da1, a1);
    bigInt *vm1;
    if (depth > 0) {
        struct toomCookArgs *argsMul2 = malloc(sizeof(struct toomCookArgs));
        argsMul2->a = temp1;
        argsMul2->b = temp2;
        argsMul2->depth = depth - 1;
        pthread_create(&thread_idMul2, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul2);
    } else {
        vm1 = multiplyToomCook3(temp1, temp2);
    }
    bigInt *da2 = smartAdd(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    bigInt *db2 = smartAdd(db1, b1);
    freeBigInt(db1);
    freeBigInt(b1);
    bigInt *v1;
    if (depth > 0) {
        struct toomCookArgs *argsMul3 = malloc(sizeof(struct toomCookArgs));
        argsMul3->a = da2;
        argsMul3->b = db2;
        argsMul3->depth = depth - 1;
        pthread_create(&thread_idMul3, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul3);
    } else {
        v1 = multiplyToomCook3(da2, db2);
    }
    bigInt *temp3 = smartAdd(da2, a2);

    bigInt *temp4 = shiftLeft_Asm(temp3, 1);
    freeBigInt(temp3);
    bigInt *temp5 = smartSub(temp4, a0);
    freeBigInt(temp4);

    bigInt *temp6 = smartAdd(db2, b2);
    bigInt *temp7 = shiftLeft_Asm(temp6, 1);
    freeBigInt(temp6);
    bigInt *temp8 = smartSub(temp7, b0);
    freeBigInt(temp7);
    bigInt *v2;
    if (depth > 0) {
        struct toomCookArgs *argsMul4 = malloc(sizeof(struct toomCookArgs));
        argsMul4->a = temp5;
        argsMul4->b = temp8;
        argsMul4->depth = depth - 1;
        pthread_create(&thread_idMul4, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul4);
    } else {
        v2 = multiplyToomCook3(temp5, temp8);
        freeBigInt(temp8);
        freeBigInt(temp5);
    }

    bigInt *vinf;
    if (depth > 0) {
        struct toomCookArgs *argsMul5 = malloc(sizeof(struct toomCookArgs));
        argsMul5->a = a2;
        argsMul5->b = b2;
        argsMul5->depth = depth - 1;
        pthread_create(&thread_idMul5, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul5);
    } else {
        vinf = multiplyToomCook3(a2, b2);
        freeBigInt(a2);
        freeBigInt(b2);
    }

    if (depth > 0) {
        //join Threads
        void *temp;
        pthread_join(thread_idMul1, &temp);
        v0 = (bigInt *) temp;
        freeBigInt(a0);
        freeBigInt(b0);

        pthread_join(thread_idMul2, &temp);
        vm1 = (bigInt *) temp;
        freeBigInt(temp1);
        freeBigInt(temp2);

        pthread_join(thread_idMul3, &temp);
        v1 = (bigInt *) temp;
        freeBigInt(da2);
        freeBigInt(db2);

        pthread_join(thread_idMul4, &temp);
        v2 = (bigInt *) temp;
        freeBigInt(temp8);
        freeBigInt(temp5);

        pthread_join(thread_idMul5, &temp);
        vinf = (bigInt *) temp;
        freeBigInt(a2);
        freeBigInt(b2);
    }

    bigInt *temp9 = smartSub(v2, vm1);
    freeBigInt(v2);
    bigInt *t2 = exactDivideBy3(temp9);
    freeBigInt(temp9);
    bigInt *temp10 = smartSub(v1, vm1);
    freeBigInt(vm1);
    bigInt *tm1 = shiftRight_Asm(temp10, 1);
    freeBigInt(temp10);
    bigInt *t1 = smartSub(v1, v0);
    freeBigInt(v1);
    bigInt *temp11 = smartSub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = smartSub(t1, tm1);
    freeBigInt(t1);
    bigInt *t1_2 = smartSub(temp12, vinf);
    freeBigInt(temp12);
    bigInt *temp13 = shiftLeft_Asm(vinf, 1);
    bigInt *t2_3 = smartSub(t2_2, temp13);
    freeBigInt(t2_2);
    freeBigInt(temp13);
    bigInt *tm2 = smartSub(tm1, t2_3);
    freeBigInt(tm1);

    bigInt *temp14 = shiftAdd_Asm(t2_3, vinf, k);
    freeBigInt(t2_3);
    freeBigInt(vinf);
    bigInt *temp15 = shiftAdd_Asm(t1_2, temp14, k);
    freeBigInt(temp14);
    freeBigInt(t1_2);
    bigInt *temp16 = shiftAdd_Asm(tm2, temp15, k);
    freeBigInt(temp15);
    freeBigInt(tm2);
    bigInt *result = shiftAdd_Asm(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);

    result->negative = sign;
    return (void *) result;
}