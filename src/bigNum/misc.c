#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <inttypes.h>


// Check if the passed pointer is NULL. If p==NULL print error message and call exit()
void mallocCheck(const void *p) {
    if (p == NULL) {
        fprintf(stderr, "An error occurred: Malloc returned null. Program terminated\n");
        exit(EXIT_FAILURE);
    }
}

// return string 0
char *getZeroString() {
    char *zero = malloc(2);
    mallocCheck(zero);
    zero[0] = '0';
    zero[1] = '\0';
    return zero;
}

// return string 0.0
char *getZeroDotZeroString() {
    char *zero = malloc(4);
    mallocCheck(zero);
    zero[0] = '0';
    zero[1] = '.';
    zero[2] = '0';
    zero[3] = '\0';
    return zero;
}

// get a string with the current date in the format DD-MM-YYYY HH:MM:SS
char *getCurrentDateTime() {
    char *dateTimeString = malloc(75 * sizeof(char)); // "DD-MM-YYYY HH:MM:SS\0"
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
             localTime.tm_mon + 1, // tm_mon is 0-based
             localTime.tm_year + 1900, // tm_year is years since 1900
             localTime.tm_hour,
             localTime.tm_min,
             localTime.tm_sec);

    return dateTimeString;
}

// leading zero count, returns 64 if n==0
size_t custom_lzcnt(const uint64_t n) {
    if (n == 0) {
        return 64;
    }
    return __builtin_clzll(n);
}

// trailing zero count, returns 64 if n==0
size_t custom_tzcnt(const uint64_t n) {
    if (n == 0) {
        return 64;
    }
    return __builtin_ctzll(n);
}

size_t max(const size_t a, const size_t b) {
    return a > b ? a : b;
}

size_t min(const size_t a, const size_t b) {
    return a < b ? a : b;
}

// Function to reverse the lowest 'bits' bits of x -> x<=64
uint64_t bit_reverse(const uint64_t x, const uint64_t bits) {
    uint64_t r = 0;
    for (uint64_t i = 0; i < bits; ++i) {
        if (x & (1u << i)) {
            r |= 1u << (bits - 1 - i);
        }
    }
    return r;
}

// calculate GCD (greatest common devisor) using Euclidean algorithm
uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        const uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// calculate LCM (least common multiple)
uint64_t lcm(const uint64_t a, const uint64_t b) {
    if (a == 0 || b == 0) return 0;
    const __uint128_t tmp = (__uint128_t) a * b;
    return tmp / gcd(a, b);
}
