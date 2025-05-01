#include <stdio.h>
#include <string.h>

#include "../bigNum/bigInt/bigInt.h"
#include "../bigNum/bigInt/bigIntMethods.h"
#include "../util.h"

void verifyBigInt(bigInt *x) {
    if (!isValidBigInt(x)) {
        fprintf(stderr, "Result is not a valid bigInt\n");
        abort();
    }
}

bigInt *readBigIntHexFromFileTestHelper(char *path) {
    char *delim = ":";
    char *filePath = strtok(path, delim);
    char *partialBigIntUnder = strtok(NULL, delim);
    char *partialBigIntOver = strtok(NULL, delim);
    char *overflow = strtok(NULL, delim);

    bigInt *fileBigInt = readBigIntHexFromFile(filePath);

    if (partialBigIntUnder == NULL) {
        return fileBigInt;
    }
    if (partialBigIntOver == NULL) {
        fprintf(stderr, "To few arguments for partialBigInt file name\n");
        exit(EXIT_FAILURE);
    }
    if (overflow != NULL) {
        fprintf(stderr, "To many arguments for partialBigInt file name\n");
        exit(EXIT_FAILURE);
    }

    size_t under = parseUINT64(partialBigIntUnder, UINT64_MAX, 0);
    size_t over = parseUINT64(partialBigIntOver, UINT64_MAX, 0);

    size_t newSize = under + getLen(fileBigInt) + over;
    bigInt *res = newBigInt(newSize);
    memset(res->bigIntArray, 0xF, under * 8);
    memcpy(res->bigIntArray + under, fileBigInt->bigIntArray, getLen(fileBigInt) * 8);
    memset(res->bigIntArray + under + getLen(fileBigInt), 0xF, over * 8);
    res->start = res->start + under;
    res->end = res->end - over;
    res->negative = fileBigInt->negative;

    freeBigInt(fileBigInt);
    return res;
}

void test2InputInt1Output(bigInt *(*operation)(const bigInt *, size_t), char *inputFile1, char *inputFile2, char *outputFile) {
    char *input1 = readFile(inputFile1);
    size_t toShift = parseUINT64(input1, UINT64_MAX, 0);
    free(input1);

    bigInt *input2 = readBigIntHexFromFileTestHelper(inputFile2);
    verifyBigInt(input2);

    bigInt *res = operation(input2, toShift);
    verifyBigInt(res);

    writeBigIntHexToFile(res, outputFile);

    freeBigInt(input2);
    freeBigInt(res);
}

void test2Input1Output(bigInt *(*operation)(const bigInt *, const bigInt *), char *inputFile1, char *inputFile2, char *outputFile) {
    bigInt *input1 = readBigIntHexFromFileTestHelper(inputFile1);
    bigInt *input2 = readBigIntHexFromFileTestHelper(inputFile2);
    verifyBigInt(input1);
    verifyBigInt(input2);

    bigInt *res = operation(input1, input2);
    verifyBigInt(res);

    writeBigIntHexToFile(res, outputFile);

    freeBigInt(input1);
    freeBigInt(input2);
    freeBigInt(res);
}

void test2Input2Output(bigInt *(*operation)(const bigInt *, const bigInt *, bigInt **), char *inputFile1, char *inputFile2,
                       char *outputFile1, char *outputFile2) {
    bigInt *input1 = readBigIntHexFromFileTestHelper(inputFile1);
    bigInt *input2 = readBigIntHexFromFileTestHelper(inputFile2);
    verifyBigInt(input1);
    verifyBigInt(input2);

    bigInt *res2;
    bigInt *res1 = operation(input1, input2, &res2);
    verifyBigInt(res1);
    verifyBigInt(res2);

    writeBigIntHexToFile(res1, outputFile1);
    writeBigIntHexToFile(res2, outputFile2);

    freeBigInt(input1);
    freeBigInt(input2);
    freeBigInt(res1);
    freeBigInt(res2);
}

void testMul(char *inputFile1, char *inputFile2, char *outputFile) {
    test2Input1Output(mul, inputFile1, inputFile2, outputFile);
}

void testDiv(char *inputFile1, char *inputFile2, char *outputFile) {
    test2Input1Output(divide, inputFile1, inputFile2, outputFile);
}

void testDivMod(char *inputFile1, char *inputFile2, char *outputFile1, char *outputFile2) {
    test2Input2Output(divideMod, inputFile1, inputFile2, outputFile1, outputFile2);
}

void testAdd(char *inputFile1, char *inputFile2, char *outputFile) {
    test2Input1Output(add, inputFile1, inputFile2, outputFile);
}

void testSub(char *inputFile1, char *inputFile2, char *outputFile) {
    test2Input1Output(sub, inputFile1, inputFile2, outputFile);
}

void testShiftLeft(char *inputFile1, char *inputFile2, char *outputFile) {
    test2InputInt1Output(shiftLeft, inputFile1, inputFile2, outputFile);
}

void testShiftRight(char *inputFile1, char *inputFile2, char *outputFile) {
    test2InputInt1Output(shiftLeft, inputFile1, inputFile2, outputFile);
}

