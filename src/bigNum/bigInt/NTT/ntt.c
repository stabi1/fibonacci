#include "ntt.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../bigIntMethods.h"
#include "../../misc.h"

#define POW2(index) ((size_t)1 << (index))

// NTT adapted from https://github.com/SheafificationOfG/Fibsonisheaf

// bit-reverse index, increment, and then bit-reverse the result
// (assumes top_set_bit is the top set bit of the max index)
static size_t bit_reversed_increment(size_t index, size_t top_set_bit) {
    while (index & (top_set_bit >>= 1))
        index ^= top_set_bit;
    return index | top_set_bit;
}

static void spread_twice(uint16_t *restrict const src_a, uint16_t *restrict const src_b, uint64_t *restrict const dst_a, uint64_t *restrict const dst_b, size_t const len,
                         size_t const top_bit) {
    for (size_t i = 0, ri = 0; i < len;
         ++i, ri = bit_reversed_increment(ri, top_bit)) {
        dst_a[ri] = (uint64_t) src_a[i];
        dst_b[ri] = (uint64_t) src_b[i];
        src_a[i] = 0;
        src_b[i] = 0;
    }
}

static void ntt_twice(uint64_t *restrict const a, uint64_t *restrict const b,
                      uint64_t const *restrict const omega,
                      size_t const len_log) {
    for (size_t s2 = 0; s2 < len_log; ++s2) {
        size_t m2 = POW2(s2);
        size_t m = m2 << 1;
        uint64_t root_of_unity_local = omega[s2 + 1];
        for (size_t k = 0; k < POW2(len_log); k += m) {
            uint64_t coef = 1;
            for (size_t j = 0; j < m2; ++j) {
                uint64_t even_a = a[k + j];
                uint64_t even_b = b[k + j];
                uint64_t odd_a = mul_mod(coef, a[k + j + m2]);
                uint64_t odd_b = mul_mod(coef, b[k + j + m2]);

                a[k + j] = add_mod(even_a, odd_a);
                b[k + j] = add_mod(even_b, odd_b);
                a[k + j + m2] = sub_mod(even_a, odd_a);
                b[k + j + m2] = sub_mod(even_b, odd_b);

                coef = mul_mod(coef, root_of_unity_local);
            }
        }
    }
}

static void ntt_once(uint64_t *restrict const a,
                     uint64_t const *restrict const omega,
                     size_t const len_log) {
    for (size_t s2 = 0; s2 < len_log; ++s2) {
        size_t m2 = POW2(s2);
        size_t m = m2 << 1;
        uint64_t root_of_unity_local = omega[s2 + 1];
        for (size_t k = 0; k < POW2(len_log); k += m) {
            uint64_t coef = 1;
            for (size_t j = 0; j < m2; ++j) {
                uint64_t even_a = a[k + j];
                uint64_t odd_a = mul_mod(coef, a[k + j + m2]);

                a[k + j] = add_mod(even_a, odd_a);
                a[k + j + m2] = sub_mod(even_a, odd_a);

                coef = mul_mod(coef, root_of_unity_local);
            }
        }
    }
}

void fold(uint64_t const *restrict const src_b,
          uint16_t *restrict const dst_b, size_t const len,
          size_t const len_log) {
    uint64_t b_carry = 0;
    for (size_t i = 0; i < len; ++i) {
        uint64_t *b_window = (uint64_t *) &dst_b[i];
        b_carry = __builtin_add_overflow(*b_window, b_carry, b_window);
        b_carry += __builtin_add_overflow(*b_window, mul_mod(src_b[i], power_of_half[len_log]), b_window);
    }
}

static size_t ceil_log2(size_t n) {
    if (n <= 1) return 0;
    size_t k = 0;
    size_t v = 1;
    while (v < n) {
        ++k;
        v <<= 1;
    }
    return k;
}

bigInt *ntt_mul(const bigInt *A, const bigInt *B) {
    if (getLen(A) < getLen(B)) {
        const bigInt *tmp = A;
        A = B;
        B = tmp;
    } // A is longer now

    if(getLen(A) > 1) {} // TODO: max length

    // 1) calculate constants
    size_t A_16_len = getLen(A) * 4;
    size_t B_16_len = getLen(B) * 4;
    size_t res_16_Len = A_16_len + B_16_len;

    size_t const digit_len = res_16_Len;
    size_t const len_radix = A_16_len;
    size_t const len_log_radix = ceil_log2(digit_len);
    size_t const len_radix_max = POW2(len_log_radix);

    // 2) radix-buffers
    uint16_t *a = calloc(len_radix_max, sizeof *a);
    uint16_t *b = calloc(len_radix_max, sizeof *b);
    mallocCheck(a);
    mallocCheck(b);
    for (size_t i = 0; i < getLen(A); ++i) {
        uint64_t block = A->bigIntArray[i];
        size_t base = i * 4;
        a[base + 0] = (uint16_t) (block & 0xFFFFu);
        a[base + 1] = (uint16_t) ((block >> 16) & 0xFFFFu);
        a[base + 2] = (uint16_t) ((block >> 32) & 0xFFFFu);
        a[base + 3] = (uint16_t) ((block >> 48) & 0xFFFFu);
    }
    for (size_t i = 0; i < getLen(B); ++i) {
        uint64_t block = B->bigIntArray[i];
        size_t base = i * 4;
        b[base + 0] = (uint16_t) (block & 0xFFFFu);
        b[base + 1] = (uint16_t) ((block >> 16) & 0xFFFFu);
        b[base + 2] = (uint16_t) ((block >> 32) & 0xFFFFu);
        b[base + 3] = (uint16_t) ((block >> 48) & 0xFFFFu);
    }

    // 3) spread
    uint64_t *a_freq = calloc(len_radix_max, sizeof *a_freq);
    uint64_t *b_freq = calloc(len_radix_max, sizeof *b_freq);
    mallocCheck(a_freq);
    mallocCheck(b_freq);
    spread_twice(a, b, a_freq, b_freq, len_radix, len_radix_max);
    free(a);
    free(b);

    // 4) forward NTT
    ntt_twice(a_freq, b_freq, root_of_unity, len_log_radix);

    // 5) pointwise
    uint64_t *c_freq = calloc(len_radix_max, sizeof *c_freq);
    for (size_t i = 0, ri = 0; i < len_radix_max; ++i, ri = bit_reversed_increment(ri, len_radix_max)) {
        c_freq[ri] = mul_mod(a_freq[i], b_freq[i]);
    }
    free(a_freq);
    free(b_freq);

    // 6) inverse NTT
    ntt_once(c_freq, conj_of_unity, len_log_radix);

    // 7) fold back
    uint16_t *C = calloc(len_radix_max + 4, sizeof *C); // Padding 4 so the window can go out of bounds
    mallocCheck(C);

    fold(c_freq, C, digit_len, len_log_radix);
    free(c_freq);

    // 8) convert back to bigInt
    bigInt *res = newBigInt(getLen(A) + getLen(B));

    uint16_t *res16 = (uint16_t *) res->bigIntArray;
    for (size_t i = 0; i < digit_len; i++) {
        res16[i] = C[i];
    }
    free(C);

    size_t blocks = getOccupiedBlocks(res); // resizing
    res->end = res->start + blocks;

    return res;
}
