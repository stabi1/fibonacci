#ifndef FIBONACCI_UTIL_H
#define FIBONACCI_UTIL_H

#include <stdint.h>

uint64_t parseUINT64(char *str, uint64_t max, uint64_t min);

void parseUINT64List(uint64_t *res, const char* list, size_t numEntries);

void getCurrentTime(struct timespec *toFill);

double calcTimeDiff(const struct timespec *start, const struct timespec *end);

#endif //FIBONACCI_UTIL_H
