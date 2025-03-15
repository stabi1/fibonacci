#ifndef FIBONACCI_COMPUTEFUNCTIONS_H
#define FIBONACCI_COMPUTEFUNCTIONS_H

#include "bigNum/bigFrac/bigFrac.h"
#include "bigNum/bigInt/bigInt.h"

enum computeOperation {
    CONVERT_NUMBER = 0,
    GOLDEN_RATIO,
    FIBONACCI,
    SQUARE_ROOT,
    PI,
    UNKNOWN_OPERATION
};

void convertNumber(char inputRadix, const char *inputFilename, char outputRadix, const char *outputFilename);

void printFibonacci(uint64_t n, char radix, char output, const char *filename, bool infoInOutputFile);

void printGoldenRatio(uint64_t digits, char radix, char output, const char *filename, bool infoInOutputFile);

void printSquareRoot(uint64_t n, uint64_t digits, char radix, char output, const char *filename, bool infoInOutputFile);

void printPi(uint64_t digits, char radix, char output, const char *filename, bool infoInOutputFile);

void conversionAndPrintHelper(bigFrac *res, const char *computeName, const char *radixStr, double timeToCalc, uint64_t digits, char radix, char output,
                              const char *filename, bool infoInOutputFile);

#endif //FIBONACCI_COMPUTEFUNCTIONS_H
