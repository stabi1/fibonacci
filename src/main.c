#include "test/tests.h"
#include "test/parseTestArgs.h"
#include "bigNum/bigInt/bigInt.h"
#include "util.h"
#include "bigNum/misc.h"
#include "computeMethods.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

enum computeOperation getComputeOperation(char *token);

void printHelpMenu();

const char *DEFAULT_FILENAME = "output.txt";

// TODO: look at add/sub if signs and size are optimal
// TODO: better error messages in bigIntIO
// TODO: caching for the bigFrac struct
// TODO: implement swapping for goldenRation and Pi, check bigFrac for swapping potential
// TODO: better root algorithm (Karatsuba Square Root), root for bigInt
// TODO: Use linter and fix code, docu with comments
// TODO: run more tests in valgrind
// TODO: make way more tests (tests with all the different features en-/disabled) (allways check partial bigInt support)
// TODO: update readme, update help-message

// Target time for ./fib -o f -r d -c pi 1000000 -v = 0.5s
// is 5.5

enum {
    OPT_MAX_THREADS = 1001,
    OPT_NUM_CORES,
    OPT_RESULT_FILENAME,
    OPT_DO_SWAP,
    OPT_SWAP_THRESHOLD,
    OPT_INPUT_FILENAME,
    OPT_INPUT_RADIX,
    OPT_DEACTIVATE_CACHES,
    OPT_INFO_IN_OUTPUTFILE,
    OPT_SUPER_VERBOSE
};

const size_t numOfArgsComputeOperation[] = {
        [CONVERT_NUMBER]    = 0,
        [GOLDEN_RATIO]      = 1,
        [FIBONACCI]         = 1,
        [SQUARE_ROOT]       = 2,
        [PI]                = 1,
        [E]                 = 1,
        [UNKNOWN_OPERATION] = 0,
};

static struct option long_options[] = {
        {"help",               no_argument,       NULL, 'h'},
        {"output-format",      required_argument, NULL, 'o'},
        {"multithread",        no_argument,       NULL, 'm'},
        {"output-radix",       required_argument, NULL, 'r'},
        {"debug",              no_argument,       NULL, 'd'},
        {"benchMark",          no_argument,       NULL, 'b'},
        {"test",               required_argument, NULL, 't'},
        {"compute",            required_argument, NULL, 'c'},
        {"verbose",            no_argument,       NULL, 'v'},
        {"super-verbose",      no_argument,       NULL, OPT_SUPER_VERBOSE},
        {"max-threads",        required_argument, NULL, OPT_MAX_THREADS},
        {"num-cores",          required_argument, NULL, OPT_NUM_CORES},
        {"output-filename",    required_argument, NULL, OPT_RESULT_FILENAME},
        {"info-in-outputfile", no_argument,       NULL, OPT_INFO_IN_OUTPUTFILE},
        {"do-swap",            no_argument,       NULL, OPT_DO_SWAP},
        {"swap-threshold",     required_argument, NULL, OPT_SWAP_THRESHOLD},
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
    bool do_debug = false;
    bool do_test = false;
    bool do_benchmark = false;
    char *outputFilename = NULL;
    bool infoInOutputFile = false;

    uint64_t computeNumberArgument1 = 0;
    uint64_t computeNumberArgument2 = 0;
    enum computeOperation computeOperation = UNKNOWN_OPERATION;

    char inputRadix = '\0';
    char *inputFilename = NULL;
    char *testArgs = NULL;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbt:mvdr:o:c:", long_options, NULL)) != -1) {
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
                case OPT_SUPER_VERBOSE:
                    global_config.verbose = true;
                    global_config.superVerbose = true;
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
                case 'c':
                    if (optind < argc) {
                        computeOperation = getComputeOperation(optarg);
                        if (computeOperation == UNKNOWN_OPERATION) {
                            fprintf(stderr, "Unknown compute Operation\n");
                            exit(EXIT_FAILURE);
                        }
                        if (numOfArgsComputeOperation[computeOperation] >= 1) {
                            computeNumberArgument1 = parseUINT64(argv[optind], UINT64_MAX, 0);
                            optind++;
                        }
                        if (numOfArgsComputeOperation[computeOperation] >= 2) {
                            if (optind >= argc) {
                                fprintf(stderr, "Option -c requires a operation and a number (-c <operation> <number> <number>)\n");
                                exit(EXIT_FAILURE);

                            }
                            computeNumberArgument2 = parseUINT64(argv[optind], UINT64_MAX, 0);
                            optind++;
                        }
                    } else {
                        fprintf(stderr, "Option -c requires a operation and a number (-c <operation> <number>)\n");
                        exit(EXIT_FAILURE);
                    }
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

