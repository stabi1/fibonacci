#include "main.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include "BigIntAsm.h"
#include "BigInt.h"
#include "../test/tests.h"
#include "util.h"
#include "mul.h"

size_t getDepth();

void printHelpMenu();

void printFibonacci(uint64_t n, char radix, char output, bool multiThread);

bool handleCPUFeatures();

void printMissingFeature(char *feature);

jmp_buf exceptionJump;

static struct option long_options[] = {
        {"help",        no_argument,       NULL, 'h'},
        {"output",      required_argument, NULL, 'o'},
        {"multiThread", no_argument,       NULL, 'm'},
        {"radix",       required_argument, NULL, 'r'},
        {"debug",       no_argument,       NULL, 'd'},
        {"benchMark",   no_argument,       NULL, 'b'},
        {"test",        no_argument,       NULL, 't'},
        {"fibonacci",   required_argument, NULL, 'f'},
        {NULL, 0,                          NULL, 0}
};

enum {
    NUMBER_N,
};

const char *fibonacciKeys[] = {
        [NUMBER_N] = "n",
};

int main(int argc, char *argv[]) {
    bool cpuFeatures = handleCPUFeatures();
    if (!cpuFeatures) {
        return EXIT_FAILURE;
    }
    //Error handling
    if (setjmp(exceptionJump)) { //Exception e.g. malloc returned null
        printf("An error occurred, program terminated\n");
        return EXIT_FAILURE;
    }

    if(argc == 1) {
        printHelpMenu();
        return EXIT_FAILURE;
    }

    char radix = 'h';
    char output = 't';
    uint64_t n = 0;

    int option;
    bool multiThread = false;
    char *subOpts, *value;
    value = NULL;
    while ((option = getopt_long(argc, argv, "hbtmdr:o:f:", long_options, NULL)) != -1) {
        subOpts = optarg;
        switch (option) {
            case 'h':
                printHelpMenu();
                return EXIT_SUCCESS;
            case 'd':
                bruteForceDebug(multiThread);
                return EXIT_SUCCESS;
            case 't':
                test();
                return EXIT_SUCCESS;
            case 'b':
                benchMark();
                return EXIT_SUCCESS;
            case 'm' :
                multiThread = true;
                break;
            case 'r' :
                if (optarg == NULL || (optarg[0] != 'h' && optarg[0] != 'd')) {
                    printf("no radix option provided!\n");
                    printHelpMenu();
                    return EXIT_FAILURE;
                }
                radix = optarg[0];
                if (radix != 'd' && radix != 'h') {
                    radix = 'h';
                    printf("Invalid output option %c!\nUsing hex instead...\n", radix);
                }
                break;
            case 'o':
                if (optarg == NULL || (optarg[0] != 'f' && optarg[0] != 't' && optarg[0] != 'n')) {
                    printf("no output option provided!\n");
                    printHelpMenu();
                    return EXIT_FAILURE;
                }
                output = optarg[0];
                if (output != 'f' && output != 't' && output != 'n') {
                    output = 'n';
                    printf("Invalid output option %c!\nno output...\n", output);
                }
                break;
            case 'f':
                while (*subOpts != '\0') {
                    switch (getsubopt(&subOpts, (char **) fibonacciKeys, &value)) {
                        case NUMBER_N:
                            if (value == NULL) {
                                printf("No explicit number provided!\n");
                                printHelpMenu();
                                return EXIT_FAILURE;
                            }
                            if (!checkIsNumber(value)) {
                                printf("%s is not a valid number!\n", value);
                                return EXIT_FAILURE;
                            }
                            n = strtol(value, NULL, 10);
                            break;
                        default:
                            printf("Invalid input formatting for fibonacci!\n");
                            printHelpMenu();
                            return EXIT_FAILURE;
                    }
                }
                printFibonacci(n, radix, output, multiThread);
                break;
            default:
                printHelpMenu();
                break;
        }
    }
    return EXIT_SUCCESS;
}

void printFibonacci(uint64_t n, char radix, char output, bool multiThread) {
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for n=%zu | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bigInt *res;
    if (multiThread) {
        printf("Multithreading enabled\n");
        res = fibExpFastDoublingMultiThread(n);
    } else {
        printf("Single Thread\n");
        res = fibExpFastDoubling(n);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

    size_t sizeInBytes = (res->end - res->start) * 8;

    if (output == 't' || output == 'f') {
        struct timespec start2;
        clock_gettime(CLOCK_MONOTONIC, &start2);
        size_t strSizeInBytes;
        char *resString;
        if (radix == 'd') {
            printf("Starting conversion to dec\n");
            resString = bigIntToDecString(res);
            strSizeInBytes = strlen(resString);
        } else {
            printf("Starting conversion to hex\n");
            resString = bigIntToHexString(res);
            strSizeInBytes = sizeInBytes * 2;
        }

        if (output == 'f') { //Terminal output
            char *filename = "output.txt";
            FILE *outputFile = fopen(filename, "a+");
            if (!outputFile) {
                printf("Error while opening/creating %s\n", filename);
            } else {
                outputFile = freopen("output.txt", "w", outputFile);
                if (!outputFile) {
                    printf("Error while opening/creating %s\n", filename);
                } else {
                    fprintf(outputFile, "Result for n=%zu | length of string=%zu:\n%s", n, strSizeInBytes, resString);
                    fclose(outputFile);
                    printf("Output in file %s\n", filename);
                }
            }

        } else { //File output (output == 't')
            printf("Result: %s\n", resString);
        }
        struct timespec end2;
        clock_gettime(CLOCK_MONOTONIC, &end2);
        double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);

        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;

        double sizeStrInKB = ((double) strSizeInBytes) / 1000;
        double sizeStrInMB = ((double) strSizeInBytes) / 1000000;

        printf("Time to calculate: %fs | Time to output: %fs\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n"
               "ResultString size in B:%zu KB:%.2f MB:%.2f\n",
               time, time2,
               sizeInBytes, sizeInKB, sizeInMB, strSizeInBytes, sizeStrInKB, sizeStrInMB);
        free(resString);
    } else {
        printf("No output\n");
        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;
        printf("Time to calculate: %fs\nResultNUmber size in B:%zu KB:%.2f MB:%.2f\n", time, sizeInBytes, sizeInKB,
               sizeInMB);
    }
    freeBigInt(res);
}

