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
#include <signal.h>

enum computeOperation getComputeOperation(const char *token);

void printHelpMenu();

const char *DEFAULT_FILENAME = "output.txt";

// Ideas:
// - implement swapping for goldenRation and Pi, check bigFrac for swapping potential
// - better root algorithm (Karatsuba Square Root), root for bigInt
// - clean up cores/threads mess
// - change -b flag, make current functionality a test argument


enum {
    OPT_MAX_THREADS = 1001,
    OPT_MIN_THREADS,
    OPT_RESULT_FILENAME,
    OPT_DO_SWAP,
    OPT_SWAP_THRESHOLD,
    OPT_INPUT_FILENAME,
    OPT_INPUT_RADIX,
    OPT_DEACTIVATE_CACHES,
    OPT_INFO_IN_OUTPUTFILE,
    OPT_SUPER_VERBOSE,
    OPT_MUL_THRESHOLDS,
    OPT_DIV_THRESHOLDS,
    OPT_K_VALUES,
};

constexpr size_t numOfArgsComputeOperation[] = {
    [CONVERT_NUMBER]    = 0,
    [GOLDEN_RATIO]      = 1,
    [FIBONACCI]         = 1,
    [SQUARE_ROOT]       = 2,
    [PI]                = 1,
    [E]                 = 1,
    [UNKNOWN_OPERATION] = 0,
};

static struct option long_options[] = {
    {"help",               no_argument,       nullptr, 'h'},
    {"output-format",      required_argument, nullptr, 'o'},
    {"multithread",        no_argument,       nullptr, 'm'},
    {"output-radix",       required_argument, nullptr, 'r'},
    {"debug",              no_argument,       nullptr, 'd'},
    {"benchMark",          no_argument,       nullptr, 'b'},
    {"test",               required_argument, nullptr, 't'},
    {"compute",            required_argument, nullptr, 'c'},
    {"verbose",            no_argument,       nullptr, 'v'},
    {"super-verbose",      no_argument,       nullptr, OPT_SUPER_VERBOSE},
    {"max-threads",        required_argument, nullptr, OPT_MAX_THREADS},
    {"num-cores",          required_argument, nullptr, OPT_MIN_THREADS},
    {"output-filename",    required_argument, nullptr, OPT_RESULT_FILENAME},
    {"info-in-outputfile", no_argument,       nullptr, OPT_INFO_IN_OUTPUTFILE},
    {"do-swap",            no_argument,       nullptr, OPT_DO_SWAP},
    {"swap-threshold",     required_argument, nullptr, OPT_SWAP_THRESHOLD},
    {"input-filename",     required_argument, nullptr, OPT_INPUT_FILENAME},
    {"input-radix",        required_argument, nullptr, OPT_INPUT_RADIX},
    {"deactivate-caches",  no_argument,       nullptr, OPT_DEACTIVATE_CACHES},
    {"mul-thresholds",     required_argument, nullptr, OPT_MUL_THRESHOLDS},
    {"div-thresholds",     required_argument, nullptr, OPT_DIV_THRESHOLDS},
    {"k-values",           required_argument, nullptr, OPT_K_VALUES},
    {nullptr, 0,                              nullptr, 0}
};


