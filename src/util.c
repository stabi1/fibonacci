#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t parseUINT64(char *str, uint64_t max, uint64_t min) {
    errno = 0;
    char *endptr;
    unsigned long long value = strtoull(str, &endptr, 10);
    if (endptr == str || *endptr != '\0') {
        fprintf(stderr, "%s could not be converted to unsigned long \n", str);
        exit(EXIT_FAILURE);
    } else if (errno == ERANGE) {
        fprintf(stderr, "%s over - or underflows unsigned long \n", str);
        exit(EXIT_FAILURE);
    } else if (str[0] == '-') {
        fprintf(stderr, "%s no negative numbers allowed \n", str);
        exit(EXIT_FAILURE);
    }
    if (value < min) {
        fprintf(stderr, "%s is smaller than %lu\n", str, min);
        exit(EXIT_FAILURE);
    } else if (value > max) {
        fprintf(stderr, "%s is bigger than %lu\n", str, max);
        exit(EXIT_FAILURE);
    }
    return value;
}
