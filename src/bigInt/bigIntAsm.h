#ifndef IMPLEMENTATION_BIGINTASM_H
#define IMPLEMENTATION_BIGINTASM_H

#include "bigInt.h"

bigInt *add_Asm(bigInt *x, bigInt *y, bool negative);

bigInt *shiftAdd_Asm(bigInt *x, bigInt *toShift, size_t n);

bigInt *sub_Asm(bigInt *x, bigInt *y, bool negative);

bigInt *shiftLeft_Asm(bigInt *x, size_t n);

bigInt *shiftRight_Asm(bigInt *x, size_t n);

#endif //IMPLEMENTATION_BIGINTASM_H
