#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include "test/tests.h"
#include "test/parseTestArgs.h"
#include "bigInt/bigInt.h"
#include "util.h"

void printHelpMenu();

void printFibonacci(uint64_t n, char radix, char output, char *filename, bool infoInOutputFile);

void doConvertNumber(char inputRadix, char *inputFilename, char outputRadix, char *outputFilename);

const char *DEFAULT_FILENAME = "output.txt";


//TODO run more tests in valgrind
//TODO make way more tests (tests with all the different features en-/disabled) (allways check partial bigInt support)
//TODO fix makefile
//TODO docu with comments, readme, help-message

enum {
    OPT_MAX_THREADS = 1001,
    OPT_NUM_CORES,
    OPT_RESULT_FILENAME,
    OPT_DO_SWAP,
    OPT_SWAP_THRESHOLD,
    OPT_CONVERT_NUMBER,
    OPT_INPUT_FILENAME,
    OPT_INPUT_RADIX,
    OPT_DEACTIVATE_CACHES,
    OPT_INFO_IN_OUTPUTFILE
};

static struct option long_options[] = {
        {"help",               no_argument,       NULL, 'h'},
        {"output-format",      required_argument, NULL, 'o'},
        {"multithread",        no_argument,       NULL, 'm'},
        {"output-radix",       required_argument, NULL, 'r'},
        {"debug",              no_argument,       NULL, 'd'},
        {"benchMark",          no_argument,       NULL, 'b'},
        {"test",               required_argument, NULL, 't'},
        {"nth-fibonacci",      required_argument, NULL, 'n'},
        {"verbose",            no_argument,       NULL, 'v'},
        {"max-threads",        required_argument, NULL, OPT_MAX_THREADS},
        {"num-cores",          required_argument, NULL, OPT_NUM_CORES},
        {"output-filename",    required_argument, NULL, OPT_RESULT_FILENAME},
        {"info-in-outputfile", no_argument,       NULL, OPT_INFO_IN_OUTPUTFILE},
        {"do-swap",            no_argument,       NULL, OPT_DO_SWAP},
        {"swap-threshold",     required_argument, NULL, OPT_SWAP_THRESHOLD},
        {"convert-number",     no_argument,       NULL, OPT_CONVERT_NUMBER},
        {"input-filename",     required_argument, NULL, OPT_INPUT_FILENAME},
        {"input-radix",        required_argument, NULL, OPT_INPUT_RADIX},
        {"deactivate-caches",  no_argument,       NULL, OPT_DEACTIVATE_CACHES},
        {NULL, 0,                                 NULL, 0}
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
    char outputRadix = 'h';
    char output = 't';
    size_t cores = 0; //available cores
    size_t max_threads = 0;
    uint64_t n = 0;
    bool do_debug = false;
    bool do_test = false;
    bool do_benchmark = false;
    char *outputFilename = NULL;
    bool infoInOutputFile = false;

