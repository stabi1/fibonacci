#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#include "bigNum/misc.h"

uint64_t parseUINT64(char *str, const uint64_t max, const uint64_t min) {
    errno = 0;
    char *endptr;
    const unsigned long long value = strtoull(str, &endptr, 10);
    if (endptr == str || *endptr != '\0') {
        fprintf(stderr, "%s could not be converted to unsigned long \n", str);
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE) {
        fprintf(stderr, "%s over - or underflows unsigned long \n", str);
        exit(EXIT_FAILURE);
    }
    if (str[0] == '-') {
        fprintf(stderr, "%s no negative numbers allowed \n", str);
        exit(EXIT_FAILURE);
    }
    if (value < min) {
        fprintf(stderr, "%s is smaller than %lu\n", str, min);
        exit(EXIT_FAILURE);
    }
    if (value > max) {
        fprintf(stderr, "%s is bigger than %lu\n", str, max);
        exit(EXIT_FAILURE);
    }
    return value;
}

void parseUINT64List(uint64_t *res, const char* list, const size_t numEntries) {
    const size_t len = strlen(list);
    char *copy = malloc(len + 1);
    mallocCheck(copy);

    memcpy(copy, list, len + 1);

    size_t i = 0;
    char *token = strtok(copy, ",");

    while (token != NULL) {
        if (i >= numEntries) {
            free(copy);
            fprintf(stderr, "%s too many entries in List\n", list);
            exit(EXIT_FAILURE);
        }

        res[i++] = parseUINT64(token, UINT64_MAX, 0);
        token = strtok(nullptr, ",");
    }
    free(copy);
    if (i != numEntries) {
        fprintf(stderr, "%s too few entries in List\n", list);
        exit(EXIT_FAILURE);
    }
}

void getCurrentTime(struct timespec *toFill) {
    const int returnCode = clock_gettime(CLOCK_MONOTONIC, toFill);
    if (returnCode == -1) {
        perror("Error measuring time!");
    }
}

double calcTimeDiff(const struct timespec *start, const struct timespec *end) {
    const double time = (double) end->tv_sec - (double) start->tv_sec + 1e-9 * (double) (end->tv_nsec - start->tv_nsec);
    return time;
}
