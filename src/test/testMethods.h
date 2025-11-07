#ifndef FIBONACCI_TESTMETHODS_H
#define FIBONACCI_TESTMETHODS_H

void testMul(char *inputFile1, char *inputFile2, const char *outputFile);

void testDiv(char *inputFile1, char *inputFile2, const char *outputFile);

void testDivMod(char *inputFile1, char *inputFile2, const char *outputFile1, const char *outputFile2);

void testAdd(char *inputFile1, char *inputFile2, const char *outputFile);

void testSub(char *inputFile1, char *inputFile2, const char *outputFile);

void testShiftLeft(char *inputFile1, char *inputFile2, const char *outputFile);

void testShiftRight(char *inputFile1, char *inputFile2, const char *outputFile);

void testSHiftAdd(char *inputFile1, char *inputFile2, char *toShiftInt, const char *outputFile);

void testWriteBigIntHexToFile(char *inputFile1, char *outputFile);

void testWriteBigIntDecToFile(char *inputFile1, char *outputFile);

#endif //FIBONACCI_TESTMETHODS_H
