#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>
#include "bigIntMul.h"
#include "mulAsm.h"
#include "bigIntAsm.h"
#include "bigIntUtil.h"
#include "bigIntDiv.h"

size_t NAIVLEMULFASTER = 10; //Size when naiveMul is faster than karatsuba // 60 //TODO change back!!!
size_t KARATSUBAFASTER = 200; //Size when karatsuba is faster than toom-cook //200

void *multiplyToomCook3MultiThreadHelper(void *input);

bigInt *mulExecute(bigInt *x, bigInt *y);

bigInt *mul(bigInt *x, bigInt *y) {
    if (!isValidBigInt(x) || !isValidBigInt(y)) {
        fprintf(stderr, "Mul: invalid bigInts supplied\n");
        exit(4);
    }

    bigInt *res;
    if (getLen(x) < getLen(y)) {
        res = mulExecute(y, x);
    } else {
        res = mulExecute(x, y);
    }
    res->negative = x->negative ^ y->negative;
    return res;
}

bigInt *mulExecute(bigInt *x, bigInt *y) {
    //printf("karatsuba: %lu %lu\n", getLen(x), getLen(y));
    if (isZero(x) || isZero(y)) {
        return getZeroBigInt();
    }

    size_t yLen = getLen(y);
    if (yLen <= NAIVLEMULFASTER) {
        printf("naivemul: %lu %lu\n", getLen(x), getLen(y));
        bigInt *tmp = copyBigInt(y);
        bigInt *res = naiveMul_Asm(x, y);
        if (!isValidBigInt(res)) {
            if (compareBigInt(y, tmp) != 0) {
                printBigIntHex(tmp);
                printBigIntHex(stripLeadingZeros(y));
            }

            printBigIntHex(x);
            printBigIntHex(y);
            exit(0);
        }
        return res;
    } else {
        return karatsuba(x, y);
    }
    // TODO Restore
    /*size_t yLen = getLen(y);
    if (yLen <= NAIVLEMULFASTER) {
        return naiveMul_Asm(x, y);
    } else if (yLen <= KARATSUBAFASTER) {
        return karatsuba(x, y);
    } else {
        return multiplyToomCook3(x, y);
    }*/
}

//returns x * y with karatsuba
bigInt *karatsuba(bigInt *x, bigInt *y) {
    //termination condition
    size_t xLen = getLen(x);
    //calculate m -> middle of the bigger bigInt
    size_t m = xLen / 2;
    //karatsuba according to the algorithm
    bigInt *x0 = getLowerFrom(x, m);
    bigInt *x1 = getUpperFrom(x, m);
    bigInt *y0 = getLowerFrom(y, m);
    bigInt *y1 = getUpperFrom(y, m);

    bigInt *x0y0 = mul(x0, y0);
    bigInt *x1y1 = mul(x1, y1);

    bigInt *temp1 = add(x0, x1);
    freeBigInt(x0);
    freeBigInt(x1);
    bigInt *temp2 = add(y0, y1);
    freeBigInt(y0);
    freeBigInt(y1);
    bigInt *x0x1y01 = mul(temp1, temp2);
    freeBigInt(temp1);
    freeBigInt(temp2);

    bigInt *temp3 = sub(x0x1y01, x0y0);
    freeBigInt(x0x1y01);
    bigInt *b = sub(temp3, x1y1);
    freeBigInt(temp3);
    bigInt *temp4 = shiftAdd(x0y0, b, m);
    freeBigInt(b);
    freeBigInt(x0y0);
    bigInt *res = shiftAdd(temp4, x1y1, m * 2);
    freeBigInt(temp4);
    freeBigInt(x1y1);
    return res;
}

