#ifndef IMPLEMENTATION_BIGINTASM_H
#define IMPLEMENTATION_BIGINTASM_H

#include "bigInt.h"

void shiftAddSameNumber_Asm(bigInt *x, const bigInt *toShift, size_t n);

bigInt *do_add_asm(const bigInt *x, const bigInt *y, bigInt *res);

bigInt *do_sub_asm(const bigInt *x, const bigInt *y, bigInt *res);

bigInt *shiftAdd_Asm(const bigInt *x, const bigInt *toShift, size_t n);

bigInt *shiftLeft_Asm(const bigInt *x, size_t n);

bigInt *shiftRight_Asm(const bigInt *x, size_t n);

#endif //IMPLEMENTATION_BIGINTASM_H
