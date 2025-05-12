#include "computeMethods.h"

#include "bigNum/misc.h"
#include "util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

void convertNumber(const char inputRadix, const char *inputFilename, const char outputRadix, const char *outputFilename) {
    if (inputRadix == outputRadix) {
        printf("WARNING: Input-radix and output-radix are the same\n");
    }
    char *dateTime = getCurrentDateTime();
    printf("Converting number from %c to %c; Time: %s\n", inputRadix, outputRadix, dateTime);
    free(dateTime);

    bigInt *tmp;
    struct timespec start, end;
    getCurrentTime(&start);
    if (inputRadix == 'd') {
        tmp = readBigIntDecFromFile(inputFilename);
    } else {
        tmp = readBigIntHexFromFile(inputFilename);
    }
    getCurrentTime(&end);
    double time = calcTimeDiff(&start, &end);
    printf("Reading from file %s and converting to bigInt done, took %.2f seconds\n", inputFilename, time);

    getCurrentTime(&start);
    if (outputRadix == 'd') {
        writeBigIntDecToFileAndFree(tmp, outputFilename);
    } else {
        writeBigIntHexToFile(tmp, outputFilename);
        freeBigInt(tmp);
    }
    getCurrentTime(&end);
    time = calcTimeDiff(&start, &end);
    printf("Finished conversion, result in %s, took %.2f seconds\n", outputFilename, time);
}

void printFibonacci(const uint64_t n, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    char *radixStr = radix == 'd' ? "decimal" : "hexadecimal";
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for the %zu th fibonacci number | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start, end;
    getCurrentTime(&start);

    bigInt *res = fibonacci(n);

    getCurrentTime(&end);
    double timeToCalc = calcTimeDiff(&start, &end);

    size_t sizeInBytes = (res->end - res->start) * 8;

    if (output == 't' || output == 'f') {
        struct timespec start2;
        getCurrentTime(&start2);
        size_t strSizeInBytes;
        char *resString;
        char *localTime = getCurrentDateTime();
        if (global_config.verbose) printf("Starting conversion to %s; %s\n", radixStr, localTime);
        free(localTime);
        if (radix == 'd') {
            resString = bigIntToDecString(res, true);
            strSizeInBytes = strlen(resString);
        } else {
            resString = bigIntToHexString(res, false);
            freeBigInt(res);
            strSizeInBytes = strlen(resString);
        }

        if (output == 'f') {
            // File output
            if (infoInOutputFile) {
                char infoStr[200];
                sprintf(infoStr, "Result for n=%zu in %s | length of string=%zu:\n", n, radixStr, strSizeInBytes);
                if (writeFile(filename, infoStr, false) == -1) exit(EXIT_FAILURE);
                if (writeFile(filename, resString, true) == -1) exit(EXIT_FAILURE);
            } else {
                if (writeFile(filename, resString, false) == -1) exit(EXIT_FAILURE);
            }
            if (global_config.verbose) printf("Result written into file %s\n", filename);
        } else { // Terminal output: output == 't'
            printf("Result: %s\n", resString);
        }

        struct timespec end2;
        getCurrentTime(&end2);
        double timeToOutput = calcTimeDiff(&start2, &end2);

        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;

        double sizeStrInKB = ((double) strSizeInBytes) / 1000;
        double sizeStrInMB = ((double) strSizeInBytes) / 1000000;

        printf("Time to calculate: %f s | Time to output: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n"
               "ResultString size in B:%zu KB:%.2f MB:%.2f\n",
               timeToCalc, timeToOutput,
               sizeInBytes, sizeInKB, sizeInMB, strSizeInBytes, sizeStrInKB, sizeStrInMB);
        free(resString);
    } else {
        freeBigInt(res);
        printf("No output\n");
        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;
        printf("Time to calculate: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n", timeToCalc, sizeInBytes, sizeInKB,
               sizeInMB);
    }
}

void printGoldenRatio(const uint64_t digits, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    size_t binaryDigits;
    char *radixStr;
    if (radix == 'd') {
        binaryDigits = ceil((double) digits * log2(10));
        radixStr = "decimal";
    } else {
        binaryDigits = digits * 4;
        radixStr = "hexadecimal";
    }
    binaryDigits += 64;

    printf("Starting calculation for %zu %s digits of the golden ratio\n", digits, radixStr);

    // calculation
    struct timespec start, end;
    getCurrentTime(&start);

    bigFrac *res = goldenRatio(binaryDigits);

    getCurrentTime(&end);
    double timeToCalc = calcTimeDiff(&start, &end);

    conversionAndPrintHelper(res, "of the golden ratio", radixStr, timeToCalc, digits, radix, output, filename, infoInOutputFile);
    return;
}

void printSquareRoot(const uint64_t n, const uint64_t digits, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    size_t binaryDigits;
    char *radixStr;
    if (radix == 'd') {
        binaryDigits = ceil((double) digits * log2(10));
        radixStr = "decimal";
    } else {
        binaryDigits = digits * 4;
        radixStr = "hexadecimal";
    }
    binaryDigits += 64;
    size_t wantedFractionBlocks = binaryDigits / 64 + 1;

    printf("Starting calculation for %zu %s digits of the square root of %ld\n", digits, radixStr, n);

    // calculation
    struct timespec start, end;
    getCurrentTime(&start);

    bigFrac *radicand = getBigFracFromUnsignedInteger(n);
    bigFrac *res = sqrt2(radicand, wantedFractionBlocks);
    freeBigFrac(radicand);

    getCurrentTime(&end);
    double timeToCalc = calcTimeDiff(&start, &end);

    char computeName[200];
    sprintf(computeName, "of the square root of %ld",n);
    conversionAndPrintHelper(res, computeName, radixStr, timeToCalc, digits, radix, output, filename, infoInOutputFile);
    return;
}

