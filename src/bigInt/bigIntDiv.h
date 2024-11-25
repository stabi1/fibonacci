#ifndef FIBONACCI_BIGINTDIV_H
#define FIBONACCI_BIGINTDIV_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bigIntMethods.h"
#include "bigIntUtil.h"
#include "bigIntAsm.h"
#include "mulAsm.h"

bigInt *exactDivideBy3(bigInt *x);

bigInt *divideHelper(bigInt *dividend, bigInt *divisor, bigInt **reminder, bool noBurnikelZiegler, bool multithread);

bigInt *divideD4Helper(bigInt *dividend, bigInt *divisor, bigInt **reminder);

bigInt *divideSingleThread(bigInt *dividend, bigInt *divisor);

bigInt *divideModSingleThread(bigInt *dividend, bigInt *divisor, bigInt **reminder);

void divideOneWord(bigInt *dividend, uint64_t divisor, bigInt *quotient, bigInt *reminder);

void divideD4(bigInt *dividend, bigInt *divisor, bigInt *quotient, bigInt *reminder);

bigInt *divideBurnikelZiegler(bigInt *A, bigInt *B, bigInt **reminder, bool multithread);

bigInt *divide2n1n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread);

bigInt *divide3n2n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread);

#endif //FIBONACCI_BIGINTDIV_H
