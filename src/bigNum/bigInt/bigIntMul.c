#include <stddef.h>
#include <pthread.h>
#include "bigIntMul.h"
#include "mulAsm.h"
#include "bigIntAsm.h"
#include "bigIntDiv.h"
#include "../config.h"
#include "bigIntIO.h"

size_t NAIVEMUL_FASTER = 100; //Size when naiveMul is faster than karatsuba
size_t KARATSUBA_FASTER = 400; //Size when karatsuba is faster than toom-cook
size_t PARALLEL_MUL_FASTER = 1000; //Size when toom-cook-multithread is faster than toom-cook

void *multiplyToomCook3MultiThreadHelper(void *input);

bigInt *mulExecute(const bigInt *x, const bigInt *y);

bigInt *mulParallelExecute(const bigInt *x, const bigInt *y, size_t depth);

bigInt *mul(const bigInt *x, const bigInt *y) {
    if (global_config.parallel)
        return mulParallel(x, y, global_config.mulDepth);
    else
        return mulSingleThread(x, y);
}

bigInt *mulSingleThread(const bigInt *x, const bigInt *y) {
    bigInt *res;
    if (getLen(x) < getLen(y)) {
        res = mulExecute(y, x);
    } else {
        res = mulExecute(x, y);
    }
    res->negative = x->negative ^ y->negative;
    return res;
}

bigInt *mulParallel(const bigInt *x, const bigInt *y, size_t depth) {
    if (depth == 0) return mulSingleThread(x, y);

    bigInt *res;
    if (getLen(x) < getLen(y)) {
        res = mulParallelExecute(y, x, depth);
    } else {
        res = mulParallelExecute(x, y, depth);
    }
    res->negative = x->negative ^ y->negative;
    return res;
}

bigInt *mulExecute(const bigInt *x, const bigInt *y) {
    if (isZero(x) || isZero(y)) {
        return getZeroBigInt();
    }

    size_t yLen = getLen(y);
    if (yLen <= NAIVEMUL_FASTER) {
        return naiveMul_Asm(x, y);
    } else if (yLen <= KARATSUBA_FASTER) {
        return karatsuba(x, y);
    } else {
        return multiplyToomCook3(x, y);
    }
}

bigInt *mulParallelExecute(const bigInt *x, const bigInt *y, size_t depth) {
    if (isZero(x) || isZero(y)) {
        return getZeroBigInt();
    }

    size_t yLen = getLen(y);
    if (yLen <= NAIVEMUL_FASTER) {
        return naiveMul_Asm(x, y);
    } else if (yLen <= KARATSUBA_FASTER) {
        return karatsuba(x, y);
    } else if (yLen <= PARALLEL_MUL_FASTER) {
        return multiplyToomCook3(x, y);
    } else {
        return multiplyToomCook3MultiThread(x, y, depth);
    }
}

