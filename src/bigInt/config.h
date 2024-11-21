#ifndef FIBONACCI_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>

#define FIBONACCI_CONFIG_H

typedef struct {
    bool verbose;
    bool parallel;
    size_t mulDepth;
    bool swap;
    size_t swapThreshold; // in MB
} Config;

Config global_config = {
        .verbose = false,
        .parallel = false,
        .mulDepth = 0,
        .swap = false,
        .swapThreshold = 100
};

size_t getMulDepthFromMaxThreads(size_t maxThreads);

size_t getMulDepthFromCores(size_t nprocsSet);

#endif
