#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>


// Check if the passed pointer is NULL. If p==NULL print error message and call exit()
void mallocCheck(void *p) {
    if (p == NULL) {
        fprintf(stderr, "An error occurred: Malloc returned null. Program terminated\n");
        exit(EXIT_FAILURE);
    }
}

char *getZeroString() {
    char *zero = malloc(2);
    mallocCheck(zero);
    zero[0] = '0';
    zero[1] = '\0';
    return zero;
}

char *getZeroDotZeroString() {
    char *zero = malloc(4);
    mallocCheck(zero);
    zero[0] = '0';
    zero[1] = '.';
    zero[2] = '0';
    zero[3] = '\0';
    return zero;
}

char *getCurrentDateTime() {
    char *dateTimeString = (char *) malloc(75 * sizeof(char)); // "DD-MM-YYYY HH:MM:SS\0"
    mallocCheck(dateTimeString);

    // see man localtime_r(3)
    tzset();

    // Get the current time
    time_t currentTime;
    currentTime = time(nullptr);

    // Convert to local time format
    struct tm localTime;
    if (localtime_r(&currentTime, &localTime) == NULL) {
        perror("localtime_r failed");
    }

    snprintf(dateTimeString, 75, "%02d-%02d-%04d %02d:%02d:%02d",
             localTime.tm_mday,
             localTime.tm_mon + 1,       // tm_mon is 0-based
             localTime.tm_year + 1900,   // tm_year is years since 1900
             localTime.tm_hour,
             localTime.tm_min,
             localTime.tm_sec);

    return dateTimeString;
}

// leading zero count, returns 64 if n==0
size_t custom_lzcnt(uint64_t n) {
    if (n == 0) {
        return 64;
    }
    return __builtin_clzll(n);
}

// trailing zero count, returns 64 if n==0
size_t custom_tzcnt(uint64_t n) {
    if (n == 0) {
        return 64;
    }
    return __builtin_ctzll(n);
}

size_t max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

size_t min(size_t a, size_t b) {
    return (a < b) ? a : b;
}

// Function to reverse the lowest 'bits' bits of x -> x<=64
uint64_t bit_reverse(uint64_t x, uint64_t bits) {
    uint64_t r = 0;
    for (uint64_t i = 0; i < bits; ++i) {
        if (x & (1u << i)) {
            r |= 1u << (bits - 1 - i);
        }
    }
    return r;
}