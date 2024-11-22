#include <stdio.h>
#include <sys/sysinfo.h>
#include "config.h"
#include "bigIntAlloc.h"

Config global_config = {
        .verbose = false,
        .parallel = false,
        .mulDepth = 0,
        .swap = false,
        .swapThreshold = 100
};

void cleanupBigIntLib() {
    //clear bigIntStructStack
    free_BigIntStack(get_thread_BigIntStack());
}

size_t getMulDepthFromMaxThreads(size_t maxThreads) {
    if (maxThreads == 0) {
        return getMulDepthFromCores(0);
    }

    if (maxThreads < 5) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 1);
        return 0;
    } else if (maxThreads < 25) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 5);
        return 1;
    } else if (maxThreads < 125) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 25);
        return 2;
    } else {
        printf("Number of compute threads that will be created during mul: %d\n", 125);
        return 3;
    }
}

size_t getMulDepthFromCores(size_t nprocsSet) {
    size_t numberOfCores;
    if (nprocsSet == 0) {
        numberOfCores = get_nprocs();
        if (global_config.verbose) printf("Number of cores detected: %lu\n", numberOfCores);
    } else {
        numberOfCores = nprocsSet;
        if (global_config.verbose) printf("Number of cores set: %lu\n", numberOfCores);
    }

    if (numberOfCores < 6) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 5);
        return 1;
    } else if (numberOfCores < 26) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 25);
        return 2;
    } else if (numberOfCores < 126) {
        if (global_config.verbose) printf("Number of compute threads that will be created during mul: %d\n", 125);
        return 3;
    } else {
        printf("Number of compute threads that will be created during mul: %d\n", 625);
        return 4;
    }
}