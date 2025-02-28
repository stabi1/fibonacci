#include "string.h"

#include "bigDecString.h"
#include "bigDec.h"
#include <stdio.h>

const char decLookup2[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

char *bigDecToDecString(__attribute__((unused)) bigDec *x) {
    return NULL;
}

char *uint64_t_FractionToDecString(__attribute__((unused)) uint64_t x) {
    return NULL;
    /*size_t bufLen = 21;
    char buf[bufLen];
    size_t charPos = bufLen - 1;

    // Scale factor (2^64)
    uint64_t scale = 1ULL << 63; // 2^63
    printf("Scale: %lx\n", scale);

    // Extract decimal digits
    while (true) {
        printf("%lu\n", x);
        x *= 10;                   // Shift left by one decimal place
        uint64_t digit = x / scale; // Extract the next decimal digit
        printf("DigVal: %lu\n", digit);
        printf("Dig: %c\n", decLookup2[x]);
        buf[charPos--] = '0' + (char) digit;
        x -= digit * scale;         // Remove the digit from x
        if (x == 0) {
            break;
        }
    }
    char *res = malloc(bufLen - charPos + 1);
    mallocCheck(res);
    memcpy(res, buf + charPos, bufLen - charPos);
    res[bufLen - charPos] = '\0';
    return res;*/
}

