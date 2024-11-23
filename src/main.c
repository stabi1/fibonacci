#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "test/tests.h"
#include "bigInt/bigInt.h"
#include "util.h"

void printHelpMenu();

void printFibonacci(uint64_t n, char radix, char output, char *filename);

const char *DEFAULT_FILENAME = "output.txt";

//TODO: zero bigIntArray only when necessary
//TODO cache for bigger bigIntArrays
//TODO fix header files
//TODO Optimize for memory, maybe add swap capabilities
//TODO make div and to_dec_string parallel
//TODO check support for variable starting point of bigIntArray
//TODO fix makefile
//TODO docu with comments, readme, help-message
//TODO Maybe cache for freed bigInts to reduce malloc calls?

enum {
    OPT_MAX_THREADS = 1001,
    OPT_NUM_CORES,
    OPT_RESULT_FILENAME,
    OPT_DO_SWAP,
    OPT_SWAP_THRESHOLD
};

static struct option long_options[] = {
        {"help",            no_argument,       NULL, 'h'},
        {"output",          required_argument, NULL, 'o'},
        {"multithread",     no_argument,       NULL, 'm'},
        {"radix",           required_argument, NULL, 'r'},
        {"debug",           no_argument,       NULL, 'd'},
        {"benchMark",       no_argument,       NULL, 'b'},
        {"test",            no_argument,       NULL, 't'},
        {"nth-fibonacci",   required_argument, NULL, 'n'},
        {"verbose",         no_argument,       NULL, 'v'},
        {"max-threads",     required_argument, NULL, OPT_MAX_THREADS},
        {"num-cores",       required_argument, NULL, OPT_NUM_CORES},
        {"result-filename", required_argument, NULL, OPT_RESULT_FILENAME},
        {"do-swap",         no_argument,       NULL, OPT_DO_SWAP},
        {"swap-threshold",  required_argument, NULL, OPT_SWAP_THRESHOLD},
        {NULL, 0,                              NULL, 0}
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No arguments, use -h for usage\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handleSignals;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering SIGINT handler");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error registering SIGTERM handler");
        return 1;
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
    char *filename = NULL;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbtmvdr:o:n:", long_options, NULL)) != -1) {
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
                case OPT_NUM_CORES:
                    cores = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 1);
                    break;
                case OPT_MAX_THREADS:
                    max_threads = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 1);
                    break;
                case OPT_DO_SWAP:
                    global_config.swap = true;
                    break;
                case OPT_SWAP_THRESHOLD:
                    global_config.swapThreshold = parseUINT64(optarg, 0xFFFFFFFFFFFFFFFF, 0);
                    break;
                case OPT_RESULT_FILENAME: {
                    size_t len = strlen(optarg);
                    filename = malloc(len + 1);
                    mallocCheck(filename);
                    strncpy(filename, optarg, len + 1);
                    break;
                }
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

    if (global_config.verbose && global_config.swap)
        printf("Swapping is activated with threshold: %lu MB\n", global_config.swapThreshold);

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
    if (filename == NULL) {
        size_t len = strlen(DEFAULT_FILENAME);
        filename = malloc(len + 1);
        mallocCheck(filename);
        strncpy(filename, DEFAULT_FILENAME, len + 1);
    }
    if (global_config.verbose && output == 'f')
        printf("Writing result in file: %s\n", filename);

    if (do_test) {
        test();
    } else if (do_debug) {
        bruteForceDebug();
    } else if (do_benchmark) {
        benchMark();
    } else {
        printFibonacci(n, radix, output, filename);
    }

    free(filename);
    cleanupBigIntLib();
    return EXIT_SUCCESS;
}

void printFibonacci(uint64_t n, char radix, char output, char *filename) {
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for n=%zu | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    bigInt *res = fibonacci(n);

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

        if (output == 'f') {
            // File output
            char infoStr[1000];
            sprintf(infoStr, "Result for n=%zu | length of string=%zu:\n", n, strSizeInBytes);
            if (writeFile(filename, infoStr, false) == -1) exit(EXIT_FAILURE);
            if (writeFile(filename, resString, true) == -1) exit(EXIT_FAILURE);
            if (global_config.verbose) printf("Result written into file %s\n", filename);
        } else {
            //Terminal output (output == 't')
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
    char *helpMenuText = "Usage:\n"
                         "fibonacci:  -o -> output f|t|n (f=file, t=terminal, n=none); default value: t\n"
                         "            -r -> radix d|h (d=decimal, h=hexadecimal); default value: h\n"
                         "            -m -> enables multithreading; default value: false\n"
                         "            -n -> nth-fibonacci number; default value: 0 | n needs to be a positive 64bit integer\n"
                         "\n"
                         "            e.g.:\n "
                         "                  Calculate 10000000t fibonacci number and write the result in decimal into the file res.txt\n"
                         "                  ./fib -o f -r d --result-filename res.txt -n 10000000\n"
                         "                  Same as before, now with verbose output and swap activated if a bigInt is bigger than 1 MB\n"
                         "                  ./fib -o f -r d --result-filename res.txt -v --do-swap --swap-threshold 1 -n 10000000 \n"
                         "\n"
                         "            The default filename is output.txt | if the file exists, it will be overwritten\n"
                         "\n"
                         "miscellaneous:  -h -> display this help message\n"
                         "                -d -> debug\n"
                         "                -t -> test\n"
                         "                -b -> benchMark\n"
                         "                -v -> verbose\n"
                         "                --result-filename -> set filename for output file\n";
    printf("%s\n", helpMenuText);
}