bigInt *multiplyToomCook3(bigInt *a, bigInt *b) {
    size_t largest = getLen(a);
    // k is the size (in qword) of the lower-order slices.
    size_t k = (largest + 2) / 3;
    // r is the size (in qword) of the highest-order slice.
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

    bigInt *v0 = mul(a0, b0);
    bigInt *da1 = add(a2, a0);
    bigInt *db1 = add(b2, b0);
    bigInt *temp1 = sub(db1, b1);
    bigInt *temp2 = sub(da1, a1);
    bigInt *vm1 = mul(temp1, temp2);
    freeBigInt(temp1);
    freeBigInt(temp2);

    bigInt *da2 = add(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    bigInt *db2 = add(db1, b1);
    freeBigInt(db1);
    freeBigInt(b1);
    bigInt *v1 = mul(da2, db2);
    bigInt *temp3 = add(da2, a2);
    freeBigInt(da2);
    bigInt *temp4 = shiftLeft_Asm(temp3, 1);
    freeBigInt(temp3);
    bigInt *temp5 = sub(temp4, a0);
    freeBigInt(temp4);
    freeBigInt(a0);
    bigInt *temp6 = add(db2, b2);
    freeBigInt(db2);
    bigInt *temp7 = shiftLeft_Asm(temp6, 1);
    freeBigInt(temp6);
    bigInt *temp8 = sub(temp7, b0);
    freeBigInt(temp7);
    freeBigInt(b0);
    bigInt *v2 = mul(temp5, temp8);
    freeBigInt(temp8);
    freeBigInt(temp5);
    bigInt *vInf = mul(a2, b2);
    freeBigInt(a2);
    freeBigInt(b2);

    bigInt *temp9 = sub(v2, vm1);
    freeBigInt(v2);
    bigInt *t2 = exactDivideBy3(temp9);
    freeBigInt(temp9);
    bigInt *temp10 = sub(v1, vm1);
    freeBigInt(vm1);
    bigInt *tm1 = shiftRight_Asm(temp10, 1);
    freeBigInt(temp10);
    bigInt *t1 = sub(v1, v0);
    freeBigInt(v1);
    bigInt *temp11 = sub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = sub(t1, tm1);
    freeBigInt(t1);
    bigInt *t1_2 = sub(temp12, vInf);
    freeBigInt(temp12);
    bigInt *temp13 = shiftLeft_Asm(vInf, 1);
    bigInt *t2_3 = sub(t2_2, temp13);
    freeBigInt(t2_2);
    freeBigInt(temp13);
    bigInt *tm2 = sub(tm1, t2_3);
    freeBigInt(tm1);

    bigInt *temp14 = shiftAdd(t2_3, vInf, k);
    freeBigInt(t2_3);
    freeBigInt(vInf);
    bigInt *temp15 = shiftAdd(t1_2, temp14, k);
    freeBigInt(temp14);
    freeBigInt(t1_2);
    bigInt *temp16 = shiftAdd(tm2, temp15, k);
    freeBigInt(temp15);
    freeBigInt(tm2);
    bigInt *result = shiftAdd(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);
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
    size_t aLen = a->end - a->start;
    size_t bLen = b->end - b->start;
    //if smaller than karatsubaFaster, use karatsuba
    if (aLen <= KARATSUBAFASTER || bLen <= KARATSUBAFASTER) {
        bigInt *res = karatsuba(a, b);
        res->negative = sign;
        return res;
    }


    struct toomCookArgs *args = malloc(sizeof(struct toomCookArgs));
    mallocCheck(args);
    args->a = a;
    args->b = b;
    args->depth = depth;
    bigInt *temp = multiplyToomCook3MultiThreadHelper((void *) args);
    free(args);
    return temp;
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
    size_t aLen = a->end - a->start;
    size_t bLen = b->end - b->start;
    //if smaller than karatsubaFaster, use karatsuba
    if (aLen <= KARATSUBAFASTER || bLen <= KARATSUBAFASTER) {
        bigInt *res = karatsuba(a, b);
        res->negative = sign;
        return res;
    }
    size_t largest;
    if (aLen < bLen) {
        largest = bLen;
    } else {
        largest = aLen;
    }

    // k is the size (in qword) of the lower-order slices.
    size_t k = (largest + 2) / 3;
    // r is the size (in qword) of the highest-order slice.
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

    struct toomCookArgs *argsMul1 = malloc(sizeof(struct toomCookArgs));
    mallocCheck(argsMul1);
    struct toomCookArgs *argsMul2 = malloc(sizeof(struct toomCookArgs));
    mallocCheck(argsMul2);
    struct toomCookArgs *argsMul3 = malloc(sizeof(struct toomCookArgs));
    mallocCheck(argsMul3);
    struct toomCookArgs *argsMul4 = malloc(sizeof(struct toomCookArgs));
    mallocCheck(argsMul4);
    struct toomCookArgs *argsMul5 = malloc(sizeof(struct toomCookArgs));
    mallocCheck(argsMul5);

    bigInt *v0;
    if (depth > 0) {
        argsMul1->a = a0;
        argsMul1->b = b0;
        argsMul1->depth = depth - 1;
        pthread_create(&thread_idMul1, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul1);
    } else {
        v0 = mul(a0, b0);
    }
    bigInt *da1 = add(a2, a0);
    bigInt *db1 = add(b2, b0);
    bigInt *temp1 = sub(db1, b1);
    bigInt *temp2 = sub(da1, a1);
    bigInt *vm1;
    if (depth > 0) {
        argsMul2->a = temp1;
        argsMul2->b = temp2;
        argsMul2->depth = depth - 1;
        pthread_create(&thread_idMul2, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul2);
    } else {
        vm1 = mul(temp1, temp2);
    }
    bigInt *da2 = add(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    bigInt *db2 = add(db1, b1);
    freeBigInt(db1);
    freeBigInt(b1);
    bigInt *v1;
    if (depth > 0) {
        argsMul3->a = da2;
        argsMul3->b = db2;
        argsMul3->depth = depth - 1;
        pthread_create(&thread_idMul3, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul3);
    } else {
        v1 = mul(da2, db2);
    }
    bigInt *temp3 = add(da2, a2);

    bigInt *temp4 = shiftLeft_Asm(temp3, 1);
    freeBigInt(temp3);
    bigInt *temp5 = sub(temp4, a0);
    freeBigInt(temp4);

    bigInt *temp6 = add(db2, b2);
    bigInt *temp7 = shiftLeft_Asm(temp6, 1);
    freeBigInt(temp6);
    bigInt *temp8 = sub(temp7, b0);
    freeBigInt(temp7);
    bigInt *v2;
    if (depth > 0) {
        argsMul4->a = temp5;
        argsMul4->b = temp8;
        argsMul4->depth = depth - 1;
        pthread_create(&thread_idMul4, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul4);
    } else {
        v2 = mul(temp5, temp8);
        freeBigInt(temp8);
        freeBigInt(temp5);
    }

    bigInt *vInf;
    if (depth > 0) {
        argsMul5->a = a2;
        argsMul5->b = b2;
        argsMul5->depth = depth - 1;
        pthread_create(&thread_idMul5, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul5);
    } else {
        vInf = mul(a2, b2);
        freeBigInt(a2);
        freeBigInt(b2);
    }

    if (depth > 0) {
        //join Threads
        void *temp;
        pthread_join(thread_idMul1, &temp);
        v0 = (bigInt *) temp;

        pthread_join(thread_idMul2, &temp);
        vm1 = (bigInt *) temp;

        pthread_join(thread_idMul3, &temp);
        v1 = (bigInt *) temp;

        pthread_join(thread_idMul4, &temp);
        v2 = (bigInt *) temp;
        freeBigInt(temp8);
        freeBigInt(temp5);

        pthread_join(thread_idMul5, &temp);
        vInf = (bigInt *) temp;
        freeBigInt(a2);
        freeBigInt(b2);
    }
    freeBigInt(temp1);
    freeBigInt(temp2);
    freeBigInt(a0);
    freeBigInt(b0);
    freeBigInt(da2);
    freeBigInt(db2);

    free(argsMul1);
    free(argsMul2);
    free(argsMul3);
    free(argsMul4);
    free(argsMul5);

    bigInt *temp9 = sub(v2, vm1);
    freeBigInt(v2);
    bigInt *t2 = exactDivideBy3(temp9);
    freeBigInt(temp9);
    bigInt *temp10 = sub(v1, vm1);
    freeBigInt(vm1);
    bigInt *tm1 = shiftRight_Asm(temp10, 1);
    freeBigInt(temp10);
    bigInt *t1 = sub(v1, v0);
    freeBigInt(v1);
    bigInt *temp11 = sub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = sub(t1, tm1);
    freeBigInt(t1);
    bigInt *t1_2 = sub(temp12, vInf);
    freeBigInt(temp12);
    bigInt *temp13 = shiftLeft_Asm(vInf, 1);
    bigInt *t2_3 = sub(t2_2, temp13);
    freeBigInt(t2_2);
    freeBigInt(temp13);
    bigInt *tm2 = sub(tm1, t2_3);
    freeBigInt(tm1);

    bigInt *temp14 = shiftAdd(t2_3, vInf, k);
    freeBigInt(t2_3);
    freeBigInt(vInf);
    bigInt *temp15 = shiftAdd(t1_2, temp14, k);
    freeBigInt(temp14);
    freeBigInt(t1_2);
    bigInt *temp16 = shiftAdd(tm2, temp15, k);
    freeBigInt(temp15);
    freeBigInt(tm2);
    bigInt *result = shiftAdd(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);

    result->negative = sign;
    return (void *) result;
}