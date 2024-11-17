#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "main.h"
#include "../test/tests.h"
#include "util.h"
#include "bigInt/config.h"

void printHelpMenu();

void printFibonacci(uint64_t n, char radix, char output);

bool handleCPUFeatures();

void printMissingFeature(char *feature);

const char *DEFAULT_FILENAME = "output.txt";

//TODO Optimize for memory, maybe add swap capabilities
//TODO make div and to_dec_string parallel
//TODO check support for variable starting point of bigIntArray
//TODO make output filename customizable
//TODO help message in binary
//TODO change makefile compilation so every file is compiled individually
//TODO docu with comments, readme

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
        {"maxThreads",    required_argument, NULL, 'p'},
        {NULL, 0,                            NULL, 0}
};

Config global_config = {
        .verbose = false,
        .parallel = false,
        .mulDepth = 0
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
    size_t cores = 0; //available cores
    size_t max_threads = 0;
    uint64_t n = 0;
    bool do_debug = false;
    bool do_test = false;
    bool do_benchmark = false;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbtmvdr:o:c:n:p:", long_options, NULL)) != -1) {
            switch (option) {
                case 'h':
                    printHelpMenu();
                    exit(EXIT_SUCCESS);
                case 'd':
                    do_debug = true;
                    break;
                case 't':
                    do_test = true;
                    break;
                case 'b':
                    do_benchmark = true;
                    break;
                case 'm':
                    global_config.parallel = true;
                    break;
                case 'v':
                    global_config.verbose = true;
                    break;
                case 'c':
                    cores = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 1);
                    break;
                case 'p':
                    max_threads = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 1);
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

    if (global_config.parallel) {
        printf("Multithreading enabled\n");
        if (cores != 0 && max_threads != 0) {
            fprintf(stderr, "Options p and n are mutually exclusive");
            exit(EXIT_FAILURE);
        }
        if (cores != 0)
            global_config.mulDepth = getMulDepthFromCores(cores);
        else
            global_config.mulDepth = getMulDepthFromMaxThreads(max_threads);
    }

    if (do_test) {
        test();
        return EXIT_SUCCESS;
    } else if (do_debug) {
        bruteForceDebug();
        return EXIT_SUCCESS;
    } else if (do_benchmark) {
        benchMark();
        return EXIT_SUCCESS;
    }

    printFibonacci(n, radix, output);

    return EXIT_SUCCESS;
}

void printFibonacci(uint64_t n, char radix, char output) {
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for n=%zu | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    bigInt *res = fibonacci(n);

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
            if (global_config.verbose) printf("Starting conversion to dec\n");
            resString = bigIntToDecString(res);
            strSizeInBytes = strlen(resString);
        } else {
            if (global_config.verbose) printf("Starting conversion to hex\n");
            resString = bigIntToHexString(res);
            strSizeInBytes = strlen(resString);
        }

        if (output == 'f') { // File output
            char infoStr[1000];
            sprintf(infoStr, "Result for n=%zu | length of string=%zu:\n", n, strSizeInBytes);
            if (writeFile(DEFAULT_FILENAME, infoStr, false) == -1) exit(EXIT_FAILURE);
            if (writeFile(DEFAULT_FILENAME, resString, true) == -1) exit(EXIT_FAILURE);
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

        printf("Time to calculate: %f s | Time to output: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n"
               "ResultString size in B:%zu KB:%.2f MB:%.2f\n",
               time, time2,
               sizeInBytes, sizeInKB, sizeInMB, strSizeInBytes, sizeStrInKB, sizeStrInMB);
        free(resString);
    } else {
        printf("No output\n");
        double sizeInKB = ((double) sizeInBytes) / 1000;
        double sizeInMB = ((double) sizeInBytes) / 1000000;
        printf("Time to calculate: %f s\nResultNumber size in B:%zu KB:%.2f MB:%.2f\n", time, sizeInBytes, sizeInKB,
               sizeInMB);
    }
    freeBigInt(res);
}

void printHelpMenu() {
    char *fileName = "HelpMenu.txt";
    char *helpMenuText = readFile(fileName);
    if (helpMenuText == NULL) {
        fprintf(stderr, "Error printing help message");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", helpMenuText);
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
