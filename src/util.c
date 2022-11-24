#include "util.h"
#include <stddef.h>
#include <stdint-gcc.h>
#include <ctype.h>

bool checkIsNumber(char *value) {
    char *x;
    for (x = value; *x; x++) {
        if (!isdigit(*x))
            return false;
    }
    return true;
}

size_t custom_lzcnt(uint64_t n) {
    if (n == 0) {
        return 0;
    }
    return __builtin_clzll(n);
}
