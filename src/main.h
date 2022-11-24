#ifndef FIBONACCI_MAIN_H
#define FIBONACCI_MAIN_H

#include <setjmp.h>
#include "bigInt/bigInt.h"

bigInt *fibExpFastDoubling(uint64_t n, bool multiThread);

extern jmp_buf exceptionJump;

#endif //FIBONACCI_MAIN_H