    if (global_config.superVerbose) {
        printf("Super Verbose activated\n");
    }
    if (global_config.verbose && global_config.deactivateCaches) {
        printf("Caches are deactivated\n");
    }
    if (global_config.verbose && global_config.swap) {
        printf("Swapping is activated with threshold: %lu MB\n", global_config.swapThreshold);
    }

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
    if (global_config.verbose && output == 'f') {
        printf("Result will be written in file: %s\n", outputFilename);
    }

    if (do_test) {
        printf("Running Tests, ignoring set input/output filename\n");
        selectTest(testArgs);
    } else if (do_debug) {
        bruteForceDebug();
    } else if (do_benchmark) {
        benchMark();
    } else {
        switch (computeOperation) {
            case CONVERT_NUMBER:
                if (inputRadix == '\0') {
                    fprintf(stderr, "input-radix required for convert-number\n");
                    exit(EXIT_FAILURE);
                } else if (inputFilename == NULL) {
                    fprintf(stderr, "input-filename required for convert-number\n");
                    exit(EXIT_FAILURE);
                }
                convertNumber(inputRadix, inputFilename, outputRadix, outputFilename);
                break;
            case FIBONACCI:
                printFibonacci(computeNumberArgument1, outputRadix, output, outputFilename, infoInOutputFile);
                break;
            case GOLDEN_RATIO:
                printGoldenRatio(computeNumberArgument1, outputRadix, output, outputFilename, infoInOutputFile);
                break;
            case SQUARE_ROOT:
                printSquareRoot(computeNumberArgument1, computeNumberArgument2, outputRadix, output, outputFilename, infoInOutputFile);
                break;
            case PI:
                printPi(computeNumberArgument1, outputRadix, output, outputFilename, infoInOutputFile);
                break;
            case E:
                print_e(computeNumberArgument1, outputRadix, output, outputFilename, infoInOutputFile);
                break;
            case UNKNOWN_OPERATION:
                printf("No operation selected\n");
                break;
        }
    }

    free(outputFilename);
    free(inputFilename);
    cleanupBigIntLib();
    return EXIT_SUCCESS;
}

enum computeOperation getComputeOperation(char *token) {
    if (strcmp(token, "convert-number") == 0) {
        return CONVERT_NUMBER;
    } else if (strcmp(token, "fibonacci") == 0) {
        return FIBONACCI;
    } else if (strcmp(token, "golden-ratio") == 0) {
        return GOLDEN_RATIO;
    } else if (strcmp(token, "square-root") == 0) {
        return SQUARE_ROOT;
    } else if (strcmp(token, "pi") == 0) {
        return PI;
    } else if (strcmp(token, "e") == 0) {
        return E;
    } else {
        return UNKNOWN_OPERATION;
    }
}

void printHelpMenu() {
    char *helpMenuText = "Usage:\n"
                         "fibonacci:  -o -> output f|t|n (f=file, t=terminal, n=none); default value: t\n"
                         "            -r -> radix d|h (d=decimal, h=hexadecimal); default value: h\n"
                         "            -m -> enables multithreading; default value: false\n"
                         "            -c -> compute\n"
                         "\n"
                         "            e.g.:\n "
                         "                  Calculate 10000000t fibonacci number and write the result in decimal into the file res.txt\n"
                         "                  ./fib -o f -r d --output-filename res.txt -c fibonacci 10000000\n"
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
