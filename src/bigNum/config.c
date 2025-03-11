#include <stdio.h>
#include <sys/sysinfo.h>
#include "config.h"
#include "bigInt/bigIntAlloc.h"

Config global_config = {
        .verbose = false,
        .parallel = false,
        .mulDepth = 0,
        .convertDepth = 0,
        .swap = false,
        .swapThreshold = 100,
        .deactivateCaches = false,
        .maxThreads = 1
};

void cleanupBigIntLib() {
    //clear bigIntStructStack
    free_BigIntStack(get_thread_BigIntStack());
    free_BigIntStack(get_thread_bigIntArrayStack_1KB());
    free_BigIntStack(get_thread_bigIntArrayStack_10KB());
    free_BigIntStack(get_thread_bigIntArrayStack_100KB());
}

size_t getNumOfThreadsFromDepthMul(size_t mulDepth) {
    switch (mulDepth) {
        case 0:
            return 1;
        case 1:
            return 5;
        case 2:
            return 25;
        default:
            return 125;
    }
}


size_t calcMulDepthForParallelMuls(size_t numMuls) {
    size_t resDepth = 0;
    while (true) {
        if (numMuls * getNumOfThreadsFromDepthMul(resDepth) > global_config.maxThreads)
            break;
        resDepth++;
    }
    return resDepth == 0 ? 0 : resDepth - 1;
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

size_t getConvertDepthFromMaxThreads(size_t maxThreads) {
    if (maxThreads == 0) {
        return getConvertDepthFromCores(0);
    }

    if (maxThreads < 2) {
        if (global_config.verbose) printf("Number of convert compute threads that will be created during conversion: %d\n", 1);
        return 0;
    } else if (maxThreads < 4) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 2);
        return 1;
    } else if (maxThreads < 8) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 4);
        return 2;
    } else if (maxThreads < 16) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 8);
        return 3;
    } else if (maxThreads < 32) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 16);
        return 4;
    } else if (maxThreads < 64) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 32);
        return 5;
    } else if (maxThreads < 128) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 64);
        return 6;
    } else {
        printf("Number of compute threads that will be created during conversion: %d\n", 128);
        return 7;
    }
}

size_t getConvertDepthFromCores(size_t nprocsSet) {
    size_t numberOfCores;
    if (nprocsSet == 0) {
        numberOfCores = get_nprocs();
        if (global_config.verbose) printf("Number of cores detected: %lu\n", numberOfCores);
    } else {
        numberOfCores = nprocsSet;
        if (global_config.verbose) printf("Number of cores set: %lu\n", numberOfCores);
    }

    if (numberOfCores < 3) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 2);
        return 1;
    } else if (numberOfCores < 5) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 4);
        return 2;
    } else if (numberOfCores < 9) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 8);
        return 3;
    } else if (numberOfCores < 17) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 16);
        return 4;
    } else if (numberOfCores < 33) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 32);
        return 5;
    } else if (numberOfCores < 65) {
        if (global_config.verbose) printf("Number of compute threads that will be created during conversion: %d\n", 64);
        return 6;
    } else {
        printf("Number of compute threads that will be created during conversion: %d\n", 128);
        return 7;
    }
}