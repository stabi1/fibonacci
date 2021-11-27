#ifndef FIBONACCI_TESTS_H
#define FIBONACCI_TESTS_H

#include <stdbool.h>
#include "BigInt.h"

void test();

void benchMark();

void bruteForceDebug(bool multiThread);

bool compareBigInts(bigInt *x, bigInt *y);

#endif //FIBONACCI_TESTS_H
