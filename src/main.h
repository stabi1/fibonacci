#ifndef FIBONACCI_MAIN_H
#define FIBONACCI_MAIN_H

#include <setjmp.h>
#include "bigInt/bigInt.h"

bigInt* fibExpFastDoubling(uint64_t n);

bigInt *fibExpFastDoublingMultiThread(uint64_t n);

extern jmp_buf exceptionJump;

#endif //FIBONACCI_MAIN_H