void printPi(const uint64_t digits, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    size_t binaryDigits;
    char *radixStr;
    if (radix == 'd') {
        binaryDigits = ceil((double) digits * log2(10));
        radixStr = "decimal";
    } else {
        binaryDigits = digits * 4;
        radixStr = "hexadecimal";
    }
    binaryDigits += 64;

    printf("Starting calculation for %zu %s digits of pi\n", digits, radixStr);

    // calculation
    struct timespec start, end;
    getCurrentTime(&start);

    bigFrac *res = pi(binaryDigits);

    getCurrentTime(&end);
    double timeToCalc = calcTimeDiff(&start, &end);

    conversionAndPrintHelper(res, "of pi", radixStr, timeToCalc, digits, radix, output, filename, infoInOutputFile);
    return;
}

void print_e(const uint64_t digits, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    size_t binaryDigits;
    char *radixStr;
    if (radix == 'd') {
        binaryDigits = ceil((double) digits * log2(10));
        radixStr = "decimal";
    } else {
        binaryDigits = digits * 4;
        radixStr = "hexadecimal";
    }
    binaryDigits += 64;

    printf("Starting calculation for %zu %s digits of e\n", digits, radixStr);

    // calculation
    struct timespec start, end;
    getCurrentTime(&start);

    bigFrac *res = e(binaryDigits);

    getCurrentTime(&end);
    double timeToCalc = calcTimeDiff(&start, &end);

    conversionAndPrintHelper(res, "of e", radixStr, timeToCalc, digits, radix, output, filename, infoInOutputFile);
    return;
}

void conversionAndPrintHelper(bigFrac *res, const char *computeName, const char *radixStr, double timeToCalc, const uint64_t digits, const char radix, const char output,
                              const char *filename, const bool infoInOutputFile) {
    size_t sizeInBytes = (res->bigIntPart->end - res->bigIntPart->start) * 8;
    if (output == 't' || output == 'f') {
        size_t strSizeInBytes;
        char *resString;
        char *localTime = getCurrentDateTime();
        if (global_config.verbose) printf("Starting conversion to %s; %s\n", radixStr, localTime);
        free(localTime);

        struct timespec start2;
        getCurrentTime(&start2);
        if (radix == 'd') {
            resString = bigFracToDecString(res, false);
            freeBigFrac(res);
            strSizeInBytes = strlen(resString);
        } else {
            resString = bigFracToHexString(res);
            freeBigFrac(res);
            strSizeInBytes = strlen(resString);
        }
        struct timespec end2;
        getCurrentTime(&end2);
        double timeToConvert = calcTimeDiff(&start2, &end2);


        if (strSizeInBytes < digits + 2) {
            fprintf(stderr, "Not enough digits calculated\n");
            exit(EXIT_FAILURE);
        }
        if (global_config.verbose) printf("Digits to much: %zu\n", strSizeInBytes - digits + 2);
        //cut of the too many digits
        if(digits == 0) {
            resString[digits + 2] = '0'; // so that 3. turns unto 3.0
            resString[digits + 3] = '\0';
        } else {
            resString[digits + 2] = '\0';
        }


        struct timespec start3;
        getCurrentTime(&start3);
        if (output == 'f') { // File output
            if (infoInOutputFile) {
                char infoStr[200];
                sprintf(infoStr, "Result for %zu %s digits %s | length of string=%zu:\n", digits, radixStr, computeName, strSizeInBytes);
                if (writeFile(filename, infoStr, false) == -1) exit(EXIT_FAILURE);
                if (writeFile(filename, resString, true) == -1) exit(EXIT_FAILURE);
            } else {
                if (writeFile(filename, resString, false) == -1) exit(EXIT_FAILURE);
            }
            if (global_config.verbose) printf("Result written into file %s\n", filename);
        } else { // Terminal output: output == 't'
            printf("Result: %s\n", resString);
        }

        struct timespec end3;
        getCurrentTime(&end3);
        double timeToOutput = calcTimeDiff(&start3, &end3);

        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;

        double sizeStrInKB = ((double) strSizeInBytes) / 1000;
        double sizeStrInMB = ((double) strSizeInBytes) / 1000000;

        printf("Time to calculate: %f s | Time to convert %f s | Time to output: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n"
               "ResultString size in B:%zu KB:%.2f MB:%.2f\n",
               timeToCalc, timeToConvert, timeToOutput,
               sizeInBytes, sizeInKB, sizeInMB, strSizeInBytes, sizeStrInKB, sizeStrInMB);
        free(resString);
    } else {
        freeBigFrac(res);
        printf("No output\n");
        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;
        printf("Time to calculate: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n", timeToCalc, sizeInBytes, sizeInKB,
               sizeInMB);
    }
}
