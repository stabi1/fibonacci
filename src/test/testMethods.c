#include <stdio.h>

#include "../bigNum/bigInt/bigInt.h"
#include "../bigNum/bigInt/bigIntMethods.h"
#include "../util.h"

void verifyBigInt(bigInt *x) {
    if (!isValidBigInt(x)) {
        fprintf(stderr, "Result is not a valid bigInt\n");
        abort();
    }
}

void
test2InputInt1Output(bigInt *(*operation)(const bigInt *, size_t), char *inputFile1, char *inputFile2, char *outputFile) {
    char *input1 = readFile(inputFile1);
    size_t toShift = parseUINT64(input1, UINT64_MAX, 0);
    free(input1);

    bigInt *input2 = readBigIntHexFromFile(inputFile2);
    verifyBigInt(input2);

    bigInt *res = operation(input2, toShift);
    verifyBigInt(res);

    writeBigIntHexToFile(res, outputFile);

    freeBigInt(input2);
    freeBigInt(res);
}

void test2Input1Output(bigInt *(*operation)(const bigInt *, const bigInt *), char *inputFile1, char *inputFile2, char *outputFile) {
    bigInt *input1 = readBigIntHexFromFile(inputFile1);
    bigInt *input2 = readBigIntHexFromFile(inputFile2);
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
    bigInt *input1 = readBigIntHexFromFile(inputFile1);
    bigInt *input2 = readBigIntHexFromFile(inputFile2);
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

