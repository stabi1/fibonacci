#ifndef FIBONACCI_CONFIG_H
#define FIBONACCI_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    bool verbose;
    bool parallel;
    size_t mulDepth;
    bool swap;
    size_t swapThreshold; // in MB
} Config;

extern Config global_config;

void cleanupBigIntLib();

size_t getMulDepthFromMaxThreads(size_t maxThreads);

size_t getMulDepthFromCores(size_t nprocsSet);

#endif
