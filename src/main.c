#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <stdbool.h>

#include "main.h"
#include "../test/tests.h"
#include "util.h"

size_t getDepth(size_t nprocsSet);

void printHelpMenu();

void printFibonacci(uint64_t n, char radix, char output, bool multiThread, size_t numberOfCoresSet);

bool handleCPUFeatures();

void printMissingFeature(char *feature);

bool verbose = false;

const char *filename = "output.txt";

//TODO add sign support to DIV,
//TODO chech sign support -> improve SmartAdd/SmartSub
//TODO check support for variable starting point of bigIntArray
//TODO mul wrapper
//TODO numberOfCores set not good, maybe set depth directly
//TODO make output filename customizable

static struct option long_options[] = {
        {"help",          no_argument,       NULL, 'h'},
        {"output",        required_argument, NULL, 'o'},
        {"multiThread",   no_argument,       NULL, 'm'},
        {"numCores",      required_argument, NULL, 'c'},
        {"radix",         required_argument, NULL, 'r'},
        {"debug",         no_argument,       NULL, 'd'},
        {"benchMark",     no_argument,       NULL, 'b'},
        {"test",          no_argument,       NULL, 't'},
        {"nth-fibonacci", required_argument, NULL, 'n'},
        {"verbose",       no_argument,       NULL, 'v'},
        {NULL, 0,                            NULL, 0}
};

int main(int argc, char *argv[]) {
    bool cpuFeatures = handleCPUFeatures();
    if (!cpuFeatures) {
        exit(EXIT_FAILURE);
    }

    if (argc == 1) {
        fprintf(stderr, "No arguments, use -h for usage\n");
        exit(EXIT_FAILURE);
    }

    // default values
    char radix = 'h';
    char output = 't';
    bool multiThread = false;
    size_t cores = 0; //available cores
    verbose = false;
    uint64_t n = 0;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbtmvdr:o:c:n:", long_options, NULL)) != -1) {
            switch (option) {
                case 'h':
                    printHelpMenu();
                    exit(EXIT_SUCCESS);
                case 'd':
                    bruteForceDebug(multiThread);
                    exit(EXIT_SUCCESS);
                case 't':
                    test();
                    exit(EXIT_SUCCESS);
                case 'b':
                    benchMark();
                    exit(EXIT_SUCCESS);
                case 'm':
                    multiThread = true;
                    break;
                case 'v':
                    verbose = true;
                    break;
                case 'c':
                    cores = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 1);
                    break;
                case 'r':
                    if ((optarg[0] != 'h' && optarg[0] != 'd') || optarg[1] != '\0') {
                        fprintf(stderr, "invalid radix option \"%s\" provided, use -h for usage\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    radix = optarg[0];
                    break;
                case 'o':
                    if ((optarg[0] != 'f' && optarg[0] != 't' && optarg[0] != 'n') || optarg[1] != '\0') {
                        fprintf(stderr, "invalid output option \"%s\" provided, use -h for usage\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    output = optarg[0];
                    if (output != 'f' && output != 't' && output != 'n') {
                        output = 'n';
                        fprintf(stderr, "Invalid output option %c!\nno output...\n", output);
                    }
                    break;
                case 'n':
                    n = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 0);
                    break;
                default:
                    fprintf(stderr, "Invalid input formatting for fibonacci, use -h for usage\n");
                    exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "No non option argument expected,  use -h for usage");
            optind++;
            exit(EXIT_FAILURE);
        }
    }

    printFibonacci(n, radix, output, multiThread, cores);

    return EXIT_SUCCESS;
}

void printFibonacci(uint64_t n, char radix, char output, bool multiThread, size_t numberOfCoresSet) {
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for n=%zu | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (multiThread) printf("Multithreading enabled\n");
    else printf("Single Thread\n");
    bigInt *res = fibExpFastDoubling(n, multiThread, numberOfCoresSet);

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
            if (verbose) printf("Starting conversion to dec\n");
            resString = bigIntToDecString(res);
            strSizeInBytes = strlen(resString);
        } else {
            if (verbose) printf("Starting conversion to hex\n");
            resString = bigIntToHexString(res);
            strSizeInBytes = strlen(resString);
        }

        if (output == 'f') { // File output
            char infoStr[1000];
            sprintf(infoStr, "Result for n=%zu | length of string=%zu:\n", n, strSizeInBytes);
            write_file(filename, infoStr, false);
            write_file(filename, resString, true);
        } else { //Terminal output (output == 't')
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
        printf("Time to calculate: %fs\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n", time, sizeInBytes, sizeInKB,
               sizeInMB);
    }
    freeBigInt(res);
}

