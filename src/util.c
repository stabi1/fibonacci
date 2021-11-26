#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <stdbool.h>
#include <ctype.h>
#include "util.h"

size_t roundUp(size_t dividend, size_t divisor);

const char hexLookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

char* uint64tToHexString(uint64_t* array, size_t lenInBytes, size_t start){
    char *str = malloc(sizeof(char) * (lenInBytes * 2 + 1));
    str[lenInBytes * 2] = '\0';
    size_t j = lenInBytes * 2 - 1;
    uint8_t *buf = (uint8_t *)array;
    for (size_t i = start*8; i < start*8 + lenInBytes; i++) {
        str[j] = hexLookup[buf[i] & 0xF];
        str[j - 1] = hexLookup[buf[i] >> 4];
        j -= 2;
    }

    char *trimmed = trimZeroes(str);
    free(str);
    return trimmed;
}

bool checkIsNumber(char* value){
    char *x;
    for (x = value ; *x ; x++) {
        if (!isdigit(*x))
            return false;
    }
    return true;
}

char* uint64tToDecString(__attribute__((unused))uint64_t* array, size_t lenInBytes) {
    char *str = malloc(sizeof(char) * (lenInBytes * 2 + 1));
    str[lenInBytes * 2] = '\0';
    //TODO
    return str;
}

//trims the leading '0's from str
char *trimZeroes(char *str) {
    //Trim leading space
    int counter = (int) strlen(str);
    while (*str == '0' && counter > 1) {
        str++;
        counter--;
    }
    int i = 0;
    char *newStr = malloc(counter + 1);
    while (counter > 0) {
        *(newStr + i) = *str;
        counter--;
        str++;
        i++;
    }
    newStr[i] = '\0';
    return newStr;
}

char *extendHexString(char *hex) {
    size_t oldLen = strlen(hex);
    if (oldLen == 0) {
        free(hex);
        char *zero = malloc(2);
        zero[0] = '0';
        zero[1] = '\0';
        return extendHexString(zero);
    }
    if (oldLen % 16 == 0) {
        return hex;
    }
    size_t newLen = roundUp(oldLen, 16) * 16;
    //determine amount of zeroes to fill up with
    size_t paddingSize = newLen - oldLen;
    char *paddedHex = malloc(sizeof(char) * (newLen + 1));
    //copies input hex to end of new hex with null char
    for (size_t i = 0; i < oldLen + 1; ++i) {
        *(paddedHex + paddingSize + i) = hex[i];
    }
    for (size_t i = 0; i < paddingSize; ++i) {
        paddedHex[i] = '0';
    }
    //free(hex);
    return paddedHex;
}

size_t custom_lzcnt(uint64_t n) {
    if (n == 0) {
        return 0;
    }
    return __builtin_clzll(n);
}

//Helper for hexStringToBigInt
size_t hexToNibble(char hex) {
    switch (hex) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'A':
            return 10;
        case 'B':
            return 11;
        case 'C':
            return 12;
        case 'D':
            return 13;
        case 'E':
            return 14;
        case 'F':
            return 15;
        default:
            printf("HexString Invalid\n");
            return 0;
    }
}

size_t roundUp(size_t dividend, size_t divisor) {
    size_t retVal = dividend / divisor;
    if (divisor * retVal < dividend) {
        retVal++;
    }
    return retVal;
}