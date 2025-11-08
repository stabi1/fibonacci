#ifndef FIBONACCI_CONFIG_H
#define FIBONACCI_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    size_t NAIVE_MUL_FASTER; // Size when naiveMul is faster than karatsuba
    size_t KARATSUBA_FASTER; // Size when karatsuba is faster than toom-cook
    size_t TOOM_COOK_FASTER; // Size when toom-cook is faster than SSA
} MulThresholds;

// struct that holds the global config vars
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
    MulThresholds mulThresholds;
} Config;

extern Config global_config;

void cleanupBigIntLib();

size_t getMulDepthFromMaxThreads(size_t maxThreads);

size_t getMulDepthFromCores(size_t nprocsSet);

size_t getConvertDepthFromMaxThreads(size_t maxThreads);

size_t getConvertDepthFromCores(size_t nprocsSet);

size_t calcMulDepthForParallelMuls(size_t numMuls);

#endif