bigInt *fibExpFastDoubling(uint64_t n, bool multiThread, size_t numberOfCoresSet) {
    size_t multiThreadDepth = 0;
    if (multiThread) multiThreadDepth = getDepth(numberOfCoresSet);
    bigInt *a = newBigInt(1);
    bigInt *b = newBigInt(1);
    b->bigIntArray[0] = 1;
    unsigned int shift = 64 - custom_lzcnt(n) - 1;
    uint64_t nBinary = ((n >> shift) << shift);

    //for verbose
    unsigned long iterations = 64 - custom_lzcnt(nBinary);
    int counter = 1;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (verbose) printf("\n");

    for (; nBinary != 0; nBinary >>= 1) {
        if (verbose) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
            size_t sizeInBytes = (a->end - a->start) * 8;
            double sizeInMB = ((double) sizeInBytes) / 1000000;
            printf("Iteration ongoing %d/%lu; Current size: %f MB; Time needed for previous iteration: %f s\n", counter,
                   iterations, sizeInMB, time);
            counter++;
            clock_gettime(CLOCK_MONOTONIC, &start);
        }
        bigInt *temp1 = shiftLeft(b, 1);
        bigInt *temp2 = smartSub(temp1, a);
        freeBigInt(temp1);

        bigInt *d;
        bigInt *temp3;
        bigInt *temp4;
        if (multiThread) {
            d = multiplyToomCook3MultiThread(a, temp2, multiThreadDepth);
            freeBigInt(temp2);
            temp3 = multiplyToomCook3MultiThread(a, a, multiThreadDepth);
            temp4 = multiplyToomCook3MultiThread(b, b, multiThreadDepth);
        } else {
            d = multiplyToomCook3(a, temp2);
            freeBigInt(temp2);
            temp3 = multiplyToomCook3(a, a);
            temp4 = multiplyToomCook3(b, b);
        }

        freeBigInt(a);
        freeBigInt(b);
        bigInt *e = smartAdd(temp3, temp4);
        freeBigInt(temp3);
        freeBigInt(temp4);
        a = d;
        b = e;

        // Advance by one conditionally
        if ((n & nBinary) != 0) {
            bigInt *c = smartAdd(a, b);
            freeBigInt(a);
            a = b;
            b = c;
        }
    }
    freeBigInt(b);
    if (verbose) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
        printf("Time needed for last Iteration: %fs\n\n", time);
    }
    return a;
}

size_t getDepth(size_t nprocsSet) {
    size_t numberOfCores;
    if (nprocsSet == 0) {
        numberOfCores = get_nprocs();
        if (verbose) printf("Number of cores detected: %lu\n", numberOfCores);
    } else {
        numberOfCores = nprocsSet;
        if (verbose) printf("Number of cores set: %lu\n", numberOfCores);
    }

    if (numberOfCores < 6) {
        if (verbose) printf("Number of compute threads created: %d\n", 5);
        return 1;
    } else if (numberOfCores < 26) {
        if (verbose) printf("Number of compute threads created: %d\n", 25);
        return 2;
    } else if (numberOfCores < 126) {
        if (verbose) printf("Number of compute threads created: %d\n", 125);
        return 3;
    } else {
        printf("Number of compute threads created: %d\n", 625);
        return 4;
    }
}

void printHelpMenu() {
    char *fileName = "HelpMenu.txt";
    FILE *helpFile = fopen(fileName, "r");
    if (!helpFile) {
        fprintf(stderr, "%s couldn't be opened\n", fileName);
        exit(EXIT_FAILURE);
    }
    if (fseek(helpFile, 0L, SEEK_END) == -1) {
        fprintf(stderr, "%s: error while getting file length\n", fileName);
        fclose(helpFile);
        exit(EXIT_FAILURE);
    }
    long int helpLen = ftell(helpFile);
    if (helpLen == -1) {
        fprintf(stderr, "%s: error while getting file length\n", fileName);
        fclose(helpFile);
        exit(EXIT_FAILURE);
    }
    if (fseek(helpFile, 0L, SEEK_SET) == -1) {
        fprintf(stderr, "%s: error while getting file length\n", fileName);
        fclose(helpFile);
        exit(EXIT_FAILURE);
    }
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
    } else if (!__builtin_cpu_supports("sse4.1")) {
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
    fprintf(stderr, "Missing Feature: %s; program terminated\n", feature);
}