    bool convertNumber = false;
    char inputRadix = '\0';
    char *inputFilename = NULL;
    char *testArgs = NULL;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbt:mvdr:o:n:", long_options, NULL)) != -1) {
            switch (option) {
                case 'h':
                    printHelpMenu();
                    exit(EXIT_SUCCESS);
                case 'd':
                    do_debug = true;
                    break;
                case 't':
                    do_test = true;
                    testArgs = optarg;
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
                    cores = parseUINT64(optarg, UINT64_MAX, 1);
                    break;
                case OPT_MAX_THREADS:
                    max_threads = parseUINT64(optarg, UINT64_MAX, 1);
                    break;
                case OPT_DO_SWAP:
                    global_config.swap = true;
                    break;
                case OPT_SWAP_THRESHOLD:
                    global_config.swapThreshold = parseUINT64(optarg, UINT64_MAX, 0);
                    break;
                case OPT_RESULT_FILENAME: {
                    size_t len = strlen(optarg);
                    outputFilename = malloc(len + 1);
                    mallocCheck(outputFilename);
                    strncpy(outputFilename, optarg, len + 1);
                    break;
                }
                case OPT_INFO_IN_OUTPUTFILE:
                    infoInOutputFile = true;
                    break;
                case 'r':
                    if ((optarg[0] != 'h' && optarg[0] != 'd') || optarg[1] != '\0') {
                        fprintf(stderr, "invalid radix option \"%s\" provided, use -h for usage\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    outputRadix = optarg[0];
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
                    n = parseUINT64(optarg, UINT64_MAX, 0);
                    break;
                case OPT_CONVERT_NUMBER:
                    convertNumber = true;
                    break;
                case OPT_INPUT_FILENAME: {
                    size_t len = strlen(optarg);
                    inputFilename = malloc(len + 1);
                    mallocCheck(inputFilename);
                    strncpy(inputFilename, optarg, len + 1);
                    break;
                }
                case OPT_INPUT_RADIX:
                    if ((optarg[0] != 'h' && optarg[0] != 'd') || optarg[1] != '\0') {
                        fprintf(stderr, "invalid radix option \"%s\" provided, use -h for usage\n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    inputRadix = optarg[0];
                    break;
                case OPT_DEACTIVATE_CACHES:
                    global_config.deactivateCaches = true;
                    break;
                default:
                    fprintf(stderr, "Invalid input formatting for fibonacci, use -h for usage\n");
                    exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "No non option argument expected, got \"%s\" ;  use -h for usage\n", argv[optind]);
            optind++;
            exit(EXIT_FAILURE);
        }
    }

    if (global_config.verbose && global_config.deactivateCaches)
        printf("Caches are deactivated\n");
    if (global_config.verbose && global_config.swap)
        printf("Swapping is activated with threshold: %lu MB\n", global_config.swapThreshold);

    if (global_config.parallel) {
        printf("Multithreading enabled\n");
        if (cores != 0 && max_threads != 0) {
            fprintf(stderr, "Options p and n are mutually exclusive");
            exit(EXIT_FAILURE);
        }
        if (cores != 0) {
            global_config.maxThreads = cores;
            global_config.mulDepth = getMulDepthFromCores(cores);
            global_config.convertDepth = getConvertDepthFromCores(cores);
        } else {
            global_config.maxThreads = max_threads;
            global_config.mulDepth = getMulDepthFromMaxThreads(max_threads);
            global_config.convertDepth = getConvertDepthFromMaxThreads(max_threads);
        }
    }
    if (outputFilename == NULL) {
        size_t len = strlen(DEFAULT_FILENAME);
        outputFilename = malloc(len + 1);
        mallocCheck(outputFilename);
        strncpy(outputFilename, DEFAULT_FILENAME, len + 1);
    }
    if (global_config.verbose && output == 'f')
        printf("Writing result in file: %s\n", outputFilename);

    if (do_test) {
        printf("Running Tests, ignoring set input/output filename\n");
        selectTest(testArgs);
    } else if (do_debug) {
        bruteForceDebug();
    } else if (do_benchmark) {
        benchMark();
    } else if (convertNumber) {
        if (inputRadix == '\0') {
            fprintf(stderr, "input-radix required for convert-number\n");
            exit(EXIT_FAILURE);
        } else if (inputFilename == NULL) {
            fprintf(stderr, "input-filename required for convert-number\n");
            exit(EXIT_FAILURE);
        }
        doConvertNumber(inputRadix, inputFilename, outputRadix, outputFilename);
    } else {
        printFibonacci(n, outputRadix, output, outputFilename, infoInOutputFile);
    }

    free(outputFilename);
    free(inputFilename);
    cleanupBigIntLib();
    return EXIT_SUCCESS;
}

void printFibonacci(uint64_t n, char radix, char output, char *filename, bool infoInOutputFile) {
    size_t estimatedSizeInBytes = (size_t) (0.0868 * (double) n + 3.8275);
    double sizeInMBEst = ((double) estimatedSizeInBytes) / 1000000;
    printf("Starting calculation for n=%zu | estimated size in bytes:%zu in MB:%0.2f\n", n, estimatedSizeInBytes,
           sizeInMBEst);

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

    bigInt *res = fibonacci(n);

    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);

    size_t sizeInBytes = (res->end - res->start) * 8;

    if (output == 't' || output == 'f') {
        struct timespec start2;
        if (clock_gettime(CLOCK_MONOTONIC, &start2) == -1) perror("Error measuring time!");
        size_t strSizeInBytes;
        char *resString;
        char *localTime = getCurrentDateTime();
        if (radix == 'd') {
            if (global_config.verbose) printf("Starting conversion to dec; %s\n", localTime);
            free(localTime);
            resString = bigIntToDecString(res, true);
            strSizeInBytes = strlen(resString);
        } else {
            if (global_config.verbose) printf("Starting conversion to hex; %s\n", localTime);
            free(localTime);
            resString = bigIntToHexString(res);
            freeBigInt(res);
            strSizeInBytes = strlen(resString);
        }

        if (output == 'f') {
            // File output
            if (infoInOutputFile) {
                char infoStr[200];
                sprintf(infoStr, "Result for n=%zu | length of string=%zu:\n", n, strSizeInBytes);
                if (writeFile(filename, infoStr, false) == -1) exit(EXIT_FAILURE);
                if (writeFile(filename, resString, true) == -1) exit(EXIT_FAILURE);
            } else {
                if (writeFile(filename, resString, false) == -1) exit(EXIT_FAILURE);
            }

            if (global_config.verbose) printf("Result written into file %s\n", filename);
        } else {
            // Terminal output: output == 't'
            printf("Result: %s\n", resString);
        }

        struct timespec end2;
        if (clock_gettime(CLOCK_MONOTONIC, &end2) == -1) perror("Error measuring time!");
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
}

void doConvertNumber(char inputRadix, char *inputFilename, char outputRadix, char *outputFilename) {
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
    if( clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Reading from file %s and converting to bigInt done, took %.2f seconds\n", inputFilename, time);

    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    if (outputRadix == 'd') {
        writeBigIntDecToFile(tmp, outputFilename, true);
    } else {
        writeBigIntHexToFile(tmp, outputFilename);
        freeBigInt(tmp);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end)  == -1) perror("Error measuring time!");
    time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Finished conversion, result in %s, took %.2f seconds\n", outputFilename, time);
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
                         "                  ./fib -o f -r d --output-filename res.txt -n 10000000\n"
                         "                  Same as before, now with verbose output and swap activated if a bigInt is bigger than 1 MB\n"
                         "                  ./fib -o f -r d --output-filename res.txt -v --do-swap --swap-threshold 1 -n 10000000 \n"
                         "\n"
                         "            The default filename is output.txt | if the file exists, it will be overwritten\n"
                         "\n"
                         "miscellaneous:  -h -> display this help message\n"
                         "                -d -> debug\n"
                         "                -t -> test\n"
                         "                -b -> benchMark\n"
                         "                -v -> verbose\n"
                         "                --output-filename -> set filename for output file\n";
    printf("%s\n", helpMenuText);
}