//returns x * y with karatsuba
bigInt *karatsuba(const bigInt *x, const bigInt *y) {
    size_t xLen = getLen(x);
    //calculate m -> middle of the bigger bigInt
    size_t m = xLen / 2;
    //karatsuba according to the algorithm
    bigInt *x0 = getLowerFrom(x, m);
    bigInt *x1 = getUpperFrom(x, m);
    bigInt *y0 = getLowerFrom(y, m);
    bigInt *y1 = getUpperFrom(y, m);

    bigInt *x0y0 = mulSingleThread(x0, y0);
    bigInt *x1y1 = mulSingleThread(x1, y1);

    bigInt *temp1 = add(x0, x1);
    freeBigInt(x0);
    freeBigInt(x1);
    bigInt *temp2 = add(y0, y1);
    freeBigInt(y0);
    freeBigInt(y1);
    bigInt *x0x1y01 = mulSingleThread(temp1, temp2);
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

bigInt *multiplyToomCook3(const bigInt *a, const bigInt *b) {
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

    // in swap (not used in mul): nothing
    bigInt *v0 = mulSingleThread(a0, b0);
    char *filename_v0 = storeBigIntInSwap(v0);
    bigInt *da1 = add(a2, a0);
    bigInt *db1 = add(b2, b0);
    bigInt *temp1 = sub(db1, b1);
    char *filename_db1 = storeBigIntInSwap(db1);
    bigInt *temp2 = sub(da1, a1);
    char *filename_da1 = storeBigIntInSwap(da1);
    // in swap (not used in mul): v0, da1 db1
    bigInt *vm1 = mulSingleThread(temp1, temp2);
    char *filename_vm1 = storeBigIntInSwap(vm1);
    freeBigInt(temp1);
    freeBigInt(temp2);

    da1 = loadBigIntFromSwap(da1, filename_da1);
    bigInt *da2 = add(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    db1 = loadBigIntFromSwap(db1, filename_db1);
    bigInt *db2 = add(db1, b1);
    freeBigInt(b1);
    freeBigInt(db1);
    // in swap (not used in mul): v0, vm1
    bigInt *v1 = mulSingleThread(da2, db2);
    char *filename_v1 = storeBigIntInSwap(v1);
    bigInt *temp3 = add(da2, a2);
    freeBigInt(da2);
    bigInt *temp4 = shiftLeft_Asm(temp3, 1);
    freeBigInt(temp3);
    bigInt *temp5 = sub(temp4, a0);
    freeBigInt(a0);
    freeBigInt(temp4);
    bigInt *temp6 = add(db2, b2);
    freeBigInt(db2);
    bigInt *temp7 = shiftLeft_Asm(temp6, 1);
    freeBigInt(temp6);
    bigInt *temp8 = sub(temp7, b0);
    freeBigInt(b0);
    freeBigInt(temp7);
    // in swap (not used in mul): v0, vm1
    bigInt *v2 = mulSingleThread(temp5, temp8);
    freeBigInt(temp8);
    freeBigInt(temp5);
    char *filename_v2 = storeBigIntInSwap(v2);
    // in swap (not used in mul): v0, vm1, v2
    bigInt *vInf = mulSingleThread(a2, b2);
    freeBigInt(a2);
    freeBigInt(b2);
    char *filename_vInf = storeBigIntInSwap(vInf);
    vm1 = loadBigIntFromSwap(vm1, filename_vm1);
    v2 = loadBigIntFromSwap(v2, filename_v2);
    bigInt *temp9 = sub(v2, vm1);
    freeBigInt(v2);
    bigInt *t2 = exactDivideBy3(temp9);
    freeBigInt(temp9);
    v1 = loadBigIntFromSwap(v1, filename_v1);
    bigInt *temp10 = sub(v1, vm1);
    freeBigInt(vm1);
    bigInt *tm1 = shiftRight_Asm(temp10, 1);
    freeBigInt(temp10);
    v0 = loadBigIntFromSwap(v0, filename_v0);
    bigInt *t1 = sub(v1, v0);
    filename_v0 = storeBigIntInSwap(v0);
    freeBigInt(v1);
    bigInt *temp11 = sub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = sub(t1, tm1);
    freeBigInt(t1);
    vInf = loadBigIntFromSwap(vInf, filename_vInf);
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
    v0 = loadBigIntFromSwap(v0, filename_v0);
    bigInt *result = shiftAdd(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);

    return result;
}

struct toomCookArgs {
    const bigInt *a;
    const bigInt *b;
    size_t depth;
};

struct toomCookReturn {
    bigInt *res;
    char *res_filename;
};

bigInt *multiplyToomCook3MultiThread(const bigInt *a, const bigInt *b, size_t depth) {
    struct toomCookArgs *args = malloc(sizeof(struct toomCookArgs));
    mallocCheck(args);
    args->a = a;
    args->b = b;
    args->depth = depth;
    struct toomCookReturn *temp = multiplyToomCook3MultiThreadHelper((void *) args);
    free(args);
    bigInt *res = loadBigIntFromSwap(temp->res, temp->res_filename);
    free(temp);
    return res;
}

void *multiplyToomCook3MultiThreadHelper(void *input) {
    const bigInt *a = ((struct toomCookArgs *) input)->a;
    const bigInt *b = ((struct toomCookArgs *) input)->b;
    size_t depth = ((struct toomCookArgs *) input)->depth;

    if (depth == 0) {
        bigInt *result = mulSingleThread(a, b);
        struct toomCookReturn *ret = malloc(sizeof(struct toomCookReturn));
        mallocCheck(ret);
        ret->res = result;
        ret->res_filename = storeBigIntInSwap(result);
        return (void *) ret;
    }

    size_t aLen = a->end - a->start;
    size_t bLen = b->end - b->start;

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
    argsMul1->a = a0;
    argsMul1->b = b0;
    argsMul1->depth = depth - 1;
    if (pthread_create(&thread_idMul1, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul1) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }

    bigInt *da1 = add(a2, a0);
    bigInt *db1 = add(b2, b0);
    bigInt *temp1 = sub(db1, b1);
    bigInt *temp2 = sub(da1, a1);

    bigInt *vm1;
    argsMul2->a = temp1;
    argsMul2->b = temp2;
    argsMul2->depth = depth - 1;
    if (pthread_create(&thread_idMul2, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul2) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }

    bigInt *da2 = add(da1, a1);
    freeBigInt(da1);
    freeBigInt(a1);
    bigInt *db2 = add(db1, b1);
    freeBigInt(db1);
    freeBigInt(b1);

    bigInt *v1;
    argsMul3->a = da2;
    argsMul3->b = db2;
    argsMul3->depth = depth - 1;
    if (pthread_create(&thread_idMul3, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul3) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
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
    argsMul4->a = temp5;
    argsMul4->b = temp8;
    argsMul4->depth = depth - 1;
    if (pthread_create(&thread_idMul4, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul4) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }

    bigInt *vInf;
    argsMul5->a = a2;
    argsMul5->b = b2;
    argsMul5->depth = depth - 1;
    if (pthread_create(&thread_idMul5, NULL, multiplyToomCook3MultiThreadHelper, (void *) argsMul5) != 0) {
        perror("Error creating thread!");
        exit(EXIT_FAILURE);
    }

    //join Threads
    void *temp;
    if (pthread_join(thread_idMul1, &temp) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    struct toomCookReturn *ret_v0 = temp;
    freeBigInt(a0);
    freeBigInt(b0);

    if (pthread_join(thread_idMul2, &temp) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    struct toomCookReturn *ret = temp;
    vm1 = loadBigIntFromSwap(ret->res, ret->res_filename);
    free(ret);
    freeBigInt(temp1);
    freeBigInt(temp2);

    if (pthread_join(thread_idMul3, &temp) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    ret = temp;
    v1 = loadBigIntFromSwap(ret->res, ret->res_filename);
    free(ret);
    freeBigInt(da2);
    freeBigInt(db2);

    if (pthread_join(thread_idMul4, &temp) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    ret = temp;
    v2 = loadBigIntFromSwap(ret->res, ret->res_filename);
    free(ret);
    freeBigInt(temp8);
    freeBigInt(temp5);

    if (pthread_join(thread_idMul5, &temp) != 0) {
        perror("Error joining thread!");
        exit(EXIT_FAILURE);
    }
    struct toomCookReturn *ret_vInf = temp;
    freeBigInt(a2);
    freeBigInt(b2);

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
    v0 = loadBigIntFromSwap(ret_v0->res, ret_v0->res_filename);
    free(ret_v0);
    bigInt *t1 = sub(v1, v0);
    char *filename_v0 = storeBigIntInSwap(v0);
    freeBigInt(v1);
    bigInt *temp11 = sub(t2, t1);
    freeBigInt(t2);
    bigInt *t2_2 = shiftRight_Asm(temp11, 1);
    freeBigInt(temp11);
    bigInt *temp12 = sub(t1, tm1);
    freeBigInt(t1);
    vInf = loadBigIntFromSwap(ret_vInf->res, ret_vInf->res_filename);
    free(ret_vInf);
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
    v0 = loadBigIntFromSwap(v0, filename_v0);
    bigInt *result = shiftAdd(v0, temp16, k);
    freeBigInt(temp16);
    freeBigInt(v0);

    struct toomCookReturn *returnStruct = malloc(sizeof(struct toomCookReturn));
    mallocCheck(returnStruct);
    returnStruct->res = result;
    if (depth == global_config.mulDepth) {
        size_t l = strlen(NOT_STORED);
        char *res = malloc(l + 1);
        mallocCheck(res);
        strncpy(res, NOT_STORED, l + 1);
        returnStruct->res_filename = res;
    } else {
        returnStruct->res_filename = storeBigIntInSwap(result);
    }
    return (void *) returnStruct;
}