int main(const int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "No arguments, use -h for usage\n");
        exit(EXIT_FAILURE);
    }

    // signal handler
    struct sigaction sa = {0};
    sa.sa_sigaction = handleSignals;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("Error registering SIGINT handler");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        perror("Error registering SIGTERM handler");
        return 1;
    }

    // default values
    char outputRadix = 'h';
    char output = 't';
    size_t min_threads = 0; //available threads
    size_t max_threads = 0;
    bool do_debug = false;
    bool do_test = false;
    bool do_benchmark = false;
    char *outputFilename = nullptr;
    bool infoInOutputFile = false;
    bool mulThresholdSet = false;
    bool divThresholdSet = false;
    bool kValuesSet = false;


    uint64_t computeNumberArgument1 = 0;
    uint64_t computeNumberArgument2 = 0;
    enum computeOperation computeOperation = UNKNOWN_OPERATION;

    char inputRadix = '\0';
    char *inputFilename = nullptr;
    char *testArgs = nullptr;

    int option;

    while (optind < argc) {
        if ((option = getopt_long(argc, argv, "+hbt:mvdr:o:c:", long_options, nullptr)) != -1) {
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
                    global_config.measureTime = true;
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
                case OPT_MIN_THREADS:
                    min_threads = parseUINT64(optarg, UINT64_MAX, 1);
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
                case OPT_MUL_THRESHOLDS: {
                    constexpr size_t mulLen = 4;
                    uint64_t res[mulLen];
                    parseUINT64List(res, optarg, mulLen);
                    global_config.mulThresholds.NAIVE_MUL_FASTER = res[0];
                    global_config.mulThresholds.KARATSUBA_FASTER = res[1];
                    global_config.mulThresholds.TOOM_COOK_FASTER = res[2];
                    global_config.mulThresholds.SSA_SMALL_FASTER = res[3];
                    mulThresholdSet = true;
                    break;
                }
                case OPT_DIV_THRESHOLDS: {
                    constexpr size_t divLen = 1;
                    uint64_t res[divLen];
                    parseUINT64List(res, optarg, divLen);
                    global_config.divThresholds.D4_FASTER = res[0];
                    divThresholdSet = true;
                    break;
                }
                case OPT_K_VALUES: {
                    constexpr size_t kLen = K_VALUES_COUNT;
                    parseUINT64List(global_config.kValuesSsaSmall, optarg, kLen);
                    kValuesSet = true;
                    break;
                }
                case OPT_RESULT_FILENAME: {
                    const size_t len = strlen(optarg);
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
                    const size_t len = strlen(optarg);
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
        printf("Swapping is activated with threshold: %zu MB\n", global_config.swapThreshold);
    }
    if ((global_config.verbose && mulThresholdSet) || global_config.superVerbose) {
        printf("Mul Thresholds - naive mul faster: %lu; karatsuba faster: %lu; Toom-Cook faster: %lu; SSA-small faster: %lu\n",
               global_config.mulThresholds.NAIVE_MUL_FASTER, global_config.mulThresholds.KARATSUBA_FASTER,
               global_config.mulThresholds.TOOM_COOK_FASTER, global_config.mulThresholds.SSA_SMALL_FASTER);
    }
    if ((global_config.verbose && divThresholdSet) || global_config.superVerbose) {
        printf("Div Thresholds: D4 faster: %lu \n", global_config.divThresholds.D4_FASTER);
    }
    if ((global_config.verbose && kValuesSet) || global_config.superVerbose) {
        printf("KValues: %zu", global_config.kValuesSsaSmall[0]);
        for (size_t i = 1; i < K_VALUES_COUNT; i++) {
            printf(",%zu: ", global_config.kValuesSsaSmall[i]);
        }
        printf("\n");
    }

    if (global_config.parallel) {
        printf("Multithreading enabled\n");
        if (min_threads != 0 && max_threads != 0) {
            fprintf(stderr, "Options p and n are mutually exclusive");
            exit(EXIT_FAILURE);
        }
        if (min_threads != 0) {
            global_config.maxThreads = min_threads;
            global_config.mulDepth = getMulDepthFromCores(min_threads);
            global_config.convertDepth = getConvertDepthFromCores(min_threads);
        } else {
            global_config.maxThreads = max_threads;
            global_config.mulDepth = getMulDepthFromMaxThreads(max_threads);
            global_config.convertDepth = getConvertDepthFromMaxThreads(max_threads);
        }
    }
    if (outputFilename == NULL) {
        const size_t len = strlen(DEFAULT_FILENAME);
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

enum computeOperation getComputeOperation(const char *token) {
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
    const char *helpMenuText = "Usage:\n"
            "main options:  -o -> output f|t|n (f=file, t=terminal, n=none); default value: t\n"
            "               -r -> radix d|h (d=decimal, h=hexadecimal); default value: h\n"
            "               -m -> enables multithreading; default value: false\n"
            "               -c -> compute [arg]:\n"
            "                   possible args:\n"
            "                       e <precision>\n"
            "                       pi <precision>\n"
            "                       square-root <number> <precision>\n"
            "                       golden-ratio <precision>\n"
            "                       fibonacci <n>\n"
            "                       convert-number -> uses input and outfile and input and output radix \n"
            "\n"
            "               e.g.:\n "
            "                  Calculate 10000000t fibonacci number and write the result in decimal into the file res.txt\n"
            "                  ./fib -o f -r d --output-filename res.txt -c fibonacci 10000000\n"
            "                  Same as before, now with verbose output and swap activated if a bigInt is bigger than 1 MB and multithreading\n"
            "                  ./fib -o f -r d --output-filename res.txt -m -v --do-swap --swap-threshold 1 -c fibonacci 10000000 \n"
            "\n"

            "\n"
            "miscellaneous:  -h -> display this help message\n"
            "                -d -> fib debug, tests all fib numbers\n"
            "                -t -> test [argument]\n"
            "                -b -> run benchmark, with -t: measure time\n"
            "                -v -> verbose, more output\n"
            "                --super-verbose -> even more output\n"
            "                --output-filename -> set filename for output file\n"
            "                   -> The default filename is output.txt | if the file exists, it will be overwritten\n"
            "                --input-filename -> set filename for input file\n"
            "                --input-radix -> radix of the input file\n"
            "                --max-threads -> set maximum number of threads the program is allowed to use\n"
            "                --min-threads -> set minimum number of threads (can be more that the machine has)\n"
            "                   -> if neither option is set, the number of threads will be auto-detected\n"
            "                --info-in-outputfile -> prints info about the number in it into the output file \n"
            "                --do-swap -> activates swaping \n"
            "                --swap-threshold -> sets the threshold in MB after which a number will be swaped \n"
            "                --deactivate-caches -> deactivates caching of bigInt/bigFrac structs and bigInt arrays\n"
            "                --mul-thresholds -> sets mul thresholds manually\n"
            "                --div-thresholds -> sets div thresholds manually\n"
            "                --k-values -> sets k values manually\n";

    printf("%s\n", helpMenuText);
}
