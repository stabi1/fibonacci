#ifndef FIBONACCI_CONFIG_H
#define FIBONACCI_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    bool verbose;
    bool superVerbose;
    bool parallel;
    size_t mulDepth;
    size_t convertDepth;
    bool swap;
    size_t swapThreshold; // in MB
    bool deactivateCaches;
    size_t maxThreads;
} Config;

extern Config global_config;

void cleanupBigIntLib();

size_t getMulDepthFromMaxThreads(size_t maxThreads);

size_t getMulDepthFromCores(size_t nprocsSet);

size_t getConvertDepthFromMaxThreads(size_t maxThreads);

size_t getConvertDepthFromCores(size_t nprocsSet);

size_t calcMulDepthForParallelMuls(size_t numMuls);

#endif
