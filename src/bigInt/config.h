#ifndef FIBONACCI_CONFIG_H

#include <stdbool.h>
#include <stdlib.h>

#define FIBONACCI_CONFIG_H

typedef struct {
    bool verbose;
    bool parallel;
    bool mulDepth;
} Config;

extern Config global_config;

size_t getMulDepthFromMaxThreads(size_t maxThreads);

size_t getMulDepthFromCores(size_t nprocsSet);

#endif