bigInt *fibExpFastDoubling(uint64_t n) {
    bigInt *a = newBigInt(1);
    bigInt *b = newBigInt(1);
    b->bigIntArray[0] = 1;
    unsigned int shift = 64 - custom_lzcnt(n) - 1;
    uint64_t nBinary = ((n >> shift) << shift);
    for (; nBinary != 0; nBinary >>= 1) {
        bigInt *temp1 = shiftLeft_Asm(b, 1);
        bigInt *temp2 = sub_Asm(temp1, a, false);
        freeBigInt(temp1);
        bigInt *d = multiplyToomCook3(a, temp2);
        freeBigInt(temp2);
        bigInt *temp3 = multiplyToomCook3(a, a);
        bigInt *temp4 = multiplyToomCook3(b, b);
        freeBigInt(a);
        freeBigInt(b);
        bigInt *e = add_Asm(temp3, temp4, false);
        freeBigInt(temp3);
        freeBigInt(temp4);
        a = d;
        b = e;

        // Advance by one conditionally
        if ((n & nBinary) != 0) {
            bigInt *c = add_Asm(a, b, false);
            freeBigInt(a);
            a = b;
            b = c;
        }
    }
    freeBigInt(b);
    return a;
}

bigInt *fibExpFastDoublingMultiThread(uint64_t n) {
    size_t depth = getDepth();
    bigInt *a = newBigInt(1);
    bigInt *b = newBigInt(1);
    b->bigIntArray[0] = 1;
    unsigned int shift = 64 - custom_lzcnt(n) - 1;
    uint64_t nBinary = ((n >> shift) << shift);
    for (; nBinary != 0; nBinary >>= 1) {
        bigInt *temp1 = shiftLeft_Asm(b, 1);
        bigInt *temp2 = sub_Asm(temp1, a, false);
        freeBigInt(temp1);
        bigInt *d = multiplyToomCook3MultiThread(a, temp2, depth);
        freeBigInt(temp2);
        bigInt *temp3 = multiplyToomCook3MultiThread(a, a, depth);
        bigInt *temp4 = multiplyToomCook3MultiThread(b, b, depth);
        freeBigInt(a);
        freeBigInt(b);
        bigInt *e = add_Asm(temp3, temp4, false);
        freeBigInt(temp3);
        freeBigInt(temp4);
        a = d;
        b = e;

        // Advance by one conditionally
        if ((n & nBinary) != 0) {
            bigInt *c = add_Asm(a, b, false);
            freeBigInt(a);
            a = b;
            b = c;
        }
    }
    freeBigInt(b);
    return a;
}

size_t getDepth() {
    size_t numberOfCores = get_nprocs();
    printf("Number of Cores: %lu\n", numberOfCores);
    if (numberOfCores < 3) {
        return 0;
    } else if (numberOfCores < 10) {
        return 1;
    } else if (numberOfCores < 28) {
        return 2;
    } else if (numberOfCores < 82) {
        return 3;
    } else {
        return 4;
    }
}

void printHelpMenu() {
    char *fileName = "HelpMenu.txt";
    FILE *helpFile = fopen(fileName, "r");
    if (!helpFile) {
        printf("%s couldn't be read\n", fileName);
        return;
    }
    fseek(helpFile, 0L, SEEK_END);
    long int helpLen = ftell(helpFile);
    fseek(helpFile, 0L, SEEK_SET);
    char menu[helpLen];
    while (fgets(menu, (int) helpLen, helpFile)) {
        printf("%s", menu);
    }
    fclose(helpFile);
}

bool handleCPUFeatures() {
    __builtin_cpu_init();
    if (!__builtin_cpu_supports("sse")) {
        printMissingFeature("sse");
        return false;
    } else if (!__builtin_cpu_supports("sse2")) {
        printMissingFeature("sse2");
        return false;
    } else if (!__builtin_cpu_supports("sse3")) {
        printMissingFeature("sse3");
        return false;
    }else if (!__builtin_cpu_supports("sse4.1")) {
        printMissingFeature("sse4.1");
        return false;
    } else if (!__builtin_cpu_supports("sse4.2")) {
        printMissingFeature("sse4.2");
        return false;
    } else if (!__builtin_cpu_supports("avx")) {
        printMissingFeature("avx");
        return false;
    } else if (!__builtin_cpu_supports("avx2")) {
        printMissingFeature("avx2");
        return false;
    }
    return true;
}

void printMissingFeature(char *feature) {
    printf("Missing Feature: %s; program terminated\n", feature);
}