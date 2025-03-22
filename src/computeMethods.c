#include "computeMethods.h"

#include "bigNum/misc.h"

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
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    if (inputRadix == 'd') {
        tmp = readBigIntDecFromFile(inputFilename);
    } else {
        tmp = readBigIntHexFromFile(inputFilename);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Reading from file %s and converting to bigInt done, took %.2f seconds\n", inputFilename, time);

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    if (outputRadix == 'd') {
        writeBigIntDecToFile(tmp, outputFilename, true);
    } else {
        writeBigIntHexToFile(tmp, outputFilename);
        freeBigInt(tmp);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Finished conversion, result in %s, took %.2f seconds\n", outputFilename, time);
}

void printFibonacci(const uint64_t n, const char radix, const char output, const char *filename, const bool infoInOutputFile) {
    char *radixStr = radix == 'd' ? "decimal" : "hexadecimal";
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for the %zu th fibonacci number | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigInt *res = fibonacci(n);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double timeToCalc = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

    size_t sizeInBytes = (res->end - res->start) * 8;

    if (output == 't' || output == 'f') {
        struct timespec start2;
        if (clock_gettime(CLOCK_MONOTONIC, &start2) == -1) perror("Error measuring time!");
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
        if (clock_gettime(CLOCK_MONOTONIC, &end2) == -1) perror("Error measuring time!");
        double timeToOutput = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);

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
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigFrac *res = goldenRatio(binaryDigits);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double timeToCalc = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

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
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigFrac *radicand = getBigFracFromUnsignedInteger(n);
    bigFrac *res = sqrt2(radicand, wantedFractionBlocks);
    freeBigFrac(radicand);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double timeToCalc = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

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
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigFrac *res = pi(binaryDigits);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double timeToCalc = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

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
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigFrac *res = e(binaryDigits);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double timeToCalc = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

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
        if (clock_gettime(CLOCK_MONOTONIC, &start2) == -1) perror("Error measuring time!");
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
        if (clock_gettime(CLOCK_MONOTONIC, &end2) == -1) perror("Error measuring time!");
        double timeToConvert = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);


        if (strSizeInBytes < digits + 2) {
            fprintf(stderr, "Not enough digits calculated\n");
            exit(EXIT_FAILURE);
        }
        if (global_config.verbose) printf("Digits to much: %zu\n", strSizeInBytes - digits + 2);
        resString[digits + 2] = '\0'; //cut of the too many digits

        struct timespec start3;
        if (clock_gettime(CLOCK_MONOTONIC, &start3) == -1) perror("Error measuring time!");
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
        if (clock_gettime(CLOCK_MONOTONIC, &end3) == -1) perror("Error measuring time!");
        double timeToOutput = (double) end3.tv_sec - (double) start3.tv_sec + 1e-9 * (double) (end3.tv_nsec - start3.tv_nsec);

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
