#include "util.h"
#include <stddef.h>
#include <stdint-gcc.h>

size_t custom_lzcnt(uint64_t n) {
    if (n == 0) {
        return 0;
    }
    return __builtin_clzll(n);
}