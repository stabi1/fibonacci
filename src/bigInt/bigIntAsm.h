#ifndef IMPLEMENTATION_BIGINTASM_H

#include "bigInt.h"

#define IMPLEMENTATION_BIGINTASM_H

bigInt *do_add_asm(bigInt *x, bigInt *y, bigInt *res);

bigInt *do_sub_asm(bigInt *x, bigInt *y, bigInt *res);

bigInt *shiftAdd_Asm(bigInt *x, bigInt *toShift, size_t n);

bigInt *shiftLeft_Asm(bigInt *x, size_t n);

bigInt *shiftRight_Asm(bigInt *x, size_t n);

#endif //IMPLEMENTATION_BIGINTASM_H
