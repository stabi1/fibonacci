#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tests.h"
#include "testMethods.h"
#include "../bigNum/misc.h"

enum TestType {
    CUSTOM_TEST = 0,
    MUL,
    DIV,
    DIV_MOD,
    ADD,
    SUB,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    SHIFT_ADD,
    PRINT_HEX,
    PRINT_DEC
};

const size_t numOfArgs[] = {
        [CUSTOM_TEST] = 0,
        [MUL]         = 3,
        [DIV]         = 3,
        [DIV_MOD]     = 4,
        [ADD]         = 3,
        [SUB]         = 3,
        [SHIFT_LEFT]  = 3,
        [SHIFT_RIGHT] = 3,
        [SHIFT_ADD]   = 4,
        [PRINT_HEX]   = 2,
        [PRINT_DEC]   = 2,
};

void onError(char *argsCpy) {
    free(argsCpy);
    exit(EXIT_FAILURE);
}

void selectTest(char *args) {
    //copy args
    size_t argLen = strlen(args);
    char *argsCpy = malloc(argLen + 1);
    mallocCheck(argsCpy);
    strncpy(argsCpy, args, argLen + 1);
    argsCpy[argLen] = '\0';

    // extract arguments
    char *delim = ",";
    char *testType = strtok(argsCpy, delim);
    char *arg1 = strtok(NULL, delim);
    char *arg2 = strtok(NULL, delim);
    char *arg3 = strtok(NULL, delim);
    char *arg4 = strtok(NULL, delim);
    char *overflow = strtok(NULL, delim);

    // determine test type
    enum TestType type;
    if (strcmp(testType, "customTest") == 0) {
        type = CUSTOM_TEST;
    } else if (strcmp(testType, "mul") == 0) {
        type = MUL;
    } else if (strcmp(testType, "div") == 0) {
        type = DIV;
    } else if (strcmp(testType, "divMod") == 0) {
        type = DIV_MOD;
    } else if (strcmp(testType, "add") == 0) {
        type = ADD;
    } else if (strcmp(testType, "sub") == 0) {
        type = SUB;
    } else if (strcmp(testType, "shiftLeft") == 0) {
        type = SHIFT_LEFT;
    } else if (strcmp(testType, "shiftRight") == 0) {
        type = SHIFT_RIGHT;
    } else if (strcmp(testType, "shiftAdd") == 0) {
        type = SHIFT_ADD;
    } else if (strcmp(testType, "printHex") == 0) {
        type = PRINT_HEX;
    } else if (strcmp(testType, "printDec") == 0) {
        type = PRINT_DEC;
    } else {
        fprintf(stderr, "Unknown test type\n");
        free(argsCpy);
        exit(EXIT_FAILURE);
    }

    size_t numArgs = numOfArgs[type];
    if (numArgs == 1) {
        if (arg1 == NULL) {
            fprintf(stderr, "Missing argument\n");
            onError(argsCpy);
        } else if (arg2 != NULL) {
            fprintf(stderr, "To many arguments\n");
            onError(argsCpy);
        }
    } else if (numArgs == 2) {
        if (arg1 == NULL || arg2 == NULL) {
            fprintf(stderr, "Missing argument\n");
            onError(argsCpy);
        } else if (arg3 != NULL) {
            fprintf(stderr, "To many arguments\n");
            onError(argsCpy);
        }
    } else if (numArgs == 3) {
        if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
            fprintf(stderr, "Missing arguments\n");
            onError(argsCpy);
        } else if (arg4 != NULL) {
            fprintf(stderr, "To many arguments\n");
            onError(argsCpy);
        }
    } else if (numArgs == 4) {
        if (arg1 == NULL || arg2 == NULL || arg3 == NULL || arg4 == NULL) {
            fprintf(stderr, "Missing arguments\n");
            onError(argsCpy);
        } else if (overflow != NULL) {
            fprintf(stderr, "To many arguments\n");
            onError(argsCpy);
        }
    }

    switch (type) {
        case CUSTOM_TEST:
            customTest();
            break;
        case MUL:
            testMul(arg1, arg2, arg3);
            break;
        case ADD:
            testAdd(arg1, arg2, arg3);
            break;
        case SUB:
            testSub(arg1, arg2, arg3);
            break;
        case SHIFT_LEFT:
            testShiftLeft(arg1, arg2, arg3);
            break;
        case SHIFT_RIGHT:
            testShiftRight(arg1, arg2, arg3);
            break;
        case DIV:
            testDiv(arg1, arg2, arg3);
            break;
        case DIV_MOD:
            testDivMod(arg1, arg2, arg3, arg4);
            break;
        case SHIFT_ADD:
            testSHiftAdd(arg1, arg2, arg3, arg4);
            break;
        case PRINT_HEX:
            testWriteBigIntHexToFile(arg1, arg2);
            break;
        case PRINT_DEC:
            testWriteBigIntDecToFile(arg1, arg2);
            break;
    }

    free(argsCpy);
}
