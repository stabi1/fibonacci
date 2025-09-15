#ifndef FIBONACCI_BIGINTDIV_H
#define FIBONACCI_BIGINTDIV_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bigIntMethods.h"
#include "bigIntString.h"
#include "bigIntAsm.h"
#include "mulAsm.h"

bigInt *exactDivideBy3(const bigInt *x);

bigInt *divideBy10Raised19(bigInt *dividend, uint64_t *reminder);

bigInt *divideHelper(bigInt *dividend, bigInt *divisor, bigInt **reminder, bool noBurnikelZiegler, bool multithread, size_t mulDepth, bool freeArguments);

bigInt *divideD4Helper(bigInt *dividend, bigInt *divisor, bigInt **reminder);

bigInt *divideSingleThread(bigInt *dividend, bigInt *divisor);

bigInt *divideModSingleThread(bigInt *dividend, bigInt *divisor, bigInt **reminder, bool freeArguments);

bigInt *divideModMultiThread(bigInt *dividend, bigInt *divisor, bigInt **reminder, size_t mulDepth, bool freeArguments);

void divideOneWord(bigInt *dividend, uint64_t divisor, bigInt *quotient, bigInt *reminder);

void divideD4(const bigInt *dividend, const bigInt *divisor, bigInt *quotient, bigInt *reminder);

bigInt *divideBurnikelZiegler(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth, bool freeArguments);

bigInt *divide2n1n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth);

bigInt *divide3n2n(bigInt *A, bigInt *B, bigInt **reminder, bool multithread, size_t mulDepth);

#endif //FIBONACCI_BIGINTDIV_H
